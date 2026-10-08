#include "VocalRangeDetector.h"
#include <algorithm>

VocalRangeDetector::VocalRangeDetector()
{
    ringBuffer.resize(ANALYSIS_WINDOW * 2, 0.0f);
    loadProfileFromDisk();
}

void VocalRangeDetector::prepare(double sampleRate, int /*samplesPerBlock*/)
{
    currentSampleRate = (sampleRate > 1000.0) ? sampleRate : 44100.0;
    ringBuffer.assign(ANALYSIS_WINDOW * 2, 0.0f);
    ringWritePos = 0;
    samplesSinceLastDetect = 0;
}

void VocalRangeDetector::reset()
{
    ringBuffer.assign(ANALYSIS_WINDOW * 2, 0.0f);
    ringWritePos = 0;
    samplesSinceLastDetect = 0;
    atomicLiveHz.store(0.0f, std::memory_order_release);
    atomicLiveMidi.store(0, std::memory_order_release);
    atomicLiveConfidence.store(0.0f, std::memory_order_release);
    atomicLiveActive.store(false, std::memory_order_release);
}

void VocalRangeDetector::startScan(float durationSeconds)
{
    float duration = std::clamp(durationSeconds, 5.0f, 30.0f);
    scanTotalSamplesTarget = static_cast<int>(currentSampleRate * duration);
    scanRecordedSamples = 0;
    tempScanLowestMidi = 127;
    tempScanHighestMidi = 0;
    validPitchesSampled = 0;
    scanProgress.store(0.0f, std::memory_order_release);
    isScanningActive.store(true, std::memory_order_release);
}

float VocalRangeDetector::getRemainingScanSeconds() const noexcept
{
    if (!isScanningActive.load(std::memory_order_relaxed))
        return 0.0f;
    int remainingSamples = std::max(0, scanTotalSamplesTarget - scanRecordedSamples);
    return static_cast<float>(remainingSamples) / static_cast<float>(currentSampleRate);
}

void VocalRangeDetector::stopScan()
{
    if (isScanningActive.load(std::memory_order_relaxed))
    {
        isScanningActive.store(false, std::memory_order_release);
        scanProgress.store(1.0f, std::memory_order_release);

        if (validPitchesSampled >= 4 && tempScanLowestMidi <= tempScanHighestMidi)
        {
            profile.lowestMidi = std::clamp(tempScanLowestMidi, 36, 84);   // C2 to C6
            profile.highestMidi = std::clamp(tempScanHighestMidi, 48, 96); // C3 to C7
            profile.isCalibrated = true;
            updateClassification();
            saveProfileToDisk();

            if (onProfileUpdated)
                onProfileUpdated(profile);
        }
    }
}


void VocalRangeDetector::setCustomRange(int lowestMidi, int highestMidi)
{
    profile.lowestMidi = std::clamp(lowestMidi, 36, 84);
    profile.highestMidi = std::clamp(highestMidi, profile.lowestMidi + 4, 96);
    profile.isCalibrated = true;
    updateClassification();
    saveProfileToDisk();

    if (onProfileUpdated)
        onProfileUpdated(profile);
}

void VocalRangeDetector::updateClassification()
{
    profile.vocalClassName = classifyVocalTypeName(profile.lowestMidi, profile.highestMidi, profile.vocalClass);
}

VocalRangeDetector::RealtimePitch VocalRangeDetector::getLivePitch() const
{
    RealtimePitch rp;
    rp.currentHz = atomicLiveHz.load(std::memory_order_relaxed);
    rp.currentMidi = atomicLiveMidi.load(std::memory_order_relaxed);
    rp.confidence = atomicLiveConfidence.load(std::memory_order_relaxed);
    rp.isVoiceActive = atomicLiveActive.load(std::memory_order_relaxed);
    rp.noteName = (rp.isVoiceActive && rp.currentMidi > 0) ? midiToNoteName(rp.currentMidi) : juce::String("--");
    return rp;
}

void VocalRangeDetector::processBlock(const juce::AudioBuffer<float>& buffer)
{
    const int numSamples = buffer.getNumSamples();
    if (numSamples <= 0)
        return;

    const bool scanning = isScanningActive.load(std::memory_order_relaxed);
    const bool liveTracking = liveTrackingEnabled.load(std::memory_order_relaxed);

    if (!scanning && !liveTracking)
        return;

    const float* channelData = buffer.getReadPointer(0);

    for (int i = 0; i < numSamples; ++i)
    {
        ringBuffer[static_cast<size_t>(ringWritePos)] = channelData[i];
        ringWritePos = (ringWritePos + 1) % static_cast<int>(ringBuffer.size());
        samplesSinceLastDetect++;

        // Detect pitch every ~512 samples (~11.6ms at 44.1kHz)
        if (samplesSinceLastDetect >= 512)
        {
            samplesSinceLastDetect = 0;
            detectPitchFromWindow();
        }
    }

    if (scanning)
    {
        scanRecordedSamples += numSamples;
        float progress = std::clamp(static_cast<float>(scanRecordedSamples) / static_cast<float>(scanTotalSamplesTarget), 0.0f, 1.0f);
        scanProgress.store(progress, std::memory_order_release);

        if (scanRecordedSamples >= scanTotalSamplesTarget)
        {
            stopScan();
        }
    }
}

void VocalRangeDetector::detectPitchFromWindow()
{
    // Extract linear window from circular buffer
    std::array<float, ANALYSIS_WINDOW> window{};
    int startIdx = (ringWritePos - ANALYSIS_WINDOW + static_cast<int>(ringBuffer.size())) % static_cast<int>(ringBuffer.size());

    double sumSq = 0.0;
    for (int i = 0; i < ANALYSIS_WINDOW; ++i)
    {
        float s = ringBuffer[static_cast<size_t>((startIdx + i) % static_cast<int>(ringBuffer.size()))];
        window[static_cast<size_t>(i)] = s;
        sumSq += s * s;
    }

    float rms = static_cast<float>(std::sqrt(sumSq / ANALYSIS_WINDOW));
    float rmsDb = 20.0f * std::log10(std::max(1e-5f, rms));

    // Silence or low noise threshold
    if (rmsDb < -42.0f)
    {
        atomicLiveActive.store(false, std::memory_order_release);
        consecutivePitchMatches = 0;
        return;
    }

    // Pitch Range: 65 Hz (C2, lag ~ 678 at 44.1k) to 1050 Hz (C6, lag ~ 42 at 44.1k)
    int minLag = std::max(20, static_cast<int>(currentSampleRate / 1050.0));
    int maxLag = std::min(ANALYSIS_WINDOW / 2, static_cast<int>(currentSampleRate / 65.0));

    double energy0 = 0.0;
    for (int i = 0; i < ANALYSIS_WINDOW / 2; ++i)
    {
        energy0 += window[static_cast<size_t>(i)] * window[static_cast<size_t>(i)];
    }

    if (energy0 < 1e-6)
    {
        atomicLiveActive.store(false, std::memory_order_release);
        return;
    }

    float bestCorr = -1.0f;
    int bestLag = -1;

    for (int lag = minLag; lag <= maxLag; ++lag)
    {
        double corr = 0.0;
        double lagEnergy = 0.0;
        for (int i = 0; i < ANALYSIS_WINDOW / 2; i += 2) // Step 2 for high performance
        {
            float x1 = window[static_cast<size_t>(i)];
            float x2 = window[static_cast<size_t>(i + lag)];
            corr += x1 * x2;
            lagEnergy += x2 * x2;
        }

        double norm = std::sqrt(energy0 * lagEnergy);
        if (norm > 1e-6)
        {
            float ncorr = static_cast<float>(corr / norm);
            if (ncorr > bestCorr)
            {
                bestCorr = ncorr;
                bestLag = lag;
            }
        }
    }

    // Confidence threshold for voiced pitch
    if (bestCorr > 0.60f && bestLag > 0)
    {
        // Parabolic Interpolation around peak
        float refinedLag = static_cast<float>(bestLag);
        if (bestLag > minLag && bestLag < maxLag)
        {
            // Simple 3-point parabolic refinement
            double cL = 0.0, cR = 0.0;
            for (int i = 0; i < ANALYSIS_WINDOW / 2; i += 2)
            {
                cL += window[static_cast<size_t>(i)] * window[static_cast<size_t>(i + bestLag - 1)];
                cR += window[static_cast<size_t>(i)] * window[static_cast<size_t>(i + bestLag + 1)];
            }
            double denom = (2.0 * bestCorr - cL - cR);
            if (std::abs(denom) > 1e-5)
            {
                float delta = static_cast<float>((cL - cR) / (2.0 * denom));
                refinedLag += std::clamp(delta, -0.5f, 0.5f);
            }
        }

        float hz = static_cast<float>(currentSampleRate / refinedLag);
        int midi = std::round(frequencyToMidi(hz));

        if (midi >= 36 && midi <= 96)
        {
            atomicLiveHz.store(hz, std::memory_order_release);
            atomicLiveMidi.store(midi, std::memory_order_release);
            atomicLiveConfidence.store(bestCorr, std::memory_order_release);
            atomicLiveActive.store(true, std::memory_order_release);

            // Stability filter: requires 2 matching frames
            if (std::abs(midi - lastDetectedMidi) <= 1)
            {
                consecutivePitchMatches++;
            }
            else
            {
                consecutivePitchMatches = 1;
                lastDetectedMidi = midi;
            }

            if (consecutivePitchMatches >= 2)
            {
                // If scanning is active, record boundaries and live-update profile
                if (isScanningActive.load(std::memory_order_relaxed))
                {
                    bool changed = false;
                    if (midi < tempScanLowestMidi) { tempScanLowestMidi = midi; changed = true; }
                    if (midi > tempScanHighestMidi) { tempScanHighestMidi = midi; changed = true; }
                    validPitchesSampled++;

                    if (changed && validPitchesSampled >= 2)
                    {
                        profile.lowestMidi = std::clamp(tempScanLowestMidi, 36, 84);
                        profile.highestMidi = std::clamp(tempScanHighestMidi, profile.lowestMidi + 2, 96);
                        updateClassification();
                    }
                }
            }
            return;

        }
    }

    atomicLiveActive.store(false, std::memory_order_release);
}

juce::String VocalRangeDetector::midiToNoteName(int midiNote)
{
    if (midiNote < 0 || midiNote > 127)
        return "--";

    static const char* noteNames[] = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };
    int noteIndex = midiNote % 12;
    int octave = (midiNote / 12) - 1;

    return juce::String(noteNames[noteIndex]) + juce::String(octave);
}

int VocalRangeDetector::noteNameToMidi(const juce::String& noteName)
{
    juce::String s = noteName.trim().toUpperCase();
    if (s.isEmpty())
        return 60; // Middle C

    static const char* names[] = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };
    int noteIdx = -1;
    int octaveStart = 1;

    if (s.startsWith("DB")) { noteIdx = 1; octaveStart = 2; }
    else if (s.startsWith("EB")) { noteIdx = 3; octaveStart = 2; }
    else if (s.startsWith("GB")) { noteIdx = 6; octaveStart = 2; }
    else if (s.startsWith("AB")) { noteIdx = 8; octaveStart = 2; }
    else if (s.startsWith("BB")) { noteIdx = 10; octaveStart = 2; }
    else
    {
        for (int i = 11; i >= 0; --i)
        {
            if (s.startsWith(names[i]))
            {
                noteIdx = i;
                octaveStart = static_cast<int>(strlen(names[i]));
                break;
            }
        }
    }

    if (noteIdx < 0)
        return 60;

    int octave = s.substring(octaveStart).getIntValue();
    return (octave + 1) * 12 + noteIdx;
}

float VocalRangeDetector::midiToFrequency(int midiNote) noexcept
{
    return 440.0f * std::pow(2.0f, (static_cast<float>(midiNote) - 69.0f) / 12.0f);
}

float VocalRangeDetector::frequencyToMidi(float hz) noexcept
{
    if (hz <= 1.0f)
        return 0.0f;
    return 69.0f + 12.0f * std::log2(hz / 440.0f);
}

juce::String VocalRangeDetector::classifyVocalTypeName(int lowestMidi, int highestMidi, VocalClass& outClass)
{
    // Midpoint note index
    int center = (lowestMidi + highestMidi) / 2;

    if (center < 56 || (lowestMidi <= 45 && highestMidi <= 65))
    {
        outClass = VocalClass::MaleBaritone;
        return juce::String::fromUTF8(u8"Nam Trầm (Baritone/Bass)");
    }
    else if (center < 63 || (lowestMidi < 53 && highestMidi <= 73))
    {
        outClass = VocalClass::MaleTenor;
        return juce::String::fromUTF8(u8"Nam Cao (Tenor)");
    }
    else if (center < 69 || (highestMidi <= 79))
    {
        outClass = VocalClass::FemaleAlto;
        return juce::String::fromUTF8(u8"Nữ Trung (Alto/Mezzo)");
    }
    else
    {
        outClass = VocalClass::FemaleSoprano;
        return juce::String::fromUTF8(u8"Nữ Cao (Soprano)");
    }
}

juce::File VocalRangeDetector::getProfileFile() const
{
    auto dir = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
        .getChildFile("HosiStudio")
        .getChildFile("HosiMicroDAW");
    if (!dir.exists())
        dir.createDirectory();
    return dir.getChildFile("vocal_range_profile.json");
}

void VocalRangeDetector::loadProfileFromDisk()
{
    auto file = getProfileFile();
    if (file.existsAsFile())
    {
        auto parsed = juce::JSON::parse(file);
        if (parsed.isObject())
        {
            profile.lowestMidi = std::clamp(static_cast<int>(parsed["lowest_midi"]), 36, 84);
            profile.highestMidi = std::clamp(static_cast<int>(parsed["highest_midi"]), profile.lowestMidi + 4, 96);
            profile.isCalibrated = static_cast<bool>(parsed["is_calibrated"]);
            updateClassification();
            return;
        }
    }

    // Default: Tenor C3 (48) - A4 (69)
    profile.lowestMidi = 48;
    profile.highestMidi = 69;
    profile.isCalibrated = false;
    updateClassification();
}

void VocalRangeDetector::saveProfileToDisk()
{
    auto file = getProfileFile();
    auto* obj = new juce::DynamicObject();
    obj->setProperty("lowest_midi", profile.lowestMidi);
    obj->setProperty("highest_midi", profile.highestMidi);
    obj->setProperty("is_calibrated", profile.isCalibrated);
    obj->setProperty("vocal_class", static_cast<int>(profile.vocalClass));
    obj->setProperty("vocal_class_name", profile.vocalClassName);

    juce::var v(obj);
    juce::String jsonStr = juce::JSON::toString(v);
    file.replaceWithText(jsonStr);
}
