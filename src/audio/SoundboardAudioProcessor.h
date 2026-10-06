#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_formats/juce_audio_formats.h>
#include <array>
#include <atomic>
#include <vector>

struct SoundPadData
{
    int index{ 0 };
    juce::String name;
    juce::String shortcutKey;
    juce::Colour padColour{ 0xff0284c7 };
    float volumeGain{ 1.0f };
    juce::File customAudioFile;
    
    // In-memory audio sample buffer
    juce::AudioBuffer<float> sampleBuffer;
    double sampleRate{ 44100.0 };

    // Playback state
    std::atomic<bool> isPlaying{ false };
    std::atomic<int> playPosition{ 0 };
};

class SoundboardAudioProcessor : public juce::AudioProcessor, public juce::ChangeBroadcaster
{
public:
    static constexpr int NUM_PADS = 8;

    SoundboardAudioProcessor();
    ~SoundboardAudioProcessor() override = default;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    void processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) override;

    // Trigger actions
    void triggerPad(int padIndex);
    void stopPad(int padIndex);
    void stopAll();

    // Configuration
    bool loadCustomSample(int padIndex, const juce::File& audioFile, juce::String& errorMsg);
    void setPadVolume(int padIndex, float gainLinear);
    void setMasterSoundboardGain(float gainLinear);
    float getMasterSoundboardGain() const noexcept { return masterGain.load(std::memory_order_relaxed); }

    bool isPadPlaying(int padIndex) const;
    const SoundPadData& getPadData(int padIndex) const;
    void setPadName(int padIndex, const juce::String& newName);
    void resetPad(int padIndex);
    void scanAndLoadSoundsFolder();

    // Standard AudioProcessor methods
    const juce::String getName() const override { return "Live Stream Soundboard"; }
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
    juce::AudioFormatManager formatManager;
    std::array<SoundPadData, NUM_PADS> pads;
    std::atomic<float> masterGain{ 1.0f };
    double currentSampleRate{ 44100.0 };

    void generateBuiltinSamples();
    void generateApplauseSample(juce::AudioBuffer<float>& buf, double sr);
    void generateLaughterSample(juce::AudioBuffer<float>& buf, double sr);
    void generateDrumrollSample(juce::AudioBuffer<float>& buf, double sr);
    void generateDingBellSample(juce::AudioBuffer<float>& buf, double sr);
    void generateAirHornSample(juce::AudioBuffer<float>& buf, double sr);
    void generateImpactHitSample(juce::AudioBuffer<float>& buf, double sr);
    void generateBuzzerSample(juce::AudioBuffer<float>& buf, double sr);
    void generateCheerSample(juce::AudioBuffer<float>& buf, double sr);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SoundboardAudioProcessor)
};
