#include "PresetManager.h"

PresetManager::PresetManager(GraphManager& gm)
    : graphManager(gm)
{
    preTalkBypassStates.resize(GraphManager::MAX_RACK_SLOTS, false);
}

void PresetManager::applyQuickPreset(QuickPresetType type)
{
    currentPreset = type;

    switch (type)
    {
        case QuickPresetType::LiveSinging:
        {
            isTalkMode = false;
            // Enable all processing: Auto-Tune, EQ, Comp, and Spatial Reverb
            for (int i = 0; i < GraphManager::MAX_RACK_SLOTS; ++i)
            {
                graphManager.setSlotBypassed(i, false);
            }

            // Sync with Built-In Vocal DSP Suite
            if (auto* dsp = graphManager.getBuiltInDsp())
            {
                if (dsp->getCurrentPreset() != BuiltInDspAudioProcessor::VocalPreset::BypassAll)
                {
                    dsp->loadPreset(BuiltInDspAudioProcessor::VocalPreset::LiveSinging);
                }
            }
            break;
        }

        case QuickPresetType::TalkStream:
        {
            isTalkMode = true;
            // Save current states before muting pitch/reverb
            for (int i = 0; i < GraphManager::MAX_RACK_SLOTS; ++i)
            {
                preTalkBypassStates[i] = graphManager.isSlotBypassed(i);
            }

            // For talk/chat: Bypass Auto-Tune (Slot 0) and Reverb/Delay (Slots 6, 7)
            graphManager.setSlotBypassed(0, true);  // Bypass Pitch correction
            graphManager.setSlotBypassed(1, false); // Keep Gate active
            graphManager.setSlotBypassed(2, false); // Keep EQ active
            graphManager.setSlotBypassed(3, false); // Keep Comp 1
            graphManager.setSlotBypassed(4, false); // Keep Comp 2
            graphManager.setSlotBypassed(5, false); // Keep De-Esser
            graphManager.setSlotBypassed(6, true);  // Bypass Reverb
            graphManager.setSlotBypassed(7, true);  // Bypass Delay

            // Sync with Built-In Vocal DSP Suite (Bypass Reverb/Echo, keep clean Gate & Comp for speech)
            if (auto* dsp = graphManager.getBuiltInDsp())
            {
                if (dsp->getCurrentPreset() != BuiltInDspAudioProcessor::VocalPreset::BypassAll)
                {
                    dsp->loadPreset(BuiltInDspAudioProcessor::VocalPreset::StreamerTalk);
                }
            }
            break;
        }

        case QuickPresetType::HighEnergyTrap:
        {
            isTalkMode = false;
            // Enable all processing with active Pitch and Spatial FX
            for (int i = 0; i < GraphManager::MAX_RACK_SLOTS; ++i)
            {
                graphManager.setSlotBypassed(i, false);
            }

            // Sync with Built-In Vocal DSP Suite
            if (auto* dsp = graphManager.getBuiltInDsp())
            {
                if (dsp->getCurrentPreset() != BuiltInDspAudioProcessor::VocalPreset::BypassAll)
                {
                    dsp->loadPreset(BuiltInDspAudioProcessor::VocalPreset::LiveSinging);
                }
            }
            break;
        }
    }
}

void PresetManager::toggleTalkMode()
{
    if (!isTalkMode)
    {
        // Turn Talk mode ON
        applyQuickPreset(QuickPresetType::TalkStream);
    }
    else
    {
        // Turn Talk mode OFF: restore to Live Singing / previous states
        isTalkMode = false;
        currentPreset = QuickPresetType::LiveSinging;

        // Restore slots
        graphManager.setSlotBypassed(0, false); // Restore Auto-Tune
        graphManager.setSlotBypassed(6, false); // Restore Reverb
        graphManager.setSlotBypassed(7, false); // Restore Delay

        // Restore Built-In DSP to Live Singing
        if (auto* dsp = graphManager.getBuiltInDsp())
        {
            if (dsp->getCurrentPreset() != BuiltInDspAudioProcessor::VocalPreset::BypassAll)
            {
                dsp->loadPreset(BuiltInDspAudioProcessor::VocalPreset::LiveSinging);
            }
        }
    }
}

juce::File PresetManager::getSessionFile() const
{
    auto appDir = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
                     .getChildFile("LiveStreamMicroDAW");
    if (!appDir.exists())
        appDir.createDirectory();
    return appDir.getChildFile("session_state.xml");
}

juce::File PresetManager::getPresetsDirectory() const
{
    auto presetsDir = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
                          .getChildFile("LiveStreamMicroDAW")
                          .getChildFile("Presets");
    if (!presetsDir.exists())
        presetsDir.createDirectory();
    return presetsDir;
}

void PresetManager::saveSessionState()
{
    savePresetToFile(getSessionFile());
}

void PresetManager::restoreSessionState()
{
    auto file = getSessionFile();
    if (file.existsAsFile())
    {
        loadPresetFromFile(file);
    }
}

juce::ValueTree PresetManager::exportStateAsValueTree() const
{
    juce::ValueTree rootTree("LiveStreamMicroDAWSession");
    rootTree.setProperty("version", 1, nullptr);
    rootTree.setProperty("quickPreset", static_cast<int>(currentPreset), nullptr);
    rootTree.setProperty("isTalkMode", isTalkMode, nullptr);
    rootTree.setProperty("masterMuted", graphManager.isMasterMuted(), nullptr);
    rootTree.setProperty("masterGain", graphManager.getMasterGain(), nullptr);
    rootTree.setProperty("inputMode", static_cast<int>(graphManager.getInputSourceMode()), nullptr);
    rootTree.setProperty("windowWidth", savedWindowWidth, nullptr);
    rootTree.setProperty("windowHeight", savedWindowHeight, nullptr);
    rootTree.setProperty("uiScale", static_cast<double>(savedUiScale), nullptr);

    // Save Smart Voice Ducking State
    if (auto* bp = graphManager.getBeatPlayer())
    {
        rootTree.setProperty("duckingEnabled", bp->isDuckingEnabled(), nullptr);
        rootTree.setProperty("duckingAmountDb", bp->getDuckingAmountDb(), nullptr);
        rootTree.setProperty("duckingThresholdDb", bp->getDuckingThresholdDb(), nullptr);
        rootTree.setProperty("duckingHoldMs", bp->getDuckingHoldMs(), nullptr);
    }

    // Save Built-In Studio Vocal DSP Suite State
    if (auto* dsp = graphManager.getBuiltInDsp())
    {
        juce::MemoryBlock dspBlock;
        dsp->getStateInformation(dspBlock);
        if (dspBlock.getSize() > 0)
        {
            rootTree.setProperty("builtInDspState", dspBlock.toBase64Encoding(), nullptr);
        }
    }

    // Save All Master Slots
    juce::ValueTree masterSlotsTree("MasterSlots");
    const auto& masterSlots = graphManager.getMasterSlots();
    for (const auto& mSlot : masterSlots)
    {
        juce::ValueTree mTree("MasterSlot");
        mTree.setProperty("index", mSlot.slotIndex, nullptr);
        mTree.setProperty("name", mSlot.slotName, nullptr);
        mTree.setProperty("pluginPath", mSlot.pluginIdentifier, nullptr);
        mTree.setProperty("isBypassed", mSlot.isBypassed, nullptr);

        if (mSlot.node != nullptr && mSlot.node->getProcessor() != nullptr)
        {
            juce::MemoryBlock stateBlock;
            mSlot.node->getProcessor()->getStateInformation(stateBlock);
            if (stateBlock.getSize() > 0)
            {
                mTree.setProperty("pluginState", stateBlock.toBase64Encoding(), nullptr);
            }
        }
        masterSlotsTree.addChild(mTree, -1, nullptr);
    }
    rootTree.addChild(masterSlotsTree, -1, nullptr);

    // Save All Rack Slots
    juce::ValueTree slotsTree("Slots");
    const auto& slots = graphManager.getSlots();
    for (const auto& slot : slots)
    {
        juce::ValueTree slotTree("Slot");
        slotTree.setProperty("index", slot.slotIndex, nullptr);
        slotTree.setProperty("name", slot.slotName, nullptr);
        slotTree.setProperty("pluginPath", slot.pluginIdentifier, nullptr);
        slotTree.setProperty("isBypassed", slot.isBypassed, nullptr);
        slotTree.setProperty("isSpatialAux", slot.isSpatialAux, nullptr);
        slotTree.setProperty("sendGainDb", slot.sendGainDb, nullptr);

        if (slot.node != nullptr && slot.node->getProcessor() != nullptr)
        {
            juce::MemoryBlock stateBlock;
            slot.node->getProcessor()->getStateInformation(stateBlock);
            if (stateBlock.getSize() > 0)
            {
                slotTree.setProperty("pluginState", stateBlock.toBase64Encoding(), nullptr);
            }
        }

        slotsTree.addChild(slotTree, -1, nullptr);
    }
    rootTree.addChild(slotsTree, -1, nullptr);

    // Save Soundboard State (Custom Pad Names & Audio Files)
    if (auto* sb = graphManager.getSoundboard())
    {
        juce::ValueTree sbTree("Soundboard");
        sbTree.setProperty("masterGain", sb->getMasterSoundboardGain(), nullptr);
        for (int i = 0; i < SoundboardAudioProcessor::NUM_PADS; ++i)
        {
            const auto& pad = sb->getPadData(i);
            juce::ValueTree padTree("Pad");
            padTree.setProperty("index", i, nullptr);
            padTree.setProperty("name", pad.name, nullptr);
            if (pad.customAudioFile.existsAsFile())
                padTree.setProperty("audioPath", pad.customAudioFile.getFullPathName(), nullptr);
            sbTree.addChild(padTree, -1, nullptr);
        }
        rootTree.addChild(sbTree, -1, nullptr);
    }

    return rootTree;
}

void PresetManager::restoreStateFromValueTree(const juce::ValueTree& rootTree)
{
    if (!rootTree.hasType("LiveStreamMicroDAWSession") && !rootTree.hasType("LiveStreamMicroDAWPreset"))
        return;

    // 1. Master controls
    if (rootTree.hasProperty("masterMuted"))
        graphManager.setMasterMute(static_cast<bool>(rootTree.getProperty("masterMuted", false)));

    if (rootTree.hasProperty("masterGain"))
        graphManager.setMasterGain(static_cast<float>(rootTree.getProperty("masterGain", 1.0f)));

    if (rootTree.hasProperty("inputMode"))
        graphManager.setInputSourceMode(static_cast<InputRouterAudioProcessor::InputMode>(static_cast<int>(rootTree.getProperty("inputMode", 0))));

    if (rootTree.hasProperty("windowWidth"))
        savedWindowWidth = static_cast<int>(rootTree.getProperty("windowWidth", 1140));

    if (rootTree.hasProperty("windowHeight"))
        savedWindowHeight = static_cast<int>(rootTree.getProperty("windowHeight", 760));

    if (rootTree.hasProperty("uiScale"))
        savedUiScale = static_cast<float>(static_cast<double>(rootTree.getProperty("uiScale", 1.0)));

    if (rootTree.hasProperty("quickPreset"))
        currentPreset = static_cast<QuickPresetType>(static_cast<int>(rootTree.getProperty("quickPreset", 0)));

    if (rootTree.hasProperty("isTalkMode"))
        isTalkMode = static_cast<bool>(rootTree.getProperty("isTalkMode", false));

    // Restore Smart Voice Ducking State
    if (auto* bp = graphManager.getBeatPlayer())
    {
        if (rootTree.hasProperty("duckingEnabled"))
            bp->setDuckingEnabled(static_cast<bool>(rootTree.getProperty("duckingEnabled", false)));
        if (rootTree.hasProperty("duckingAmountDb"))
            bp->setDuckingAmountDb(static_cast<float>(rootTree.getProperty("duckingAmountDb", -12.0f)));
        if (rootTree.hasProperty("duckingThresholdDb"))
            bp->setDuckingThresholdDb(static_cast<float>(rootTree.getProperty("duckingThresholdDb", -36.0f)));
        if (rootTree.hasProperty("duckingHoldMs"))
            bp->setDuckingHoldMs(static_cast<float>(rootTree.getProperty("duckingHoldMs", 500.0f)));
    }

    // Restore Built-In Studio Vocal DSP Suite State
    if (rootTree.hasProperty("builtInDspState"))
    {
        if (auto* dsp = graphManager.getBuiltInDsp())
        {
            juce::MemoryBlock dspBlock;
            dspBlock.fromBase64Encoding(rootTree.getProperty("builtInDspState").toString());
            if (dspBlock.getSize() > 0)
            {
                dsp->setStateInformation(dspBlock.getData(), static_cast<int>(dspBlock.getSize()));
            }
        }
    }

    // 2. Master Slots
    auto masterSlotsTree = rootTree.getChildWithName("MasterSlots");
    if (masterSlotsTree.isValid())
    {
        const int numSavedMasterSlots = masterSlotsTree.getNumChildren();
        while (static_cast<int>(graphManager.getMasterSlots().size()) < numSavedMasterSlots)
        {
            graphManager.addMasterSlot("Master Insert");
        }
        while (static_cast<int>(graphManager.getMasterSlots().size()) > std::max(1, numSavedMasterSlots))
        {
            graphManager.removeMasterSlot(static_cast<int>(graphManager.getMasterSlots().size()) - 1);
        }

        for (int i = 0; i < numSavedMasterSlots; ++i)
        {
            auto mTree = masterSlotsTree.getChild(i);
            const juce::String name = mTree.getProperty("name", "Master Insert");
            const juce::String pluginPath = mTree.getProperty("pluginPath", {});
            const bool isBypassed = mTree.getProperty("isBypassed", false);
            const juce::String pluginState = mTree.getProperty("pluginState", {});

            graphManager.setMasterSlotName(i, name);

            if (pluginPath.isNotEmpty())
            {
                juce::String err;
                if (graphManager.loadMasterPluginFromFileOrIdentifier(i, pluginPath, err))
                {
                    graphManager.setMasterSlotBypassed(i, isBypassed);
                    if (pluginState.isNotEmpty())
                    {
                        juce::MemoryBlock block;
                        if (block.fromBase64Encoding(pluginState))
                        {
                            auto node = graphManager.getMasterSlotNode(i);
                            if (node != nullptr && node->getProcessor() != nullptr)
                            {
                                node->getProcessor()->setStateInformation(block.getData(), static_cast<int>(block.getSize()));
                            }
                        }
                    }
                }
            }
            else
            {
                graphManager.removePluginFromMasterSlot(i);
            }
        }
    }
    else
    {
        // Legacy single Master Limiter fallback
        auto limiterTree = rootTree.getChildWithName("MasterLimiter");
        if (limiterTree.isValid())
        {
            const juce::String pluginPath = limiterTree.getProperty("pluginPath", {});
            const bool isBypassed = limiterTree.getProperty("isBypassed", false);
            const juce::String pluginState = limiterTree.getProperty("pluginState", {});

            if (pluginPath.isNotEmpty())
            {
                juce::String err;
                if (graphManager.loadMasterPluginFromFileOrIdentifier(0, pluginPath, err))
                {
                    graphManager.setMasterSlotBypassed(0, isBypassed);
                    if (pluginState.isNotEmpty())
                    {
                        juce::MemoryBlock block;
                        if (block.fromBase64Encoding(pluginState))
                        {
                            auto node = graphManager.getMasterSlotNode(0);
                            if (node != nullptr && node->getProcessor() != nullptr)
                            {
                                node->getProcessor()->setStateInformation(block.getData(), static_cast<int>(block.getSize()));
                            }
                        }
                    }
                }
            }
            else
            {
                graphManager.removePluginFromMasterSlot(0);
            }
        }
    }

    // 3. Rack Slots
    juce::ValueTree slotsTree = rootTree.getChildWithName("Slots");
    if (!slotsTree.isValid())
    {
        // Legacy fallback
        slotsTree = rootTree;
    }

    const int numSavedSlots = slotsTree.getNumChildren();
    if (numSavedSlots > 0)
    {
        // Adjust slots size
        while (static_cast<int>(graphManager.getSlots().size()) < numSavedSlots)
        {
            graphManager.addCustomSlot("Custom Insert", false);
        }
        while (static_cast<int>(graphManager.getSlots().size()) > numSavedSlots && static_cast<int>(graphManager.getSlots().size()) > GraphManager::DEFAULT_SLOTS)
        {
            graphManager.removeSlot(static_cast<int>(graphManager.getSlots().size()) - 1);
        }

        for (int i = 0; i < numSavedSlots; ++i)
        {
            const auto slotTree = slotsTree.getChild(i);
            if (!slotTree.hasType("Slot"))
                continue;

            const int slotIndex = slotTree.getProperty("index", i);
            if (slotIndex < 0 || slotIndex >= static_cast<int>(graphManager.getSlots().size()))
                continue;

            const juce::String name = slotTree.getProperty("name", {});
            if (name.isNotEmpty())
                graphManager.setSlotName(slotIndex, name);

            const bool isAux = slotTree.getProperty("isSpatialAux", false);
            graphManager.setSlotRoutingType(slotIndex, isAux);

            const float sendGainDb = slotTree.getProperty("sendGainDb", 0.0f);
            graphManager.setSlotGainDb(slotIndex, sendGainDb);

            const bool isBypassed = slotTree.getProperty("isBypassed", false);
            const juce::String pluginPath = slotTree.getProperty("pluginPath", slotTree.getProperty("pluginIdentifier", {}));
            const juce::String pluginState = slotTree.getProperty("pluginState", {});

            if (pluginPath.isNotEmpty())
            {
                juce::String err;
                if (graphManager.loadPluginFromFileOrIdentifier(slotIndex, pluginPath, err))
                {
                    graphManager.setSlotBypassed(slotIndex, isBypassed);
                    if (pluginState.isNotEmpty())
                    {
                        juce::MemoryBlock block;
                        if (block.fromBase64Encoding(pluginState))
                        {
                            auto node = graphManager.getSlotNode(slotIndex);
                            if (node != nullptr && node->getProcessor() != nullptr)
                            {
                                node->getProcessor()->setStateInformation(block.getData(), static_cast<int>(block.getSize()));
                            }
                        }
                    }
                }
            }
            else
            {
                graphManager.removePluginFromSlot(slotIndex);
                graphManager.setSlotBypassed(slotIndex, isBypassed);
            }
        }
    }

    // 4. Restore Soundboard State (Custom Names & Custom Audio Files)
    auto sbTree = rootTree.getChildWithName("Soundboard");
    if (sbTree.isValid())
    {
        if (auto* sb = graphManager.getSoundboard())
        {
            if (sbTree.hasProperty("masterGain"))
                sb->setMasterSoundboardGain(static_cast<float>(sbTree.getProperty("masterGain", 1.0f)));

            for (int i = 0; i < sbTree.getNumChildren(); ++i)
            {
                auto padTree = sbTree.getChild(i);
                int idx = padTree.getProperty("index", i);
                if (idx >= 0 && idx < SoundboardAudioProcessor::NUM_PADS)
                {
                    if (padTree.hasProperty("name"))
                    {
                        juce::String name = padTree.getProperty("name");
                        if (name.isNotEmpty())
                            sb->setPadName(idx, name);
                    }
                    if (padTree.hasProperty("audioPath"))
                    {
                        juce::File audioFile(padTree.getProperty("audioPath").toString());
                        if (audioFile.existsAsFile())
                        {
                            juce::String err;
                            sb->loadCustomSample(idx, audioFile, err);
                        }
                    }
                }
            }
        }
    }

    graphManager.rebuildConnections();
}

bool PresetManager::savePresetToFile(const juce::File& file)
{
    const auto tree = exportStateAsValueTree();
    const auto xml = tree.createXml();
    if (xml != nullptr)
    {
        return xml->writeTo(file);
    }
    return false;
}

bool PresetManager::loadPresetFromFile(const juce::File& file)
{
    if (!file.existsAsFile())
        return false;

    const auto xml = juce::XmlDocument::parse(file);
    if (xml != nullptr)
    {
        const auto tree = juce::ValueTree::fromXml(*xml);
        restoreStateFromValueTree(tree);
        return true;
    }
    return false;
}

void PresetManager::fetchArtistPresetsCloudAsync(std::function<void(bool success, const std::vector<ArtistPresetItem>& presets, const juce::String& msg)> callback)
{
    if (isFetchingPresets.exchange(true))
    {
        if (callback)
            callback(false, cachedArtistPresets, juce::String::fromUTF8(u8"Đang tải cấu hình preset từ Cloud..."));
        return;
    }

    juce::Thread::launch([this, callback]() {
        juce::String jsonText;

        const std::vector<juce::String> cloudUrls = {
            "https://raw.githubusercontent.com/lachinhan/Hosi-Micro-Daw/main/cloud/artist_presets.json",
            "https://raw.githubusercontent.com/lachinhan/Hosi-Micro-Daw-Private/main/cloud/artist_presets.json",
            "https://api.lachinhan.xyz/microdaw/v3/artist_presets.json"
        };

        for (const auto& urlStr : cloudUrls)
        {
            try
            {
                juce::URL url(urlStr);
                auto options = juce::URL::InputStreamOptions(juce::URL::ParameterHandling::inAddress)
                                    .withConnectionTimeoutMs(4000);
                std::unique_ptr<juce::InputStream> stream(url.createInputStream(options));
                if (stream != nullptr)
                {
                    jsonText = stream->readEntireStreamAsString();
                    if (jsonText.isNotEmpty() && jsonText.trim().startsWith("["))
                    {
                        break;
                    }
                }
            }
            catch (...) {}
        }

        // Check local development fallback if network not reached
        if (jsonText.isEmpty())
        {
            auto localCloud = juce::File::getCurrentWorkingDirectory().getChildFile("cloud").getChildFile("artist_presets.json");
            if (localCloud.existsAsFile())
                jsonText = localCloud.loadFileAsString();
        }

        if (jsonText.isNotEmpty())
        {
            auto parsed = juce::JSON::parse(jsonText);
            if (parsed.isArray())
            {
                auto* arr = parsed.getArray();
                std::vector<ArtistPresetItem> loadedPresets;
                for (const auto& itemVar : *arr)
                {
                    if (itemVar.isObject())
                        loadedPresets.push_back(ArtistPresetItem::fromVar(itemVar));
                }

                juce::MessageManager::callAsync([this, loadedPresets, callback]() {
                    cachedArtistPresets = loadedPresets;
                    isFetchingPresets.store(false);

                    juce::String msg = juce::String::fromUTF8(u8"✓ Đã tải thành công ") + 
                                       juce::String(loadedPresets.size()) + 
                                       juce::String::fromUTF8(u8" Artist Preset từ Cloud.");

                    if (callback)
                        callback(true, cachedArtistPresets, msg);
                });
                return;
            }
        }

        juce::MessageManager::callAsync([this, callback]() {
            isFetchingPresets.store(false);
            if (callback)
                callback(false, cachedArtistPresets, juce::String::fromUTF8(u8"Không thể kết nối đến máy chủ Cloud."));
        });
    });
}

bool PresetManager::applyArtistPreset(const ArtistPresetItem& preset)
{
    auto* dsp = graphManager.getBuiltInDsp();
    if (dsp == nullptr)
        return false;

    const auto& d = preset.dspSettings;
    if (!d.isObject())
        return false;

    // AI Shield
    if (d.hasProperty("aiDenoise")) dsp->setAiDenoiseEnabled(static_cast<bool>(d["aiDenoise"]));
    if (d.hasProperty("aiDenoiseAmount")) dsp->setAiDenoiseAmount(static_cast<float>(d["aiDenoiseAmount"]));
    if (d.hasProperty("aiDeReverb")) dsp->setAiDeReverbEnabled(static_cast<bool>(d["aiDeReverb"]));
    if (d.hasProperty("aiDeReverbAmount")) dsp->setAiDeReverbAmount(static_cast<float>(d["aiDeReverbAmount"]));

    // Gate
    if (d.hasProperty("gateEnabled")) dsp->setGateEnabled(static_cast<bool>(d["gateEnabled"]));
    if (d.hasProperty("gateThreshold")) dsp->setGateThresholdDb(static_cast<float>(d["gateThreshold"]));

    // EQ
    if (d.hasProperty("eqEnabled")) dsp->setEqEnabled(static_cast<bool>(d["eqEnabled"]));
    if (d.hasProperty("eqLowGain")) dsp->setEqLowGainDb(static_cast<float>(d["eqLowGain"]));
    if (d.hasProperty("eqMidGain")) dsp->setEqMidGainDb(static_cast<float>(d["eqMidGain"]));
    if (d.hasProperty("eqHighGain")) dsp->setEqHighGainDb(static_cast<float>(d["eqHighGain"]));

    // Compressor
    if (d.hasProperty("compEnabled")) dsp->setCompEnabled(static_cast<bool>(d["compEnabled"]));
    if (d.hasProperty("compThreshold")) dsp->setCompThresholdDb(static_cast<float>(d["compThreshold"]));
    if (d.hasProperty("compRatio")) dsp->setCompRatio(static_cast<float>(d["compRatio"]));

    // Reverb
    if (d.hasProperty("reverbEnabled")) dsp->setReverbEnabled(static_cast<bool>(d["reverbEnabled"]));
    if (d.hasProperty("reverbSync")) dsp->setReverbBpmSync(static_cast<bool>(d["reverbSync"]));
    if (d.hasProperty("reverbRoomSize")) dsp->setReverbSize(static_cast<float>(d["reverbRoomSize"]));
    if (d.hasProperty("reverbDamping")) dsp->setReverbDamp(static_cast<float>(d["reverbDamping"]));
    if (d.hasProperty("reverbWet")) dsp->setReverbWetMix(static_cast<float>(d["reverbWet"]));

    // Delay
    if (d.hasProperty("delayEnabled")) dsp->setDelayEnabled(static_cast<bool>(d["delayEnabled"]));
    if (d.hasProperty("delaySync")) dsp->setDelayBpmSync(static_cast<bool>(d["delaySync"]));
    if (d.hasProperty("delayFeedback")) dsp->setDelayFeedback(static_cast<float>(d["delayFeedback"]));
    if (d.hasProperty("delayWet")) dsp->setDelayWetMix(static_cast<float>(d["delayWet"]));
    if (d.hasProperty("delaySubdiv"))
    {
        juce::String divStr = d["delaySubdiv"].toString();
        if (divStr == "1/4") dsp->setDelaySubdivision(TempoSyncEngine::DelaySubdivision::Quarter);
        else if (divStr == "1/8") dsp->setDelaySubdivision(TempoSyncEngine::DelaySubdivision::Eighth);
        else if (divStr.contains("Dotted")) dsp->setDelaySubdivision(TempoSyncEngine::DelaySubdivision::DottedEighth);
        else if (divStr.contains("Triplet")) dsp->setDelaySubdivision(TempoSyncEngine::DelaySubdivision::TripletEighth);
    }

    // Set Live Singing slots active
    currentPreset = QuickPresetType::LiveSinging;
    isTalkMode = false;
    for (int i = 0; i < GraphManager::MAX_RACK_SLOTS; ++i)
    {
        graphManager.setSlotBypassed(i, false);
    }

    return true;
}

