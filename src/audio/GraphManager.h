#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include "IpcEmitterAudioProcessor.h"
#include "BeatPlayerAudioProcessor.h"
#include "SoundboardAudioProcessor.h"
#include "BuiltInDspAudioProcessor.h"
#include "AudioRecorder.h"
#include <vector>
#include <memory>
#include <functional>

class InputRouterAudioProcessor : public juce::AudioProcessor
{
public:
    enum class InputMode
    {
        MonoIn1 = 0,   // Input 1 (Mic 1) -> Dual-Mono Center (L + R)
        MonoIn2 = 1,   // Input 2 (Mic 2) -> Dual-Mono Center (L + R)
        Stereo12 = 2   // Input 1 & 2 -> Stereo (L -> L, R -> R)
    };

    InputRouterAudioProcessor()
        : AudioProcessor(BusesProperties().withInput("Input", juce::AudioChannelSet::discreteChannels(8), true)
                                          .withOutput("Output", juce::AudioChannelSet::stereo(), true))
    {}

    void prepareToPlay(double, int) override {}
    void releaseResources() override {}
    void processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&) override
    {
        const int numChannels = buffer.getNumChannels();
        const int numSamples = buffer.getNumSamples();
        if (numSamples <= 0 || numChannels <= 0)
            return;

        const auto mode = inputMode.load(std::memory_order_relaxed);

        if (mode == InputMode::MonoIn1)
        {
            if (numChannels >= 2)
            {
                // If channel 0 is silent but channel 2 or 1 has audio (e.g. mic in input 2 / secondary ASIO pair)
                if (buffer.getMagnitude(0, 0, numSamples) <= 0.000001f)
                {
                    if (numChannels > 2 && buffer.getMagnitude(2, 0, numSamples) > 0.00001f)
                        buffer.copyFrom(0, 0, buffer, 2, 0, numSamples);
                    else if (buffer.getMagnitude(1, 0, numSamples) > 0.00001f)
                        buffer.copyFrom(0, 0, buffer, 1, 0, numSamples);
                }
                buffer.copyFrom(1, 0, buffer, 0, 0, numSamples);
            }
        }
        else if (mode == InputMode::MonoIn2)
        {
            if (numChannels >= 2)
            {
                if (numChannels > 3 && buffer.getMagnitude(1, 0, numSamples) <= 0.000001f && buffer.getMagnitude(3, 0, numSamples) > 0.00001f)
                    buffer.copyFrom(1, 0, buffer, 3, 0, numSamples);

                buffer.copyFrom(0, 0, buffer, 1, 0, numSamples);
                buffer.copyFrom(1, 0, buffer, 0, 0, numSamples);
            }
        }
        else // Stereo12
        {
            if (numChannels >= 2)
            {
                if (numChannels >= 4 && buffer.getMagnitude(0, 0, numSamples) <= 0.000001f && buffer.getMagnitude(1, 0, numSamples) <= 0.000001f)
                {
                    if (buffer.getMagnitude(2, 0, numSamples) > 0.00001f || buffer.getMagnitude(3, 0, numSamples) > 0.00001f)
                    {
                        buffer.copyFrom(0, 0, buffer, 2, 0, numSamples);
                        buffer.copyFrom(1, 0, buffer, 3, 0, numSamples);
                    }
                }
            }
        }

        if (recorder != nullptr && recorder->isRecording())
        {
            recorder->pushDryMicAudio(buffer.getArrayOfReadPointers(), buffer.getNumChannels(), numSamples);
        }
    }

    void setAudioRecorder(AudioRecorder* rec) noexcept { recorder = rec; }
    void setInputMode(InputMode mode) noexcept { inputMode.store(mode, std::memory_order_release); }
    InputMode getInputMode() const noexcept { return inputMode.load(std::memory_order_relaxed); }

    const juce::String getName() const override { return "Input Router & Dual-Mono Center"; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}
    void getStateInformation(juce::MemoryBlock&) override {}
    void setStateInformation(const void*, int) override {}
    juce::AudioProcessorEditor* createEditor() override { return nullptr; }
    bool hasEditor() const override { return false; }

private:
    AudioRecorder* recorder{ nullptr };
    std::atomic<InputMode> inputMode{ InputMode::MonoIn1 };
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(InputRouterAudioProcessor)
};

class SlotGainAudioProcessor : public juce::AudioProcessor
{
public:
    SlotGainAudioProcessor()
        : AudioProcessor(BusesProperties().withInput("Input", juce::AudioChannelSet::stereo(), true)
                                          .withOutput("Output", juce::AudioChannelSet::stereo(), true))
    {}

    void prepareToPlay(double sampleRate, int /*samplesPerBlock*/) override
    {
        smoothedGain.reset(sampleRate, 0.02);
        smoothedGain.setCurrentAndTargetValue(gain.load(std::memory_order_relaxed));
    }
    void releaseResources() override {}
    void processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&) override
    {
        const int numChannels = buffer.getNumChannels();
        const int numSamples = buffer.getNumSamples();
        if (numSamples <= 0 || numChannels <= 0)
            return;

        // If a mono plugin outputted to ch 0 only, mirror to ch 1 so signal stays centered stereo
        if (numChannels >= 2)
        {
            const float mag0 = buffer.getMagnitude(0, 0, numSamples);
            const float mag1 = buffer.getMagnitude(1, 0, numSamples);
            if (mag0 > 0.000001f && mag1 <= 0.0000001f)
            {
                buffer.copyFrom(1, 0, buffer, 0, 0, numSamples);
            }
        }

        const float target = gain.load(std::memory_order_relaxed);
        smoothedGain.setTargetValue(target);
        smoothedGain.applyGain(buffer, numSamples);
    }

    void setGain(float newGain) noexcept { gain.store(newGain, std::memory_order_release); }
    float getGain() const noexcept { return gain.load(std::memory_order_relaxed); }

    const juce::String getName() const override { return "Slot Trim & Send"; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}
    void getStateInformation(juce::MemoryBlock&) override {}
    void setStateInformation(const void*, int) override {}
    juce::AudioProcessorEditor* createEditor() override { return nullptr; }
    bool hasEditor() const override { return false; }

private:
    std::atomic<float> gain{ 1.0f };
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> smoothedGain;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SlotGainAudioProcessor)
};

class MicroDawPlayHead : public juce::AudioPlayHead
{
public:
    MicroDawPlayHead() = default;

    void setBeatPlayer(BeatPlayerAudioProcessor* player) noexcept { beatPlayer = player; }

    juce::Optional<PositionInfo> getPosition() const override
    {
        PositionInfo info;
        
        // Transport active for continuous live stream & beat
        info.setIsPlaying(true);
        info.setIsRecording(false);
        
        const bool isLooping = (beatPlayer != nullptr && beatPlayer->isLooping());
        info.setIsLooping(isLooping);
        
        constexpr double defaultBpm = 120.0;
        info.setBpm(defaultBpm);
        info.setTimeSignature(juce::AudioPlayHead::TimeSignature{ 4, 4 });
        
        if (beatPlayer != nullptr && beatPlayer->isPlaying())
        {
            double posSec = beatPlayer->getCurrentPosition();
            info.setTimeInSeconds(posSec);
            info.setTimeInSamples(static_cast<juce::int64>(posSec * 44100.0));
            info.setPpqPosition((posSec * defaultBpm) / 60.0);
            info.setPpqPositionOfLastBarStart(0.0);
        }
        else
        {
            info.setTimeInSeconds(0.0);
            info.setTimeInSamples(0);
            info.setPpqPosition(0.0);
            info.setPpqPositionOfLastBarStart(0.0);
        }
        
        return info;
    }

private:
    BeatPlayerAudioProcessor* beatPlayer{ nullptr };
};

struct PluginSlotData
{
    int slotIndex{ 0 };
    juce::String slotName{ "Custom Insert" };
    juce::String pluginIdentifier;
    bool isBypassed{ false };
    bool isSpatialAux{ false }; // false = Serial Insert, true = Parallel Aux Send
    float sendGainDb{ 0.0f };   // e.g. -60dB to +6dB (default 0dB)
    float sendGainLinear{ 1.0f };
    juce::AudioProcessorGraph::Node::Ptr node;
    juce::AudioProcessorGraph::Node::Ptr gainNode;
    SlotGainAudioProcessor* gainProcessor{ nullptr };
};

class GraphManager : public juce::ChangeBroadcaster
{
public:
    static constexpr int DEFAULT_SLOTS = 8;
    static constexpr int MAX_RACK_SLOTS = 24;

    GraphManager();
    ~GraphManager() override;

    juce::AudioProcessorGraph& getGraph() noexcept { return *graph; }
    juce::AudioPlayHead* getPlayHead() noexcept { return &playHead; }
    juce::KnownPluginList& getKnownPluginList() noexcept { return knownPluginList; }
    juce::AudioPluginFormatManager& getFormatManager() noexcept { return formatManager; }

    void initializeGraph();
    void rebuildConnections();

    // Slot Management
    int addCustomSlot(const juce::String& name = "Custom Insert", bool isAux = false);
    void removeSlot(int slotIndex);
    void moveSlot(int fromIndex, int toIndex);
    void setSlotName(int slotIndex, const juce::String& name);
    void setSlotRoutingType(int slotIndex, bool isSpatialAux);
    bool isSlotSpatialAux(int slotIndex) const;

    // Send Level / Slot Gain (dB)
    void setSlotGainDb(int slotIndex, float gainDb);
    float getSlotGainDb(int slotIndex) const;

    bool loadPluginIntoSlot(int slotIndex, const juce::PluginDescription& description, juce::String& errorMessage);
    bool loadPluginFromFileOrIdentifier(int slotIndex, const juce::String& identifierOrFile, juce::String& errorMessage);
    void removePluginFromSlot(int slotIndex);
    void setSlotBypassed(int slotIndex, bool isBypassed);
    bool isSlotBypassed(int slotIndex) const;

    const std::vector<PluginSlotData>& getSlots() const noexcept { return slots; }
    juce::AudioProcessorGraph::Node::Ptr getSlotNode(int slotIndex) const;

    // Master Multi-Slot Rack Management (Bus Insert Chain)
    int addMasterSlot(const juce::String& name = "Master Insert");
    void removeMasterSlot(int masterSlotIndex);
    void moveMasterSlot(int fromIndex, int toIndex);
    void setMasterSlotName(int masterSlotIndex, const juce::String& name);

    bool loadMasterPluginIntoSlot(int masterSlotIndex, const juce::PluginDescription& description, juce::String& errorMessage);
    bool loadMasterPluginFromFileOrIdentifier(int masterSlotIndex, const juce::String& identifierOrFile, juce::String& errorMessage);
    void removePluginFromMasterSlot(int masterSlotIndex);
    void setMasterSlotBypassed(int masterSlotIndex, bool isBypassed);
    bool isMasterSlotBypassed(int masterSlotIndex) const;

    const std::vector<PluginSlotData>& getMasterSlots() const noexcept { return masterSlots; }
    juce::AudioProcessorGraph::Node::Ptr getMasterSlotNode(int masterSlotIndex) const;
    juce::String getMasterSlotPluginName(int masterSlotIndex) const;

    // Backward compatibility helpers for Slot 0
    bool loadMasterPlugin(const juce::PluginDescription& description, juce::String& errorMessage) { return loadMasterPluginIntoSlot(0, description, errorMessage); }
    bool loadMasterPluginFromFileOrIdentifier(const juce::String& id, juce::String& err) { return loadMasterPluginFromFileOrIdentifier(0, id, err); }
    void removeMasterPlugin() { removePluginFromMasterSlot(0); }
    void setMasterPluginBypassed(bool isBypassed) { setMasterSlotBypassed(0, isBypassed); }
    bool isMasterPluginBypassed() const noexcept { return isMasterSlotBypassed(0); }
    juce::AudioProcessorGraph::Node::Ptr getMasterPluginNode() const noexcept { return getMasterSlotNode(0); }
    juce::String getMasterPluginIdentifier() const noexcept { return (!masterSlots.empty()) ? masterSlots[0].pluginIdentifier : juce::String(); }
    juce::String getMasterPluginName() const { return getMasterSlotPluginName(0); }

    // Input Source Routing (Mono Mic 1, Mono Mic 2, Stereo)
    void setInputSourceMode(InputRouterAudioProcessor::InputMode mode);
    InputRouterAudioProcessor::InputMode getInputSourceMode() const;

    // Mode & Master controls
    void setApiMode(LiveStreamIPC::AudioApiMode mode);
    void setMasterMute(bool shouldMute);
    bool isMasterMuted() const noexcept { return isMuted; }

    void setMasterGain(float linearGain);
    float getMasterGain() const;
    float getLeftPeak() const;
    float getRightPeak() const;

    BeatPlayerAudioProcessor* getBeatPlayer() noexcept { return beatPlayerProcessor; }
    SoundboardAudioProcessor* getSoundboard() noexcept { return soundboardProcessor; }
    BuiltInDspAudioProcessor* getBuiltInDsp() noexcept { return builtInDspProcessor; }
    AudioRecorder& getAudioRecorder() noexcept { return audioRecorder; }

private:
    AudioRecorder audioRecorder;
    std::unique_ptr<juce::AudioProcessorGraph> graph;
    juce::AudioPluginFormatManager formatManager;
    juce::KnownPluginList knownPluginList;

    juce::AudioProcessorGraph::Node::Ptr audioInputNode;
    juce::AudioProcessorGraph::Node::Ptr inputRouterNode;
    InputRouterAudioProcessor* inputRouterProcessor{ nullptr };

    juce::AudioProcessorGraph::Node::Ptr builtInDspNode;
    BuiltInDspAudioProcessor* builtInDspProcessor{ nullptr };

    juce::AudioProcessorGraph::Node::Ptr audioOutputNode;
    juce::AudioProcessorGraph::Node::Ptr ipcEmitterNode;
    IpcEmitterAudioProcessor* ipcEmitterProcessor{ nullptr };

    juce::AudioProcessorGraph::Node::Ptr beatPlayerNode;
    BeatPlayerAudioProcessor* beatPlayerProcessor{ nullptr };

    juce::AudioProcessorGraph::Node::Ptr soundboardNode;
    SoundboardAudioProcessor* soundboardProcessor{ nullptr };

    // Vocal Rack Slots (Channel Chain)
    std::vector<PluginSlotData> slots;

    // Master Bus Slots (Mastering Chain)
    std::vector<PluginSlotData> masterSlots;

    bool isMuted{ false };
    MicroDawPlayHead playHead;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GraphManager)
};
