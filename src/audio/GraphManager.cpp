#include "GraphManager.h"
#include <cmath>

GraphManager::GraphManager()
{
    formatManager.addDefaultFormats(); // Adds VST3 format

    tempoSyncEngine.addChangeListener(this);

    // Standard 8 Vocal Rack Slots
    slots.resize(DEFAULT_SLOTS);
    slots[0].slotIndex = 0; slots[0].slotName = "Pitch Correction / Auto-Tune"; slots[0].isSpatialAux = false; slots[0].sendGainDb = 0.0f;
    slots[1].slotIndex = 1; slots[1].slotName = "Noise Gate"; slots[1].isSpatialAux = false; slots[1].sendGainDb = 0.0f;
    slots[2].slotIndex = 2; slots[2].slotName = "Subtractive EQ"; slots[2].isSpatialAux = false; slots[2].sendGainDb = 0.0f;
    slots[3].slotIndex = 3; slots[3].slotName = "Serial Compressor 1 (Peak)"; slots[3].isSpatialAux = false; slots[3].sendGainDb = 0.0f;
    slots[4].slotIndex = 4; slots[4].slotName = "Serial Compressor 2 (RMS)"; slots[4].isSpatialAux = false; slots[4].sendGainDb = 0.0f;
    slots[5].slotIndex = 5; slots[5].slotName = "De-Esser"; slots[5].isSpatialAux = false; slots[5].sendGainDb = 0.0f;
    slots[6].slotIndex = 6; slots[6].slotName = "Spatial FX (Reverb Send)"; slots[6].isSpatialAux = true; slots[6].sendGainDb = 0.0f;
    slots[7].slotIndex = 7; slots[7].slotName = "Spatial FX (Delay Send)"; slots[7].isSpatialAux = true; slots[7].sendGainDb = 0.0f;

    // Standard Master Insert Chain (starts with 1 slot for Master Limiter / Glue Comp)
    masterSlots.resize(1);
    masterSlots[0].slotIndex = 0;
    masterSlots[0].slotName = "Master Limiter";
    masterSlots[0].isSpatialAux = false;
    masterSlots[0].sendGainDb = 0.0f;
}

GraphManager::~GraphManager()
{
    tempoSyncEngine.removeChangeListener(this);
    if (graph != nullptr)
        graph->clear();
}

void GraphManager::changeListenerCallback(juce::ChangeBroadcaster* source)
{
    if (source == &tempoSyncEngine)
    {
        if (builtInDspProcessor != nullptr)
        {
            builtInDspProcessor->setHostBpm(tempoSyncEngine.getBpm());
        }
        sendChangeMessage();
    }
}

void GraphManager::initializeGraph()
{
    graph = std::make_unique<juce::AudioProcessorGraph>();

    using AudioGraphIOProcessor = juce::AudioProcessorGraph::AudioGraphIOProcessor;

    // Create Physical Input Node
    audioInputNode = graph->addNode(std::make_unique<AudioGraphIOProcessor>(AudioGraphIOProcessor::audioInputNode));

    // Create Input Router Node (Converts physical mono mic in to centered stereo L+R)
    auto router = std::make_unique<InputRouterAudioProcessor>();
    inputRouterProcessor = router.get();
    inputRouterNode = graph->addNode(std::move(router));

    // Create Built-In Studio DSP Vocal Suite Node
    auto dsp = std::make_unique<BuiltInDspAudioProcessor>();
    builtInDspProcessor = dsp.get();
    builtInDspProcessor->setHostBpm(tempoSyncEngine.getBpm());
    builtInDspNode = graph->addNode(std::move(dsp));

    // Create Beat Player & Key Detector Node
    auto player = std::make_unique<BeatPlayerAudioProcessor>();
    beatPlayerProcessor = player.get();
    beatPlayerProcessor->setTempoSyncEngine(&tempoSyncEngine);
    beatPlayerNode = graph->addNode(std::move(player));

    // Create Soundboard Node
    auto sb = std::make_unique<SoundboardAudioProcessor>();
    soundboardProcessor = sb.get();
    soundboardNode = graph->addNode(std::move(sb));

    // Create Physical Output Node
    audioOutputNode = graph->addNode(std::make_unique<AudioGraphIOProcessor>(AudioGraphIOProcessor::audioOutputNode));

    // Create IPC Emitter Node (routes into OBS shared memory)
    auto emitter = std::make_unique<IpcEmitterAudioProcessor>();
    ipcEmitterProcessor = emitter.get();
    ipcEmitterNode = graph->addNode(std::move(emitter));

    // Connect AudioRecorder hooks
    if (inputRouterProcessor != nullptr)
        inputRouterProcessor->setAudioRecorder(&audioRecorder);

    if (ipcEmitterProcessor != nullptr)
        ipcEmitterProcessor->setAudioRecorder(&audioRecorder);

    // Attach Host PlayHead with active continuous live-streaming transport & beat sync
    playHead.setBeatPlayer(beatPlayerProcessor);
    playHead.setTempoSyncEngine(&tempoSyncEngine);
    graph->setPlayHead(&playHead);

    rebuildConnections();
}

void GraphManager::rebuildConnections()
{
    if (graph == nullptr || audioInputNode == nullptr || audioOutputNode == nullptr)
        return;

    // Remove all existing graph audio connections
    const auto connections = graph->getConnections();
    for (const auto& conn : connections)
    {
        graph->removeConnection(conn);
    }

    // If microphone is muted, leave input disconnected
    if (isMuted)
    {
        sendChangeMessage();
        return;
    }

    // Connect physical audio input into input router
    if (inputRouterNode != nullptr)
    {
        graph->addConnection({ { audioInputNode->nodeID, 0 }, { inputRouterNode->nodeID, 0 } });
        graph->addConnection({ { audioInputNode->nodeID, 1 }, { inputRouterNode->nodeID, 1 } });
    }

    // Connect Input Router -> Built-in Studio DSP Suite
    juce::AudioProcessorGraph::Node::Ptr currentSource = (inputRouterNode != nullptr) ? inputRouterNode : audioInputNode;
    if (builtInDspNode != nullptr && inputRouterNode != nullptr)
    {
        graph->addConnection({ { inputRouterNode->nodeID, 0 }, { builtInDspNode->nodeID, 0 } });
        graph->addConnection({ { inputRouterNode->nodeID, 1 }, { builtInDspNode->nodeID, 1 } });
        currentSource = builtInDspNode;
    }

    // Prepare active serial nodes and parallel aux send nodes
    struct ActiveSlotNode {
        juce::AudioProcessorGraph::Node::Ptr pluginNode;
        juce::AudioProcessorGraph::Node::Ptr gainNode;
    };

    std::vector<ActiveSlotNode> linearNodes;
    std::vector<ActiveSlotNode> spatialNodes;

    for (auto& slot : slots)
    {
        if (slot.node != nullptr && !slot.isBypassed)
        {
            // Ensure gain node exists for slot
            if (slot.gainNode == nullptr)
            {
                auto gainProc = std::make_unique<SlotGainAudioProcessor>();
                slot.gainProcessor = gainProc.get();
                slot.gainProcessor->setGain(slot.sendGainLinear);
                slot.gainNode = graph->addNode(std::move(gainProc));
            }
            else if (slot.gainProcessor != nullptr)
            {
                slot.gainProcessor->setGain(slot.sendGainLinear);
            }

            // Connect plugin -> gain node
            graph->addConnection({ { slot.node->nodeID, 0 }, { slot.gainNode->nodeID, 0 } });
            graph->addConnection({ { slot.node->nodeID, 1 }, { slot.gainNode->nodeID, 1 } });

            // If this is a Key Detection / Analysis plugin (Auto-Key, SongKey...), route Beat audio into it!
            auto* proc = slot.node->getProcessor();
            if (proc != nullptr)
            {
                const juce::String pName = proc->getName().toLowerCase();
                if (pName.contains("key") || pName.contains("songkey") || pName.contains("mixed in key") || pName.contains("decoda"))
                {
                    if (beatPlayerNode != nullptr)
                    {
                        graph->addConnection({ { beatPlayerNode->nodeID, 0 }, { slot.node->nodeID, 0 } });
                        graph->addConnection({ { beatPlayerNode->nodeID, 1 }, { slot.node->nodeID, 1 } });
                    }
                }
            }

            ActiveSlotNode active{ slot.node, slot.gainNode };
            if (slot.isSpatialAux)
                spatialNodes.push_back(active);
            else
                linearNodes.push_back(active);
        }
    }

    // Connect serial vocal chain (Source -> Plugin -> Gain -> Next Source)
    for (auto& active : linearNodes)
    {
        graph->addConnection({ { currentSource->nodeID, 0 }, { active.pluginNode->nodeID, 0 } });
        graph->addConnection({ { currentSource->nodeID, 1 }, { active.pluginNode->nodeID, 1 } });
        currentSource = active.gainNode;
    }

    // -------------------------------------------------------------
    // Master Bus Serial Routing (Multi-Slot Master Chain)
    // -------------------------------------------------------------
    // Gather active non-bypassed master nodes
    std::vector<juce::AudioProcessorGraph::Node::Ptr> activeMasterNodes;
    for (auto& ms : masterSlots)
    {
        if (ms.node != nullptr && !ms.isBypassed)
        {
            activeMasterNodes.push_back(ms.node);
        }
    }

    // Determine entry node for the master section
    juce::AudioProcessorGraph::Node::Ptr masterEntryNode = (!activeMasterNodes.empty()) ? activeMasterNodes.front() : ipcEmitterNode;

    if (masterEntryNode != nullptr)
    {
        // Connect dry serial vocal chain into master entry
        graph->addConnection({ { currentSource->nodeID, 0 }, { masterEntryNode->nodeID, 0 } });
        graph->addConnection({ { currentSource->nodeID, 1 }, { masterEntryNode->nodeID, 1 } });

        // Connect parallel spatial aux sends into master entry
        for (auto& spatialActive : spatialNodes)
        {
            graph->addConnection({ { currentSource->nodeID, 0 }, { spatialActive.pluginNode->nodeID, 0 } });
            graph->addConnection({ { currentSource->nodeID, 1 }, { spatialActive.pluginNode->nodeID, 1 } });

            graph->addConnection({ { spatialActive.gainNode->nodeID, 0 }, { masterEntryNode->nodeID, 0 } });
            graph->addConnection({ { spatialActive.gainNode->nodeID, 1 }, { masterEntryNode->nodeID, 1 } });
        }

        // Chain multiple master plugins in serial: Master 0 -> Master 1 -> Master 2 -> ... -> IpcEmitter
        if (!activeMasterNodes.empty())
        {
            for (size_t i = 0; i + 1 < activeMasterNodes.size(); ++i)
            {
                graph->addConnection({ { activeMasterNodes[i]->nodeID, 0 }, { activeMasterNodes[i + 1]->nodeID, 0 } });
                graph->addConnection({ { activeMasterNodes[i]->nodeID, 1 }, { activeMasterNodes[i + 1]->nodeID, 1 } });
            }

            if (ipcEmitterNode != nullptr)
            {
                graph->addConnection({ { activeMasterNodes.back()->nodeID, 0 }, { ipcEmitterNode->nodeID, 0 } });
                graph->addConnection({ { activeMasterNodes.back()->nodeID, 1 }, { ipcEmitterNode->nodeID, 1 } });
            }
        }

        // Master Emitter connects to Physical Audio Output Node
        if (ipcEmitterNode != nullptr)
        {
            graph->addConnection({ { ipcEmitterNode->nodeID, 0 }, { audioOutputNode->nodeID, 0 } });
            graph->addConnection({ { ipcEmitterNode->nodeID, 1 }, { audioOutputNode->nodeID, 1 } });
        }
    }
    else
    {
        graph->addConnection({ { currentSource->nodeID, 0 }, { audioOutputNode->nodeID, 0 } });
        graph->addConnection({ { currentSource->nodeID, 1 }, { audioOutputNode->nodeID, 1 } });
    }

    // Connect Beat Player & Key Detector
    if (beatPlayerNode != nullptr)
    {
        if (inputRouterNode != nullptr)
        {
            graph->addConnection({ { inputRouterNode->nodeID, 0 }, { beatPlayerNode->nodeID, 0 } });
            graph->addConnection({ { inputRouterNode->nodeID, 1 }, { beatPlayerNode->nodeID, 1 } });
        }

        // Connect Beat Player into Master Entry so Master plugins (Auto-Key on Master, Limiter...) receive Beat
        if (masterEntryNode != nullptr)
        {
            graph->addConnection({ { beatPlayerNode->nodeID, 0 }, { masterEntryNode->nodeID, 0 } });
            graph->addConnection({ { beatPlayerNode->nodeID, 1 }, { masterEntryNode->nodeID, 1 } });
        }
        else if (ipcEmitterNode != nullptr)
        {
            graph->addConnection({ { beatPlayerNode->nodeID, 0 }, { ipcEmitterNode->nodeID, 0 } });
            graph->addConnection({ { beatPlayerNode->nodeID, 1 }, { ipcEmitterNode->nodeID, 1 } });
        }
    }

    // Connect Soundboard into Master Entry
    if (soundboardNode != nullptr)
    {
        if (masterEntryNode != nullptr)
        {
            graph->addConnection({ { soundboardNode->nodeID, 0 }, { masterEntryNode->nodeID, 0 } });
            graph->addConnection({ { soundboardNode->nodeID, 1 }, { masterEntryNode->nodeID, 1 } });
        }
        else if (ipcEmitterNode != nullptr)
        {
            graph->addConnection({ { soundboardNode->nodeID, 0 }, { ipcEmitterNode->nodeID, 0 } });
            graph->addConnection({ { soundboardNode->nodeID, 1 }, { ipcEmitterNode->nodeID, 1 } });
        }
    }

    sendChangeMessage();
}

int GraphManager::addCustomSlot(const juce::String& name, bool isAux)
{
    PluginSlotData newSlot;
    newSlot.slotIndex = static_cast<int>(slots.size());
    newSlot.slotName = name.isEmpty() ? "Custom Insert" : name;
    newSlot.isSpatialAux = isAux;
    newSlot.sendGainDb = 0.0f;
    newSlot.sendGainLinear = 1.0f;
    newSlot.isBypassed = false;

    slots.push_back(newSlot);
    rebuildConnections();
    return newSlot.slotIndex;
}

void GraphManager::removeSlot(int slotIndex)
{
    if (slotIndex >= 0 && slotIndex < static_cast<int>(slots.size()))
    {
        removePluginFromSlot(slotIndex);
        if (slots[slotIndex].gainNode != nullptr)
        {
            graph->removeNode(slots[slotIndex].gainNode->nodeID);
        }
        slots.erase(slots.begin() + slotIndex);

        // Re-index remaining slots
        for (size_t i = 0; i < slots.size(); ++i)
        {
            slots[i].slotIndex = static_cast<int>(i);
        }

        rebuildConnections();
    }
}

void GraphManager::moveSlot(int fromIndex, int toIndex)
{
    const int numSlots = static_cast<int>(slots.size());
    if (fromIndex < 0 || fromIndex >= numSlots || toIndex < 0 || toIndex >= numSlots || fromIndex == toIndex)
        return;

    auto movedSlot = std::move(slots[static_cast<size_t>(fromIndex)]);
    slots.erase(slots.begin() + fromIndex);
    slots.insert(slots.begin() + toIndex, std::move(movedSlot));

    for (size_t i = 0; i < slots.size(); ++i)
    {
        slots[i].slotIndex = static_cast<int>(i);
    }

    rebuildConnections();
}

void GraphManager::setSlotName(int slotIndex, const juce::String& name)
{
    if (slotIndex >= 0 && slotIndex < static_cast<int>(slots.size()))
    {
        slots[slotIndex].slotName = name;
        sendChangeMessage();
    }
}

void GraphManager::setSlotRoutingType(int slotIndex, bool isSpatialAux)
{
    if (slotIndex >= 0 && slotIndex < static_cast<int>(slots.size()))
    {
        if (slots[slotIndex].isSpatialAux != isSpatialAux)
        {
            slots[slotIndex].isSpatialAux = isSpatialAux;
            rebuildConnections();
        }
    }
}

bool GraphManager::isSlotSpatialAux(int slotIndex) const
{
    if (slotIndex >= 0 && slotIndex < static_cast<int>(slots.size()))
        return slots[slotIndex].isSpatialAux;
    return false;
}

void GraphManager::setSlotGainDb(int slotIndex, float gainDb)
{
    if (slotIndex >= 0 && slotIndex < static_cast<int>(slots.size()))
    {
        slots[slotIndex].sendGainDb = gainDb;
        slots[slotIndex].sendGainLinear = (gainDb <= -59.5f) ? 0.0f : std::pow(10.0f, gainDb / 20.0f);
        if (slots[slotIndex].gainProcessor != nullptr)
        {
            slots[slotIndex].gainProcessor->setGain(slots[slotIndex].sendGainLinear);
        }
    }
}

float GraphManager::getSlotGainDb(int slotIndex) const
{
    if (slotIndex >= 0 && slotIndex < static_cast<int>(slots.size()))
        return slots[slotIndex].sendGainDb;
    return 0.0f;
}

bool GraphManager::loadPluginIntoSlot(int slotIndex, const juce::PluginDescription& description, juce::String& errorMessage)
{
    if (slotIndex < 0 || slotIndex >= static_cast<int>(slots.size()))
    {
        errorMessage = "Invalid slot index.";
        return false;
    }

    removePluginFromSlot(slotIndex);

    std::unique_ptr<juce::AudioPluginInstance> instance = formatManager.createPluginInstance(
        description,
        graph->getSampleRate(),
        graph->getBlockSize(),
        errorMessage
    );

    if (instance == nullptr)
        return false;

    instance->setPlayHead(&playHead);

    slots[slotIndex].node = graph->addNode(std::move(instance));
    slots[slotIndex].pluginIdentifier = description.fileOrIdentifier;
    slots[slotIndex].isBypassed = false;

    rebuildConnections();
    return true;
}

bool GraphManager::loadPluginFromFileOrIdentifier(int slotIndex, const juce::String& identifierOrFile, juce::String& errorMessage)
{
    if (identifierOrFile.isEmpty())
        return false;

    juce::File file(identifierOrFile);
    if (file.exists())
    {
        juce::OwnedArray<juce::PluginDescription> descriptions;
        juce::VST3PluginFormat format;
        format.findAllTypesForFile(descriptions, file.getFullPathName());
        if (!descriptions.isEmpty())
        {
            return loadPluginIntoSlot(slotIndex, *descriptions[0], errorMessage);
        }
    }

    for (const auto& desc : knownPluginList.getTypes())
    {
        if (desc.fileOrIdentifier == identifierOrFile || desc.name == identifierOrFile)
        {
            return loadPluginIntoSlot(slotIndex, desc, errorMessage);
        }
    }

    errorMessage = "Plugin file not found: " + identifierOrFile;
    return false;
}

void GraphManager::removePluginFromSlot(int slotIndex)
{
    if (slotIndex >= 0 && slotIndex < static_cast<int>(slots.size()))
    {
        if (slots[slotIndex].node != nullptr)
        {
            graph->removeNode(slots[slotIndex].node->nodeID);
            slots[slotIndex].node = nullptr;
            slots[slotIndex].pluginIdentifier.clear();
        }
        if (slots[slotIndex].gainNode != nullptr)
        {
            graph->removeNode(slots[slotIndex].gainNode->nodeID);
            slots[slotIndex].gainNode = nullptr;
            slots[slotIndex].gainProcessor = nullptr;
        }
        rebuildConnections();
    }
}

void GraphManager::setSlotBypassed(int slotIndex, bool isBypassed)
{
    if (slotIndex >= 0 && slotIndex < static_cast<int>(slots.size()))
    {
        if (slots[slotIndex].isBypassed != isBypassed)
        {
            slots[slotIndex].isBypassed = isBypassed;
            if (slots[slotIndex].node != nullptr)
            {
                slots[slotIndex].node->setBypassed(isBypassed);
            }
            rebuildConnections();
        }
    }
}

bool GraphManager::isSlotBypassed(int slotIndex) const
{
    if (slotIndex >= 0 && slotIndex < static_cast<int>(slots.size()))
        return slots[slotIndex].isBypassed;
    return false;
}

juce::AudioProcessorGraph::Node::Ptr GraphManager::getSlotNode(int slotIndex) const
{
    if (slotIndex >= 0 && slotIndex < static_cast<int>(slots.size()))
        return slots[slotIndex].node;
    return nullptr;
}

// ==============================================================================
// Master Multi-Slot Rack Implementation
// ==============================================================================
int GraphManager::addMasterSlot(const juce::String& name)
{
    PluginSlotData newSlot;
    newSlot.slotIndex = static_cast<int>(masterSlots.size());
    newSlot.slotName = name.isEmpty() ? ("Master Insert " + juce::String(newSlot.slotIndex + 1)) : name;
    newSlot.isSpatialAux = false;
    newSlot.sendGainDb = 0.0f;
    newSlot.sendGainLinear = 1.0f;
    newSlot.isBypassed = false;

    masterSlots.push_back(newSlot);
    rebuildConnections();
    return newSlot.slotIndex;
}

void GraphManager::removeMasterSlot(int masterSlotIndex)
{
    if (masterSlotIndex >= 0 && masterSlotIndex < static_cast<int>(masterSlots.size()))
    {
        removePluginFromMasterSlot(masterSlotIndex);
        masterSlots.erase(masterSlots.begin() + masterSlotIndex);

        for (size_t i = 0; i < masterSlots.size(); ++i)
        {
            masterSlots[i].slotIndex = static_cast<int>(i);
        }

        rebuildConnections();
    }
}

void GraphManager::moveMasterSlot(int fromIndex, int toIndex)
{
    const int numSlots = static_cast<int>(masterSlots.size());
    if (fromIndex < 0 || fromIndex >= numSlots || toIndex < 0 || toIndex >= numSlots || fromIndex == toIndex)
        return;

    auto movedSlot = std::move(masterSlots[static_cast<size_t>(fromIndex)]);
    masterSlots.erase(masterSlots.begin() + fromIndex);
    masterSlots.insert(masterSlots.begin() + toIndex, std::move(movedSlot));

    for (size_t i = 0; i < masterSlots.size(); ++i)
    {
        masterSlots[i].slotIndex = static_cast<int>(i);
    }

    rebuildConnections();
}

void GraphManager::setMasterSlotName(int masterSlotIndex, const juce::String& name)
{
    if (masterSlotIndex >= 0 && masterSlotIndex < static_cast<int>(masterSlots.size()))
    {
        masterSlots[masterSlotIndex].slotName = name;
        sendChangeMessage();
    }
}

bool GraphManager::loadMasterPluginIntoSlot(int masterSlotIndex, const juce::PluginDescription& description, juce::String& errorMessage)
{
    if (masterSlotIndex < 0)
        return false;

    while (static_cast<int>(masterSlots.size()) <= masterSlotIndex)
    {
        addMasterSlot("Master Insert " + juce::String(masterSlots.size() + 1));
    }

    removePluginFromMasterSlot(masterSlotIndex);

    std::unique_ptr<juce::AudioPluginInstance> instance = formatManager.createPluginInstance(
        description,
        graph->getSampleRate(),
        graph->getBlockSize(),
        errorMessage
    );

    if (instance == nullptr)
        return false;

    instance->setPlayHead(&playHead);

    masterSlots[masterSlotIndex].node = graph->addNode(std::move(instance));
    masterSlots[masterSlotIndex].pluginIdentifier = description.fileOrIdentifier;
    masterSlots[masterSlotIndex].isBypassed = false;

    rebuildConnections();
    return true;
}

bool GraphManager::loadMasterPluginFromFileOrIdentifier(int masterSlotIndex, const juce::String& identifierOrFile, juce::String& errorMessage)
{
    if (identifierOrFile.isEmpty())
        return false;

    juce::File file(identifierOrFile);
    if (file.exists())
    {
        juce::OwnedArray<juce::PluginDescription> descriptions;
        juce::VST3PluginFormat format;
        format.findAllTypesForFile(descriptions, file.getFullPathName());
        if (!descriptions.isEmpty())
        {
            return loadMasterPluginIntoSlot(masterSlotIndex, *descriptions[0], errorMessage);
        }
    }

    for (const auto& desc : knownPluginList.getTypes())
    {
        if (desc.fileOrIdentifier == identifierOrFile || desc.name == identifierOrFile)
        {
            return loadMasterPluginIntoSlot(masterSlotIndex, desc, errorMessage);
        }
    }

    errorMessage = "Master plugin file not found: " + identifierOrFile;
    return false;
}

void GraphManager::removePluginFromMasterSlot(int masterSlotIndex)
{
    if (masterSlotIndex >= 0 && masterSlotIndex < static_cast<int>(masterSlots.size()))
    {
        if (masterSlots[masterSlotIndex].node != nullptr)
        {
            graph->removeNode(masterSlots[masterSlotIndex].node->nodeID);
            masterSlots[masterSlotIndex].node = nullptr;
            masterSlots[masterSlotIndex].pluginIdentifier.clear();
            rebuildConnections();
        }
    }
}

void GraphManager::setMasterSlotBypassed(int masterSlotIndex, bool isBypassed)
{
    if (masterSlotIndex >= 0 && masterSlotIndex < static_cast<int>(masterSlots.size()))
    {
        if (masterSlots[masterSlotIndex].isBypassed != isBypassed)
        {
            masterSlots[masterSlotIndex].isBypassed = isBypassed;
            if (masterSlots[masterSlotIndex].node != nullptr)
            {
                masterSlots[masterSlotIndex].node->setBypassed(isBypassed);
            }
            rebuildConnections();
        }
    }
}

bool GraphManager::isMasterSlotBypassed(int masterSlotIndex) const
{
    if (masterSlotIndex >= 0 && masterSlotIndex < static_cast<int>(masterSlots.size()))
        return masterSlots[masterSlotIndex].isBypassed;
    return false;
}

juce::AudioProcessorGraph::Node::Ptr GraphManager::getMasterSlotNode(int masterSlotIndex) const
{
    if (masterSlotIndex >= 0 && masterSlotIndex < static_cast<int>(masterSlots.size()))
        return masterSlots[masterSlotIndex].node;
    return nullptr;
}

juce::String GraphManager::getMasterSlotPluginName(int masterSlotIndex) const
{
    if (masterSlotIndex >= 0 && masterSlotIndex < static_cast<int>(masterSlots.size()))
    {
        if (masterSlots[masterSlotIndex].node != nullptr && masterSlots[masterSlotIndex].node->getProcessor() != nullptr)
            return masterSlots[masterSlotIndex].node->getProcessor()->getName();
    }
    return {};
}

void GraphManager::setMasterMute(bool shouldMute)
{
    if (isMuted != shouldMute)
    {
        isMuted = shouldMute;
        rebuildConnections();
    }
}

void GraphManager::setMasterGain(float linearGain)
{
    if (ipcEmitterProcessor != nullptr)
    {
        ipcEmitterProcessor->setMasterGain(linearGain);
    }
}

float GraphManager::getMasterGain() const
{
    return ipcEmitterProcessor ? ipcEmitterProcessor->getMasterGain() : 1.0f;
}

float GraphManager::getLeftPeak() const
{
    return ipcEmitterProcessor ? ipcEmitterProcessor->getLeftPeak() : 0.0f;
}

float GraphManager::getRightPeak() const
{
    return ipcEmitterProcessor ? ipcEmitterProcessor->getRightPeak() : 0.0f;
}

void GraphManager::setApiMode(LiveStreamIPC::AudioApiMode mode)
{
    if (ipcEmitterProcessor != nullptr)
    {
        ipcEmitterProcessor->setApiMode(mode);
    }
}

void GraphManager::setInputSourceMode(InputRouterAudioProcessor::InputMode mode)
{
    if (inputRouterProcessor != nullptr)
    {
        inputRouterProcessor->setInputMode(mode);
        sendChangeMessage();
    }
}

InputRouterAudioProcessor::InputMode GraphManager::getInputSourceMode() const
{
    if (inputRouterProcessor != nullptr)
        return inputRouterProcessor->getInputMode();
    return InputRouterAudioProcessor::InputMode::MonoIn1;
}
