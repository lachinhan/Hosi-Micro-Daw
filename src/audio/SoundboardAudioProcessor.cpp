#include "SoundboardAudioProcessor.h"
#include <cmath>
#include <random>

SoundboardAudioProcessor::SoundboardAudioProcessor()
    : AudioProcessor(BusesProperties().withOutput("Output", juce::AudioChannelSet::stereo(), true))
{
    formatManager.registerBasicFormats();

    // Default 8 Soundboard Pads Configuration
    pads[0].index = 0; pads[0].name = juce::String::fromUTF8(u8"VỖ TAY"); pads[0].shortcutKey = "1"; pads[0].padColour = juce::Colour(0xff059669); // Emerald
    pads[1].index = 1; pads[1].name = juce::String::fromUTF8(u8"TIẾNG CƯỜI"); pads[1].shortcutKey = "2"; pads[1].padColour = juce::Colour(0xffeab308); // Amber/Yellow
    pads[2].index = 2; pads[2].name = juce::String::fromUTF8(u8"HỒI HỘP"); pads[2].shortcutKey = "3"; pads[2].padColour = juce::Colour(0xfff97316); // Orange
    pads[3].index = 3; pads[3].name = juce::String::fromUTF8(u8"TING CHUÔNG"); pads[3].shortcutKey = "4"; pads[3].padColour = juce::Colour(0xff06b6d4); // Cyan
    pads[4].index = 4; pads[4].name = juce::String::fromUTF8(u8"KÈN AIRHORN"); pads[4].shortcutKey = "5"; pads[4].padColour = juce::Colour(0xffec4899); // Pink
    pads[5].index = 5; pads[5].name = juce::String::fromUTF8(u8"TIẾNG NỔ BOOM"); pads[5].shortcutKey = "6"; pads[5].padColour = juce::Colour(0xffdc2626); // Red
    pads[6].index = 6; pads[6].name = juce::String::fromUTF8(u8"ÂM BÁO LỖI"); pads[6].shortcutKey = "7"; pads[6].padColour = juce::Colour(0xff64748b); // Slate
    pads[7].index = 7; pads[7].name = juce::String::fromUTF8(u8"REO HÒ WIN"); pads[7].shortcutKey = "8"; pads[7].padColour = juce::Colour(0xff8b5cf6); // Purple

    scanAndLoadSoundsFolder();
}

void SoundboardAudioProcessor::scanAndLoadSoundsFolder()
{
    juce::File soundsDir = juce::File::getSpecialLocation(juce::File::currentExecutableFile).getParentDirectory().getChildFile("sounds");
    if (!soundsDir.isDirectory())
    {
        soundsDir = juce::File::getCurrentWorkingDirectory().getChildFile("sounds");
    }

    if (soundsDir.isDirectory())
    {
        auto audioFiles = soundsDir.findChildFiles(juce::File::findFiles, false, "*.wav;*.mp3;*.flac;*.ogg;*.aiff");
        audioFiles.sort();

        for (int i = 0; i < std::min(NUM_PADS, audioFiles.size()); ++i)
        {
            juce::String err;
            loadCustomSample(i, audioFiles[i], err);
        }
    }
}

void SoundboardAudioProcessor::prepareToPlay(double sampleRate, int /*samplesPerBlock*/)
{
    if (std::abs(currentSampleRate - sampleRate) > 1.0)
    {
        currentSampleRate = sampleRate;
        generateBuiltinSamples();
    }
}

void SoundboardAudioProcessor::releaseResources()
{
}

void SoundboardAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    // CRITICAL: Clear buffer first because this is a sound generator node (no input bus)
    buffer.clear();

    const int numSamples = buffer.getNumSamples();
    const int numChannels = buffer.getNumChannels();
    if (numSamples <= 0 || numChannels <= 0)
        return;

    const float globalGain = masterGain.load(std::memory_order_relaxed);

    for (int p = 0; p < NUM_PADS; ++p)
    {
        auto& pad = pads[p];
        if (pad.isPlaying.load(std::memory_order_relaxed))
        {
            int currentPos = pad.playPosition.load(std::memory_order_relaxed);
            const int sampleLen = pad.sampleBuffer.getNumSamples();
            const int sampleChannels = pad.sampleBuffer.getNumChannels();

            if (currentPos >= sampleLen || sampleLen == 0)
            {
                pad.isPlaying.store(false, std::memory_order_release);
                continue;
            }

            const int samplesToPlay = std::min(numSamples, sampleLen - currentPos);
            const float padGain = pad.volumeGain * globalGain;

            for (int ch = 0; ch < numChannels; ++ch)
            {
                const int srcCh = (sampleChannels > 1) ? (ch % sampleChannels) : 0;
                buffer.addFrom(ch, 0, pad.sampleBuffer, srcCh, currentPos, samplesToPlay, padGain);
            }

            currentPos += samplesToPlay;
            pad.playPosition.store(currentPos, std::memory_order_release);

            if (currentPos >= sampleLen)
            {
                pad.isPlaying.store(false, std::memory_order_release);
            }
        }
    }
}

void SoundboardAudioProcessor::triggerPad(int padIndex)
{
    if (padIndex >= 0 && padIndex < NUM_PADS)
    {
        pads[padIndex].playPosition.store(0, std::memory_order_release);
        pads[padIndex].isPlaying.store(true, std::memory_order_release);
        sendChangeMessage();
    }
}

void SoundboardAudioProcessor::stopPad(int padIndex)
{
    if (padIndex >= 0 && padIndex < NUM_PADS)
    {
        pads[padIndex].isPlaying.store(false, std::memory_order_release);
        pads[padIndex].playPosition.store(0, std::memory_order_release);
        sendChangeMessage();
    }
}

void SoundboardAudioProcessor::stopAll()
{
    for (int i = 0; i < NUM_PADS; ++i)
    {
        pads[i].isPlaying.store(false, std::memory_order_release);
        pads[i].playPosition.store(0, std::memory_order_release);
    }
    sendChangeMessage();
}

bool SoundboardAudioProcessor::loadCustomSample(int padIndex, const juce::File& audioFile, juce::String& errorMsg)
{
    if (padIndex < 0 || padIndex >= NUM_PADS)
        return false;

    if (!audioFile.existsAsFile())
    {
        errorMsg = "File does not exist: " + audioFile.getFullPathName();
        return false;
    }

    auto* reader = formatManager.createReaderFor(audioFile);
    if (reader == nullptr)
    {
        errorMsg = "Unsupported audio format. Supported: WAV, MP3, FLAC, OGG, AIFF";
        return false;
    }

    pads[padIndex].isPlaying.store(false, std::memory_order_release);
    pads[padIndex].customAudioFile = audioFile;
    pads[padIndex].sampleBuffer.setSize(static_cast<int>(reader->numChannels), static_cast<int>(reader->lengthInSamples));
    reader->read(&pads[padIndex].sampleBuffer, 0, static_cast<int>(reader->lengthInSamples), 0, true, true);
    pads[padIndex].name = audioFile.getFileNameWithoutExtension().substring(0, 12);

    delete reader;
    sendChangeMessage();
    return true;
}

void SoundboardAudioProcessor::setPadVolume(int padIndex, float gainLinear)
{
    if (padIndex >= 0 && padIndex < NUM_PADS)
    {
        pads[padIndex].volumeGain = std::clamp(gainLinear, 0.0f, 2.0f);
    }
}

void SoundboardAudioProcessor::setMasterSoundboardGain(float gainLinear)
{
    masterGain.store(std::clamp(gainLinear, 0.0f, 2.0f), std::memory_order_release);
}

bool SoundboardAudioProcessor::isPadPlaying(int padIndex) const
{
    if (padIndex >= 0 && padIndex < NUM_PADS)
        return pads[padIndex].isPlaying.load(std::memory_order_relaxed);
    return false;
}

const SoundPadData& SoundboardAudioProcessor::getPadData(int padIndex) const
{
    jassert(padIndex >= 0 && padIndex < NUM_PADS);
    return pads[padIndex];
}

void SoundboardAudioProcessor::setPadName(int padIndex, const juce::String& newName)
{
    if (padIndex >= 0 && padIndex < NUM_PADS)
    {
        pads[padIndex].name = newName;
        sendChangeMessage();
    }
}

void SoundboardAudioProcessor::resetPad(int padIndex)
{
    if (padIndex < 0 || padIndex >= NUM_PADS)
        return;

    const char* defaultNames[NUM_PADS] = {
        (const char*)u8"VỖ TAY",
        (const char*)u8"TIẾNG CƯỜI",
        (const char*)u8"HỒI HỘP",
        (const char*)u8"TING CHUÔNG",
        (const char*)u8"KÈN AIRHORN",
        (const char*)u8"TIẾNG NỔ BOOM",
        (const char*)u8"ÂM BÁO LỖI",
        (const char*)u8"REO HÒ WIN"
    };

    pads[padIndex].name = juce::String::fromUTF8(defaultNames[padIndex]);
    pads[padIndex].customAudioFile = juce::File();
    pads[padIndex].volumeGain = 1.0f;
    pads[padIndex].isPlaying.store(false, std::memory_order_release);
    pads[padIndex].playPosition.store(0, std::memory_order_release);

    const double sr = (currentSampleRate > 8000.0) ? currentSampleRate : 44100.0;
    switch (padIndex)
    {
        case 0: generateApplauseSample(pads[0].sampleBuffer, sr); break;
        case 1: generateLaughterSample(pads[1].sampleBuffer, sr); break;
        case 2: generateDrumrollSample(pads[2].sampleBuffer, sr); break;
        case 3: generateDingBellSample(pads[3].sampleBuffer, sr); break;
        case 4: generateAirHornSample(pads[4].sampleBuffer, sr); break;
        case 5: generateImpactHitSample(pads[5].sampleBuffer, sr); break;
        case 6: generateBuzzerSample(pads[6].sampleBuffer, sr); break;
        case 7: generateCheerSample(pads[7].sampleBuffer, sr); break;
    }

    sendChangeMessage();
}

// -----------------------------------------------------------------------------
// Studio Quality Sound Synthesis
// -----------------------------------------------------------------------------

void SoundboardAudioProcessor::generateBuiltinSamples()
{
    const double sr = (currentSampleRate > 8000.0) ? currentSampleRate : 44100.0;

    generateApplauseSample(pads[0].sampleBuffer, sr);
    generateLaughterSample(pads[1].sampleBuffer, sr);
    generateDrumrollSample(pads[2].sampleBuffer, sr);
    generateDingBellSample(pads[3].sampleBuffer, sr);
    generateAirHornSample(pads[4].sampleBuffer, sr);
    generateImpactHitSample(pads[5].sampleBuffer, sr);
    generateBuzzerSample(pads[6].sampleBuffer, sr);
    generateCheerSample(pads[7].sampleBuffer, sr);
}

void SoundboardAudioProcessor::generateApplauseSample(juce::AudioBuffer<float>& buf, double sr)
{
    const int numSamples = static_cast<int>(sr * 2.2); // 2.2s applause
    buf.setSize(2, numSamples);
    buf.clear();

    std::mt19937 rng(42);
    std::uniform_real_distribution<float> dist(-1.0f, 1.0f);

    float* l = buf.getWritePointer(0);
    float* r = buf.getWritePointer(1);

    for (int i = 0; i < numSamples; ++i)
    {
        const float t = static_cast<float>(i) / static_cast<float>(numSamples);
        const float env = (t < 0.1f) ? (t / 0.1f) : std::exp(-(t - 0.1f) * 2.5f);

        const float clapPulse = std::pow(std::sin(static_cast<float>(i) * 0.003f), 4.0f);
        const float noiseL = dist(rng) * (0.3f + 0.7f * clapPulse);
        const float noiseR = dist(rng) * (0.3f + 0.7f * clapPulse);

        l[i] = noiseL * env * 0.35f;
        r[i] = noiseR * env * 0.35f;
    }
}

void SoundboardAudioProcessor::generateLaughterSample(juce::AudioBuffer<float>& buf, double sr)
{
    const int numSamples = static_cast<int>(sr * 1.8); // 1.8s laughter
    buf.setSize(2, numSamples);
    buf.clear();

    float* l = buf.getWritePointer(0);
    float* r = buf.getWritePointer(1);

    for (int i = 0; i < numSamples; ++i)
    {
        const float time = static_cast<float>(i) / static_cast<float>(sr);
        const float haHaFreq = 4.0f;
        const float haHaEnv = std::max(0.0f, std::sin(juce::MathConstants<float>::twoPi * haHaFreq * time));

        const float pitch = 340.0f + 80.0f * std::sin(juce::MathConstants<float>::twoPi * 2.5f * time);
        const float wave = std::sin(juce::MathConstants<float>::twoPi * pitch * time)
                         + 0.3f * std::sin(juce::MathConstants<float>::twoPi * pitch * 2.0f * time);

        const float globalEnv = (time < 0.15f) ? (time / 0.15f) : std::exp(-(time - 0.15f) * 1.8f);
        const float sample = wave * haHaEnv * globalEnv * 0.3f;

        l[i] = sample;
        r[i] = sample;
    }
}

void SoundboardAudioProcessor::generateDrumrollSample(juce::AudioBuffer<float>& buf, double sr)
{
    const int numSamples = static_cast<int>(sr * 2.5); // 2.5s drumroll + crash
    buf.setSize(2, numSamples);
    buf.clear();

    std::mt19937 rng(1337);
    std::uniform_real_distribution<float> dist(-1.0f, 1.0f);

    float* l = buf.getWritePointer(0);
    float* r = buf.getWritePointer(1);

    for (int i = 0; i < numSamples; ++i)
    {
        const float time = static_cast<float>(i) / static_cast<float>(sr);
        if (time < 1.8f)
        {
            const float rollSpeed = 16.0f + time * 10.0f;
            const float pulse = std::abs(std::sin(juce::MathConstants<float>::twoPi * rollSpeed * time));
            const float snareTone = std::sin(juce::MathConstants<float>::twoPi * 190.0f * time);
            const float noise = dist(rng);
            const float env = (time / 1.8f) * 0.35f;

            const float s = (snareTone * 0.4f + noise * 0.6f) * pulse * env;
            l[i] = s;
            r[i] = s;
        }
        else
        {
            // Crash hit at 1.8s
            const float crashTime = time - 1.8f;
            const float crashEnv = std::exp(-crashTime * 4.0f);
            const float noise = dist(rng);
            const float kick = std::sin(juce::MathConstants<float>::twoPi * (85.0f * std::exp(-crashTime * 10.0f)) * crashTime);

            l[i] = (noise * 0.5f + kick * 0.5f) * crashEnv * 0.55f;
            r[i] = (dist(rng) * 0.5f + kick * 0.5f) * crashEnv * 0.55f;
        }
    }
}

void SoundboardAudioProcessor::generateDingBellSample(juce::AudioBuffer<float>& buf, double sr)
{
    const int numSamples = static_cast<int>(sr * 1.5); // 1.5s clean crystal bell
    buf.setSize(2, numSamples);
    buf.clear();

    float* l = buf.getWritePointer(0);
    float* r = buf.getWritePointer(1);

    for (int i = 0; i < numSamples; ++i)
    {
        const float time = static_cast<float>(i) / static_cast<float>(sr);
        const float env = std::exp(-time * 3.0f);

        // Pristine Bell Chime (C6 + G6 + C7)
        const float wave = 0.55f * std::sin(juce::MathConstants<float>::twoPi * 1046.50f * time)
                         + 0.30f * std::sin(juce::MathConstants<float>::twoPi * 1567.98f * time)
                         + 0.15f * std::sin(juce::MathConstants<float>::twoPi * 2093.00f * time);

        const float s = wave * env * 0.45f;
        l[i] = s;
        r[i] = s;
    }
}

void SoundboardAudioProcessor::generateAirHornSample(juce::AudioBuffer<float>& buf, double sr)
{
    const int numSamples = static_cast<int>(sr * 1.0); // 1.0s airhorn
    buf.setSize(2, numSamples);
    buf.clear();

    float* l = buf.getWritePointer(0);
    float* r = buf.getWritePointer(1);

    for (int i = 0; i < numSamples; ++i)
    {
        const float time = static_cast<float>(i) / static_cast<float>(sr);
        const float blastTime = std::fmod(time, 0.22f);
        const float blastEnv = (blastTime < 0.18f) ? 1.0f : std::max(0.0f, 1.0f - (blastTime - 0.18f) / 0.04f);
        const float globalEnv = std::exp(-time * 1.8f);

        // Brass chord
        const float wave = 0.4f * std::sin(juce::MathConstants<float>::twoPi * 466.16f * time)
                         + 0.35f * std::sin(juce::MathConstants<float>::twoPi * 587.33f * time)
                         + 0.25f * std::sin(juce::MathConstants<float>::twoPi * 698.46f * time);

        const float s = wave * blastEnv * globalEnv * 0.4f;
        l[i] = s;
        r[i] = s;
    }
}

void SoundboardAudioProcessor::generateImpactHitSample(juce::AudioBuffer<float>& buf, double sr)
{
    const int numSamples = static_cast<int>(sr * 1.6); // 1.6s punchy sub-drop
    buf.setSize(2, numSamples);
    buf.clear();

    std::mt19937 rng(999);
    std::uniform_real_distribution<float> dist(-1.0f, 1.0f);

    float* l = buf.getWritePointer(0);
    float* r = buf.getWritePointer(1);

    for (int i = 0; i < numSamples; ++i)
    {
        const float time = static_cast<float>(i) / static_cast<float>(sr);
        const float subPitch = 100.0f * std::exp(-time * 10.0f) + 40.0f;
        const float subWave = std::sin(juce::MathConstants<float>::twoPi * subPitch * time);
        const float noise = dist(rng) * std::exp(-time * 20.0f);

        const float env = std::exp(-time * 2.5f);
        const float s = (subWave * 0.75f + noise * 0.25f) * env * 0.55f;

        l[i] = s;
        r[i] = s;
    }
}

void SoundboardAudioProcessor::generateBuzzerSample(juce::AudioBuffer<float>& buf, double sr)
{
    // Musical Game-Show "Wrong" Tone (Two descending minor chimes: Eb4 -> C4)
    const int numSamples = static_cast<int>(sr * 1.0);
    buf.setSize(2, numSamples);
    buf.clear();

    float* l = buf.getWritePointer(0);
    float* r = buf.getWritePointer(1);

    for (int i = 0; i < numSamples; ++i)
    {
        const float time = static_cast<float>(i) / static_cast<float>(sr);
        float s = 0.0f;

        if (time < 0.35f)
        {
            // Note 1: Eb4 (311.13 Hz)
            const float env1 = std::exp(-time * 6.0f);
            s = std::sin(juce::MathConstants<float>::twoPi * 311.13f * time) * env1;
        }
        else
        {
            // Note 2: C4 (261.63 Hz)
            const float t2 = time - 0.35f;
            const float env2 = std::exp(-t2 * 4.0f);
            s = std::sin(juce::MathConstants<float>::twoPi * 261.63f * t2) * env2;
        }

        const float sample = s * 0.45f;
        l[i] = sample;
        r[i] = sample;
    }
}

void SoundboardAudioProcessor::generateCheerSample(juce::AudioBuffer<float>& buf, double sr)
{
    const int numSamples = static_cast<int>(sr * 2.0); // 2.0s victory fanfare
    buf.setSize(2, numSamples);
    buf.clear();

    std::mt19937 rng(777);
    std::uniform_real_distribution<float> dist(-1.0f, 1.0f);

    float* l = buf.getWritePointer(0);
    float* r = buf.getWritePointer(1);

    for (int i = 0; i < numSamples; ++i)
    {
        const float time = static_cast<float>(i) / static_cast<float>(sr);
        const float env = (time < 0.15f) ? (time / 0.15f) : std::exp(-(time - 0.15f) * 1.5f);
        
        // Victory Fanfare (C5 - G5 - C6)
        const float horn = 0.4f * std::sin(juce::MathConstants<float>::twoPi * 523.25f * time)
                         + 0.35f * std::sin(juce::MathConstants<float>::twoPi * 783.99f * time)
                         + 0.25f * std::sin(juce::MathConstants<float>::twoPi * 1046.50f * time);

        const float cheerNoise = dist(rng) * 0.2f;
        const float s = (horn * 0.75f + cheerNoise * 0.25f) * env * 0.4f;

        l[i] = s;
        r[i] = s;
    }
}
