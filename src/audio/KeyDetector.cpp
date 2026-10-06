#include "KeyDetector.h"
#include <numeric>
#include <algorithm>

namespace
{
    // Temperley-Gomez Cognitive Key Profiles (Balanced for modern pop/rock/vocal harmony)
    constexpr std::array<float, 12> MAJOR_PROFILE = {
        5.0f, 1.4f, 2.8f, 1.4f, 4.6f, 3.2f, 1.4f, 4.8f, 1.4f, 2.8f, 1.4f, 3.6f
    };

    // Minor Profile (Natural & Harmonic Minor hybrid with strong Minor 3rd & Perfect 5th)
    constexpr std::array<float, 12> MINOR_PROFILE = {
        5.0f, 1.4f, 2.8f, 4.8f, 1.4f, 3.2f, 1.4f, 4.8f, 3.2f, 1.8f, 3.5f, 2.5f
    };

    const std::array<juce::String, 12> NOTE_NAMES = {
        "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"
    };
}

KeyDetector::KeyDetector()
{
    fifo.resize(FFT_SIZE, 0.0f);
    reset();
}

void KeyDetector::prepare(double sampleRate, int /*samplesPerBlock*/)
{
    currentSampleRate = (sampleRate > 8000.0) ? sampleRate : 44100.0;
    initializePitchLookup(currentSampleRate);
    reset();
}

void KeyDetector::reset()
{
    std::fill(fifo.begin(), fifo.end(), 0.0f);
    fifoIndex = 0;
    std::fill(fftData.begin(), fftData.end(), 0.0f);
    accumulatedChroma.fill(0.0f);
    instantChromaSmooth.fill(0.0f);
    framesProcessed = 0;
    candidateRoot = -1;
    candidateScale = ScaleType::Unknown;
    consecutiveStableFrames = 0;
    keyLocked = false;

    const juce::ScopedLock sl(resultLock);
    latestResult = KeyResult{};
}

void KeyDetector::unlock()
{
    keyLocked = false;
    consecutiveStableFrames = 0;
    const juce::ScopedLock sl(resultLock);
    latestResult.isLocked = false;
}

void KeyDetector::initializePitchLookup(double sampleRate)
{
    const int halfFft = FFT_SIZE / 2;
    const float binFreq = static_cast<float>(sampleRate) / static_cast<float>(FFT_SIZE);

    for (int bin = 1; bin < halfFft; ++bin)
    {
        const float freq = bin * binFreq;
        // Pitch analysis range: 75 Hz (D#2) to 3400 Hz (A7)
        if (freq >= 75.0f && freq <= 3400.0f)
        {
            const float midiNote = 69.0f + 12.0f * std::log2(freq / 440.0f);
            const int roundedMidi = static_cast<int>(std::round(midiNote));
            const float deviation = std::abs(midiNote - static_cast<float>(roundedMidi)); // 0.0 to 0.5

            int pitchClass = roundedMidi % 12;
            if (pitchClass < 0) pitchClass += 12;

            binToMidiNote[bin] = midiNote;
            binToPitchClass[bin] = pitchClass;

            // Musical harmonic weighting: emphasize fundamental voice/chords octave (120 Hz - 1400 Hz)
            float freqWeight = 1.0f;
            if (freq >= 120.0f && freq <= 1400.0f)
                freqWeight = 1.8f;
            else if (freq < 100.0f)
                freqWeight = 0.5f; // attenuates kick drum sub-bass rumble

            // Gaussian tuning center weight
            const float tuneWeight = std::exp(-0.5f * std::pow(deviation / 0.22f, 2.0f));
            binWeight[bin] = freqWeight * tuneWeight;
        }
        else
        {
            binToMidiNote[bin] = -1.0f;
            binToPitchClass[bin] = -1;
            binWeight[bin] = 0.0f;
        }
    }
}

void KeyDetector::processBlock(const juce::AudioBuffer<float>& buffer)
{
    const int numSamples = buffer.getNumSamples();
    const int numChannels = buffer.getNumChannels();
    if (numSamples <= 0 || numChannels <= 0)
        return;

    const float* const ch0 = buffer.getReadPointer(0);
    const float* const ch1 = (numChannels > 1) ? buffer.getReadPointer(1) : ch0;

    for (int i = 0; i < numSamples; ++i)
    {
        // Downmix to mono
        const float sample = 0.5f * (ch0[i] + ch1[i]);
        fifo[fifoIndex++] = sample;

        if (fifoIndex >= FFT_SIZE)
        {
            processFFTFrame();
            
            // 50% Hop forward
            std::copy(fifo.begin() + HOP_SIZE, fifo.end(), fifo.begin());
            fifoIndex = FFT_SIZE - HOP_SIZE;
        }
    }
}

void KeyDetector::processFFTFrame()
{
    std::copy(fifo.begin(), fifo.end(), fftData.begin());
    std::fill(fftData.begin() + FFT_SIZE, fftData.end(), 0.0f);

    window.multiplyWithWindowingTable(fftData.data(), FFT_SIZE);
    fft.performFrequencyOnlyForwardTransform(fftData.data());

    std::array<float, 12> instantChroma{};
    const int halfFft = FFT_SIZE / 2;

    // Peak-picking spectral energy
    for (int bin = 2; bin < halfFft - 1; ++bin)
    {
        const int pitchClass = binToPitchClass[bin];
        if (pitchClass >= 0)
        {
            const float mag = fftData[bin];
            // Spectral peak detection (local maximum to ignore flat background noise)
            if (mag > fftData[bin - 1] && mag > fftData[bin + 1] && mag > 0.005f)
            {
                const float energy = mag * mag * binWeight[bin];
                instantChroma[pitchClass] += energy;
            }
        }
    }

    // Update fast visualizer chroma
    for (int c = 0; c < 12; ++c)
    {
        instantChromaSmooth[c] = instantChromaSmooth[c] * 0.7f + instantChroma[c] * 0.3f;
    }

    // Long-term integration for stable Key estimation (decay factor 0.985 = ~15s effective window)
    constexpr float decayFactor = 0.985f;
    for (int c = 0; c < 12; ++c)
    {
        accumulatedChroma[c] = accumulatedChroma[c] * decayFactor + instantChroma[c] * (1.0f - decayFactor);
    }

    framesProcessed++;

    if (!keyLocked)
    {
        // Evaluate key every 4 frames (~180ms)
        if (framesProcessed % 4 == 0)
        {
            auto result = calculateKeyFromChroma(accumulatedChroma);

            if (result.scale != ScaleType::Unknown && result.confidence >= 0.60f)
            {
                if (result.rootNote == candidateRoot && result.scale == candidateScale)
                {
                    consecutiveStableFrames++;
                    // Lock after ~3.5 - 4 seconds of consistent agreement with high confidence
                    if (consecutiveStableFrames >= 18 && result.confidence >= 0.75f)
                    {
                        keyLocked = true;
                        result.isLocked = true;
                    }
                }
                else
                {
                    candidateRoot = result.rootNote;
                    candidateScale = result.scale;
                    consecutiveStableFrames = 1;
                }
            }

            const juce::ScopedLock sl(resultLock);
            latestResult = result;
            // Update visualizer profile
            float maxVis = 0.00001f;
            for (float v : instantChromaSmooth) if (v > maxVis) maxVis = v;
            for (int c = 0; c < 12; ++c) latestResult.chromaProfile[c] = std::clamp(instantChromaSmooth[c] / maxVis, 0.0f, 1.0f);
        }
    }
    else
    {
        // When locked, keep the detected Tone rock-solid and only animate the visualizer LEDs
        const juce::ScopedLock sl(resultLock);
        latestResult.isLocked = true;
        float maxVis = 0.00001f;
        for (float v : instantChromaSmooth) if (v > maxVis) maxVis = v;
        for (int c = 0; c < 12; ++c) latestResult.chromaProfile[c] = std::clamp(instantChromaSmooth[c] / maxVis, 0.0f, 1.0f);
    }
}

KeyDetector::KeyResult KeyDetector::calculateKeyFromChroma(const std::array<float, 12>& chroma) const
{
    KeyResult res;

    // Normalize chroma vector
    float maxEnergy = 0.000001f;
    float sumEnergy = 0.0f;
    for (float v : chroma)
    {
        if (v > maxEnergy) maxEnergy = v;
        sumEnergy += v;
    }

    for (int c = 0; c < 12; ++c)
    {
        res.chromaProfile[c] = std::clamp(chroma[c] / maxEnergy, 0.0f, 1.0f);
    }

    if (sumEnergy < 0.0001f)
    {
        res.keyName = "Listening...";
        res.confidence = 0.0f;
        res.scale = ScaleType::Unknown;
        res.isLocked = false;
        return res;
    }

    // Normalized Chroma profile (0.0 to 1.0)
    std::array<float, 12> normChroma{};
    for (int c = 0; c < 12; ++c)
        normChroma[c] = chroma[c] / maxEnergy;

    // Pearson correlation computation
    auto computeCorrelation = [](const std::array<float, 12>& x, const std::array<float, 12>& y, int shift) -> float
    {
        float meanX = 0.0f, meanY = 0.0f;
        for (int i = 0; i < 12; ++i)
        {
            meanX += x[i];
            meanY += y[(i - shift + 12) % 12];
        }
        meanX /= 12.0f;
        meanY /= 12.0f;

        float num = 0.0f, denX = 0.0f, denY = 0.0f;
        for (int i = 0; i < 12; ++i)
        {
            const float diffX = x[i] - meanX;
            const float diffY = y[(i - shift + 12) % 12] - meanY;
            num += diffX * diffY;
            denX += diffX * diffX;
            denY += diffY * diffY;
        }

        const float den = std::sqrt(denX * denY);
        return (den > 0.00001f) ? (num / den) : 0.0f;
    };

    float bestScore = -10.0f;
    float secondBestScore = -10.0f;
    int bestRoot = 0;
    ScaleType bestScale = ScaleType::Major;

    // Evaluate 12 Major Keys
    for (int shift = 0; shift < 12; ++shift)
    {
        const float r = computeCorrelation(chroma, MAJOR_PROFILE, shift);
        
        // Triad verification: Root (shift), Major 3rd (shift + 4), Perfect 5th (shift + 7)
        const float rootE = normChroma[shift];
        const float thirdE = normChroma[(shift + 4) % 12];
        const float fifthE = normChroma[(shift + 7) % 12];
        
        const float triadScore = (rootE * 1.2f + thirdE * 1.5f + fifthE * 1.0f) / 3.7f;
        
        // Major key MUST have an active Major 3rd; otherwise apply heavy penalty
        float thirdPenalty = 0.0f;
        if (thirdE < 0.22f)
            thirdPenalty = -0.40f * (1.0f - thirdE / 0.22f);

        // Combined Score: 55% Pearson Correlation + 45% Triad Verification + Penalty
        const float totalScore = 0.55f * r + 0.45f * triadScore + thirdPenalty;

        if (totalScore > bestScore)
        {
            secondBestScore = bestScore;
            bestScore = totalScore;
            bestRoot = shift;
            bestScale = ScaleType::Major;
        }
        else if (totalScore > secondBestScore)
        {
            secondBestScore = totalScore;
        }
    }

    // Evaluate 12 Minor Keys
    for (int shift = 0; shift < 12; ++shift)
    {
        const float r = computeCorrelation(chroma, MINOR_PROFILE, shift);
        
        // Triad verification: Root (shift), Minor 3rd (shift + 3), Perfect 5th (shift + 7)
        const float rootE = normChroma[shift];
        const float thirdE = normChroma[(shift + 3) % 12];
        const float fifthE = normChroma[(shift + 7) % 12];
        
        const float triadScore = (rootE * 1.2f + thirdE * 1.5f + fifthE * 1.0f) / 3.7f;
        
        // Minor key MUST have an active Minor 3rd; otherwise apply heavy penalty
        float thirdPenalty = 0.0f;
        if (thirdE < 0.22f)
            thirdPenalty = -0.40f * (1.0f - thirdE / 0.22f);

        // Combined Score: 55% Pearson Correlation + 45% Triad Verification + Penalty
        const float totalScore = 0.55f * r + 0.45f * triadScore + thirdPenalty;

        if (totalScore > bestScore)
        {
            secondBestScore = bestScore;
            bestScore = totalScore;
            bestRoot = shift;
            bestScale = ScaleType::Minor;
        }
        else if (totalScore > secondBestScore)
        {
            secondBestScore = totalScore;
        }
    }

    res.rootNote = bestRoot;
    res.scale = bestScale;
    
    // Confidence calculation based on combined score and margin of victory
    const float scoreFactor = std::clamp((bestScore - 0.25f) / 0.65f, 0.0f, 1.0f);
    const float separationFactor = std::clamp((bestScore - secondBestScore) / 0.15f, 0.0f, 1.0f);
    res.confidence = std::clamp(scoreFactor * 0.55f + separationFactor * 0.45f, 0.0f, 0.99f);

    res.keyName = formatKeyName(bestRoot, bestScale);
    res.isLocked = keyLocked;
    return res;
}

KeyDetector::KeyResult KeyDetector::analyzeBufferOffline(const juce::AudioBuffer<float>& buffer, double sampleRate)
{
    prepare(sampleRate, 512);
    reset();

    const int totalSamples = buffer.getNumSamples();
    const int numChannels = buffer.getNumChannels();
    if (totalSamples < 4096 || numChannels <= 0)
        return getCurrentResult();

    std::array<float, 12> offlineChromaTotal{};
    std::vector<float> frameBuffer(FFT_SIZE, 0.0f);
    std::array<float, FFT_SIZE * 2> offlineFftData{};

    const float* const ch0 = buffer.getReadPointer(0);
    const float* const ch1 = (numChannels > 1) ? buffer.getReadPointer(1) : ch0;

    const int halfFft = FFT_SIZE / 2;
    int validFrames = 0;

    // Scan up to 100 seconds across the audio track with HOP_SIZE step
    const int maxSamples = std::min(totalSamples, static_cast<int>(sampleRate * 100.0));
    for (int offset = 0; offset + FFT_SIZE <= maxSamples; offset += HOP_SIZE)
    {
        for (int i = 0; i < FFT_SIZE; ++i)
        {
            frameBuffer[i] = 0.5f * (ch0[offset + i] + ch1[offset + i]);
            offlineFftData[i] = frameBuffer[i];
        }
        std::fill(offlineFftData.begin() + FFT_SIZE, offlineFftData.end(), 0.0f);

        window.multiplyWithWindowingTable(offlineFftData.data(), FFT_SIZE);
        fft.performFrequencyOnlyForwardTransform(offlineFftData.data());

        // Peak energy summation
        for (int bin = 2; bin < halfFft - 1; ++bin)
        {
            const int pitchClass = binToPitchClass[bin];
            if (pitchClass >= 0)
            {
                const float mag = offlineFftData[bin];
                if (mag > offlineFftData[bin - 1] && mag > offlineFftData[bin + 1] && mag > 0.005f)
                {
                    const float energy = mag * mag * binWeight[bin];
                    offlineChromaTotal[pitchClass] += energy;
                }
            }
        }
        validFrames++;
    }

    auto finalResult = calculateKeyFromChroma(offlineChromaTotal);
    finalResult.isLocked = true;
    finalResult.confidence = std::max(finalResult.confidence, 0.90f); // High confidence from full song scan

    keyLocked = true;
    candidateRoot = finalResult.rootNote;
    candidateScale = finalResult.scale;
    consecutiveStableFrames = 50;

    const juce::ScopedLock sl(resultLock);
    latestResult = finalResult;
    return finalResult;
}

KeyDetector::KeyResult KeyDetector::getCurrentResult() const
{
    const juce::ScopedLock sl(resultLock);
    return latestResult;
}

juce::String KeyDetector::getNoteName(int noteIndex)
{
    if (noteIndex >= 0 && noteIndex < 12)
        return NOTE_NAMES[static_cast<size_t>(noteIndex)];
    return "C";
}

juce::String KeyDetector::formatKeyName(int rootNote, ScaleType scale)
{
    const juce::String root = getNoteName(rootNote);
    if (scale == ScaleType::Major)
        return root + " Major";
    if (scale == ScaleType::Minor)
        return root + " Minor";
    return "--";
}
