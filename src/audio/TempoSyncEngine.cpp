#include "TempoSyncEngine.h"
#include <cmath>
#include <numeric>
#include <algorithm>

TempoSyncEngine::TempoSyncEngine()
{
    latestBeatResult.estimatedBpm = 120.0;
    latestBeatResult.confidence = 0.0f;
    latestBeatResult.isStable = false;
}

void TempoSyncEngine::setBpm(double newBpm, const juce::String& source)
{
    double clampedBpm = std::clamp(newBpm, 40.0, 260.0);
    currentBpm.store(clampedBpm, std::memory_order_release);

    {
        const juce::ScopedLock sl(stateLock);
        lastBpmSource = source;
    }

    sendChangeMessage();
}

juce::String TempoSyncEngine::getBpmSource() const
{
    const juce::ScopedLock sl(stateLock);
    return lastBpmSource;
}

void TempoSyncEngine::tapTempo()
{
    auto now = std::chrono::steady_clock::now();

    {
        const juce::ScopedLock sl(stateLock);

        if (!tapHistory.empty())
        {
            auto elapsedMs = std::chrono::duration_cast<std::chrono::milliseconds>(now - tapHistory.back()).count();
            if (elapsedMs > TAP_TIMEOUT_MS)
            {
                tapHistory.clear();
            }
        }

        tapHistory.push_back(now);
        if (tapHistory.size() > MAX_TAP_HISTORY)
        {
            tapHistory.erase(tapHistory.begin());
        }

        if (tapHistory.size() >= 2)
        {
            std::vector<double> intervals;
            for (size_t i = 1; i < tapHistory.size(); ++i)
            {
                auto diff = std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(tapHistory[i] - tapHistory[i - 1]).count();
                if (diff >= 230.0 && diff <= 1500.0) // 40 to 260 BPM
                {
                    intervals.push_back(diff);
                }
            }

            if (!intervals.empty())
            {
                double avgIntervalMs = std::accumulate(intervals.begin(), intervals.end(), 0.0) / static_cast<double>(intervals.size());
                double calculatedBpm = 60000.0 / avgIntervalMs;
                setBpm(std::round(calculatedBpm), "Tap Tempo");
                return;
            }
        }
    }
}

void TempoSyncEngine::resetTapHistory()
{
    const juce::ScopedLock sl(stateLock);
    tapHistory.clear();
}

float TempoSyncEngine::calculateDelayTimeMs(double bpm, DelaySubdivision subdivision)
{
    if (bpm <= 0.0) bpm = 120.0;
    const double quarterMs = 60000.0 / bpm;

    switch (subdivision)
    {
    case DelaySubdivision::Quarter:       return static_cast<float>(quarterMs);
    case DelaySubdivision::DottedEighth:  return static_cast<float>(quarterMs * 0.75);
    case DelaySubdivision::Eighth:        return static_cast<float>(quarterMs * 0.5);
    case DelaySubdivision::TripletEighth: return static_cast<float>(quarterMs * (1.0 / 3.0));
    case DelaySubdivision::Sixteenth:     return static_cast<float>(quarterMs * 0.25);
    case DelaySubdivision::Half:          return static_cast<float>(quarterMs * 2.0);
    case DelaySubdivision::Free:
    default:                              return 260.0f;
    }
}

float TempoSyncEngine::calculateReverbDecaySec(double bpm, ReverbBarLength barLength)
{
    if (bpm <= 0.0) bpm = 120.0;
    const double quarterSec = 60.0 / bpm;
    const double barSec = quarterSec * 4.0; // 4/4 Bar

    switch (barLength)
    {
    case ReverbBarLength::HalfBar:  return static_cast<float>(barSec * 0.5); // e.g. 1.0s @ 120BPM
    case ReverbBarLength::OneBar:   return static_cast<float>(barSec * 1.0); // e.g. 2.0s @ 120BPM (Studio Clean)
    case ReverbBarLength::TwoBars:  return static_cast<float>(barSec * 2.0); // e.g. 4.0s @ 120BPM (Lush Ballad)
    case ReverbBarLength::FourBars: return static_cast<float>(barSec * 4.0); // e.g. 8.0s @ 120BPM (Cathedral)
    case ReverbBarLength::Free:
    default:                        return 2.5f;
    }
}

juce::String TempoSyncEngine::getSubdivisionName(DelaySubdivision subdivision)
{
    switch (subdivision)
    {
    case DelaySubdivision::Quarter:       return "1/4";
    case DelaySubdivision::DottedEighth:  return "1/8D (Dotted)";
    case DelaySubdivision::Eighth:        return "1/8";
    case DelaySubdivision::TripletEighth: return "1/8T (Triplet)";
    case DelaySubdivision::Sixteenth:     return "1/16";
    case DelaySubdivision::Half:          return "1/2";
    case DelaySubdivision::Free:          return "Free";
    default:                              return "1/8D";
    }
}

juce::String TempoSyncEngine::getBarLengthName(ReverbBarLength barLength)
{
    switch (barLength)
    {
    case ReverbBarLength::HalfBar:  return "1/2 Bar";
    case ReverbBarLength::OneBar:   return "1 Bar";
    case ReverbBarLength::TwoBars:  return "2 Bars";
    case ReverbBarLength::FourBars: return "4 Bars";
    case ReverbBarLength::Free:     return "Free";
    default:                        return "1 Bar";
    }
}

void TempoSyncEngine::prepare(double sampleRate, int /*samplesPerBlock*/)
{
    currentSampleRate = (sampleRate > 8000.0) ? sampleRate : 44100.0;
    inputFifo.assign(ONSET_BUFFER_SIZE, 0.0f);
    fifoWritePos = 0;
    previousFrameEnergy = 0.0f;
    onsetFluxRing.fill(0.0f);
    onsetRingPos = 0;
    onsetFrameCounter = 0;
}

void TempoSyncEngine::resetDetector()
{
    const juce::ScopedLock sl(beatLock);
    latestBeatResult.confidence = 0.0f;
    latestBeatResult.isStable = false;
    onsetFluxRing.fill(0.0f);
    onsetRingPos = 0;
    onsetFrameCounter = 0;
}

void TempoSyncEngine::processAudioBlock(const juce::AudioBuffer<float>& buffer)
{
    const int numSamples = buffer.getNumSamples();
    if (numSamples <= 0 || currentSampleRate <= 0.0)
        return;

    const float* const* channelData = buffer.getArrayOfReadPointers();
    const int numChannels = buffer.getNumChannels();

    for (int i = 0; i < numSamples; ++i)
    {
        float sample = 0.0f;
        if (numChannels >= 2)
            sample = (channelData[0][i] + channelData[1][i]) * 0.5f;
        else if (numChannels == 1)
            sample = channelData[0][i];

        inputFifo[static_cast<size_t>(fifoWritePos)] = sample;
        fifoWritePos = (fifoWritePos + 1) % ONSET_BUFFER_SIZE;

        if (fifoWritePos % ONSET_HOP_SIZE == 0)
        {
            processOnsetFrame(inputFifo.data(), ONSET_BUFFER_SIZE);
        }
    }
}

void TempoSyncEngine::processOnsetFrame(const float* data, int numSamples)
{
    // Fast Root-Mean-Square Energy + High-Frequency Content weighting for crisp drum onset detection
    float energy = 0.0f;
    for (int i = 0; i < numSamples; ++i)
    {
        energy += data[i] * data[i];
    }
    energy = std::sqrt(energy / static_cast<float>(numSamples));

    // Half-wave rectified Spectral Energy Difference (Onset Flux)
    float flux = std::max(0.0f, energy - previousFrameEnergy);
    previousFrameEnergy = energy * 0.85f; // Exponential decay

    onsetFluxRing[static_cast<size_t>(onsetRingPos)] = flux;
    onsetRingPos = (onsetRingPos + 1) % ONSET_RING_SIZE;
    ++onsetFrameCounter;

    // Run autocorrelation beat estimation every 16 onset frames (~185ms)
    if (onsetFrameCounter >= 16)
    {
        onsetFrameCounter = 0;
        computeAutocorrelationTempo();
    }
}

void TempoSyncEngine::computeAutocorrelationTempo()
{
    // Frame rate of onsets: Fs / ONSET_HOP_SIZE
    const double onsetFps = currentSampleRate / static_cast<double>(ONSET_HOP_SIZE);
    if (onsetFps <= 0.0) return;

    // We scan BPM range from 60 to 200 BPM
    const int minLag = static_cast<int>((60.0 / 200.0) * onsetFps); // max BPM lag (~25 frames)
    const int maxLag = static_cast<int>((60.0 / 60.0) * onsetFps);  // min BPM lag (~86 frames)

    if (maxLag >= ONSET_RING_SIZE / 2 || minLag < 2)
        return;

    // Unroll ring buffer into continuous array for linear correlation
    std::array<float, ONSET_RING_SIZE> linearBuffer{};
    for (int i = 0; i < ONSET_RING_SIZE; ++i)
    {
        linearBuffer[static_cast<size_t>(i)] = onsetFluxRing[static_cast<size_t>((onsetRingPos + i) % ONSET_RING_SIZE)];
    }

    float bestCorrelation = 0.0f;
    int bestLag = -1;
    float sumEnergy = 0.0f;

    for (int lag = minLag; lag <= maxLag; ++lag)
    {
        float corr = 0.0f;
        int count = 0;
        for (int i = 0; i < ONSET_RING_SIZE - lag; ++i)
        {
            corr += linearBuffer[static_cast<size_t>(i)] * linearBuffer[static_cast<size_t>(i + lag)];
            sumEnergy += linearBuffer[static_cast<size_t>(i)];
            ++count;
        }

        if (count > 0)
        {
            corr /= static_cast<float>(count);
            if (corr > bestCorrelation)
            {
                bestCorrelation = corr;
                bestLag = lag;
            }
        }
    }

    if (bestLag > 0 && bestCorrelation > 0.0001f)
    {
        double estimatedBpm = (60.0 * onsetFps) / static_cast<double>(bestLag);
        
        // Normalize tempo into reasonable singing range (60 - 180 BPM)
        while (estimatedBpm < 60.0) estimatedBpm *= 2.0;
        while (estimatedBpm > 180.0) estimatedBpm /= 2.0;

        float confidence = std::clamp(bestCorrelation * 40.0f, 0.0f, 1.0f);

        const juce::ScopedLock sl(beatLock);
        latestBeatResult.estimatedBpm = std::round(estimatedBpm);
        latestBeatResult.confidence = latestBeatResult.confidence * 0.7f + confidence * 0.3f;
        latestBeatResult.isStable = (latestBeatResult.confidence > 0.65f);
    }
}

TempoSyncEngine::BeatDetectionResult TempoSyncEngine::getDetectedBeatResult() const
{
    const juce::ScopedLock sl(beatLock);
    return latestBeatResult;
}
