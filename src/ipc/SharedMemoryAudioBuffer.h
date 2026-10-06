#pragma once

#include <cstdint>
#include <atomic>
#include <cstring>
#include <algorithm>

#if defined(_WIN32) || defined(_WIN64)
    #ifndef WIN32_LEAN_AND_MEAN
        #define WIN32_LEAN_AND_MEAN
    #endif
    #ifndef NOMINMAX
        #define NOMINMAX
    #endif
    #include <windows.h>
#elif defined(__APPLE__) || defined(__linux__)
    #include <sys/mman.h>
    #include <sys/stat.h>
    #include <fcntl.h>
    #include <unistd.h>
    #include <errno.h>
#endif

namespace LiveStreamIPC
{
#if defined(_WIN32) || defined(_WIN64)
    static constexpr const char* IPC_SHARED_MEMORY_NAME = "Local\\LiveStreamMicroDAW_AudioIPC_v1";
#else
    static constexpr const char* IPC_SHARED_MEMORY_NAME = "/LiveStreamMicroDAW_AudioIPC_v1";
#endif
    static constexpr uint32_t IPC_MAGIC_NUMBER = 0x4D494352; // "MICR"
    static constexpr uint32_t IPC_VERSION = 1;
    static constexpr uint32_t RING_BUFFER_CAPACITY = 16384; // Power of two (plenty of head room)
    static constexpr uint32_t RING_BUFFER_MASK = RING_BUFFER_CAPACITY - 1;
    static constexpr uint32_t MAX_CHANNELS = 2;

    enum class AudioApiMode : uint32_t
    {
        WasapiShared = 0,
        AsioExclusive = 1
    };

    #pragma pack(push, 8)
    struct alignas(64) AudioIpcHeader
    {
        uint32_t magicNumber{ IPC_MAGIC_NUMBER };
        uint32_t version{ IPC_VERSION };
        uint32_t sampleRate{ 48000 };
        uint16_t numChannels{ 2 };
        uint16_t bitDepth{ 32 };
        uint32_t bufferCapacity{ RING_BUFFER_CAPACITY };
        uint32_t bufferMask{ RING_BUFFER_MASK };

        std::atomic<uint32_t> isDawActive{ 0 };
        std::atomic<uint32_t> audioApiMode{ static_cast<uint32_t>(AudioApiMode::WasapiShared) };

        // Cacheline aligned atomic cursors for lock-free SPSC
        alignas(64) std::atomic<uint64_t> writeCursor{ 0 };
        alignas(64) std::atomic<uint64_t> readCursor{ 0 };
    };

    struct SharedAudioMemoryLayout
    {
        AudioIpcHeader header;
        alignas(64) float channelData[MAX_CHANNELS][RING_BUFFER_CAPACITY];
    };
    #pragma pack(pop)

    /**
     * Producer / Sender Interface for the Micro-DAW host.
     */
    class SharedMemoryAudioSender
    {
    public:
        SharedMemoryAudioSender() = default;
        ~SharedMemoryAudioSender() { close(); }

        bool initialize()
        {
#if defined(_WIN32) || defined(_WIN64)
            if (memoryMapHandle != nullptr)
                return true;

            const size_t totalSize = sizeof(SharedAudioMemoryLayout);
            memoryMapHandle = CreateFileMappingA(
                INVALID_HANDLE_VALUE,
                nullptr,
                PAGE_READWRITE,
                0,
                static_cast<DWORD>(totalSize),
                IPC_SHARED_MEMORY_NAME
            );

            if (memoryMapHandle == nullptr)
                return false;

            mappedView = MapViewOfFile(
                memoryMapHandle,
                FILE_MAP_ALL_ACCESS,
                0,
                0,
                totalSize
            );

            if (mappedView == nullptr)
            {
                CloseHandle(memoryMapHandle);
                memoryMapHandle = nullptr;
                return false;
            }

            layout = static_cast<SharedAudioMemoryLayout*>(mappedView);
            layout->header.magicNumber = IPC_MAGIC_NUMBER;
            layout->header.version = IPC_VERSION;
            layout->header.sampleRate = 48000;
            layout->header.numChannels = MAX_CHANNELS;
            layout->header.bufferCapacity = RING_BUFFER_CAPACITY;
            layout->header.bufferMask = RING_BUFFER_MASK;
            layout->header.writeCursor.store(0, std::memory_order_relaxed);
            layout->header.readCursor.store(0, std::memory_order_relaxed);
            layout->header.isDawActive.store(1, std::memory_order_release);

            std::memset(layout->channelData, 0, sizeof(layout->channelData));
            return true;
#elif defined(__APPLE__) || defined(__linux__)
            if (shmFd >= 0)
                return true;

            const size_t totalSize = sizeof(SharedAudioMemoryLayout);
            shmFd = shm_open(IPC_SHARED_MEMORY_NAME, O_RDWR | O_CREAT, 0666);
            if (shmFd < 0)
                return false;

            if (ftruncate(shmFd, static_cast<off_t>(totalSize)) != 0)
            {
                ::close(shmFd);
                shmFd = -1;
                return false;
            }

            mappedView = mmap(nullptr, totalSize, PROT_READ | PROT_WRITE, MAP_SHARED, shmFd, 0);
            if (mappedView == MAP_FAILED || mappedView == nullptr)
            {
                ::close(shmFd);
                shmFd = -1;
                mappedView = nullptr;
                return false;
            }

            layout = static_cast<SharedAudioMemoryLayout*>(mappedView);
            layout->header.magicNumber = IPC_MAGIC_NUMBER;
            layout->header.version = IPC_VERSION;
            layout->header.sampleRate = 48000;
            layout->header.numChannels = MAX_CHANNELS;
            layout->header.bufferCapacity = RING_BUFFER_CAPACITY;
            layout->header.bufferMask = RING_BUFFER_MASK;
            layout->header.writeCursor.store(0, std::memory_order_relaxed);
            layout->header.readCursor.store(0, std::memory_order_relaxed);
            layout->header.isDawActive.store(1, std::memory_order_release);

            std::memset(layout->channelData, 0, sizeof(layout->channelData));
            return true;
#else
            return false;
#endif
        }

        void close()
        {
#if defined(_WIN32) || defined(_WIN64)
            if (layout != nullptr)
            {
                layout->header.isDawActive.store(0, std::memory_order_release);
                UnmapViewOfFile(mappedView);
                mappedView = nullptr;
                layout = nullptr;
            }
            if (memoryMapHandle != nullptr)
            {
                CloseHandle(memoryMapHandle);
                memoryMapHandle = nullptr;
            }
#elif defined(__APPLE__) || defined(__linux__)
            if (layout != nullptr)
            {
                layout->header.isDawActive.store(0, std::memory_order_release);
                munmap(mappedView, sizeof(SharedAudioMemoryLayout));
                mappedView = nullptr;
                layout = nullptr;
            }
            if (shmFd >= 0)
            {
                ::close(shmFd);
                shm_unlink(IPC_SHARED_MEMORY_NAME);
                shmFd = -1;
            }
#endif
        }

        void setAudioApiMode(AudioApiMode mode)
        {
            if (layout != nullptr)
            {
                layout->header.audioApiMode.store(static_cast<uint32_t>(mode), std::memory_order_release);
            }
        }

        void setSampleRate(uint32_t sampleRate)
        {
            if (layout != nullptr)
            {
                layout->header.sampleRate = sampleRate;
            }
        }

        // Lock-free real-time safe audio push (never blocks, continuous streaming)
        void writeAudio(const float* const* channelData, int numChannels, int numSamples) noexcept
        {
            if (layout == nullptr || numSamples <= 0 || channelData == nullptr)
                return;

            const uint64_t w = layout->header.writeCursor.load(std::memory_order_relaxed);
            const int chans = std::min(numChannels, static_cast<int>(MAX_CHANNELS));

            for (int ch = 0; ch < chans; ++ch)
            {
                const float* src = channelData[ch];
                float* dst = layout->channelData[ch];

                if (src != nullptr && dst != nullptr)
                {
                    for (int i = 0; i < numSamples; ++i)
                    {
                        const uint32_t idx = static_cast<uint32_t>((w + static_cast<uint64_t>(i)) & RING_BUFFER_MASK);
                        dst[idx] = src[i];
                    }
                }
            }

            // Duplicate mono to stereo if necessary
            if (chans == 1 && channelData[0] != nullptr)
            {
                const float* src = channelData[0];
                float* dst = layout->channelData[1];
                if (dst != nullptr)
                {
                    for (int i = 0; i < numSamples; ++i)
                    {
                        const uint32_t idx = static_cast<uint32_t>((w + static_cast<uint64_t>(i)) & RING_BUFFER_MASK);
                        dst[idx] = src[i];
                    }
                }
            }

            layout->header.writeCursor.store(w + static_cast<uint64_t>(numSamples), std::memory_order_release);
        }

    private:
#if defined(_WIN32) || defined(_WIN64)
        HANDLE memoryMapHandle{ nullptr };
        void* mappedView{ nullptr };
#elif defined(__APPLE__) || defined(__linux__)
        int shmFd{ -1 };
        void* mappedView{ nullptr };
#endif
        SharedAudioMemoryLayout* layout{ nullptr };
    };

    /**
     * Consumer / Receiver Interface for the OBS Studio VST3 plugin.
     */
    class SharedMemoryAudioReceiver
    {
    public:
        SharedMemoryAudioReceiver() = default;
        ~SharedMemoryAudioReceiver() { close(); }

        bool isConnected() const noexcept
        {
            return layout != nullptr && layout->header.isDawActive.load(std::memory_order_relaxed) != 0;
        }

        bool initialize()
        {
#if defined(_WIN32) || defined(_WIN64)
            if (memoryMapHandle != nullptr && layout != nullptr)
                return true;

            memoryMapHandle = OpenFileMappingA(
                FILE_MAP_READ | FILE_MAP_WRITE,
                FALSE,
                IPC_SHARED_MEMORY_NAME
            );

            if (memoryMapHandle == nullptr)
                return false;

            mappedView = MapViewOfFile(
                memoryMapHandle,
                FILE_MAP_READ | FILE_MAP_WRITE,
                0,
                0,
                sizeof(SharedAudioMemoryLayout)
            );

            if (mappedView == nullptr)
            {
                CloseHandle(memoryMapHandle);
                memoryMapHandle = nullptr;
                return false;
            }

            layout = static_cast<SharedAudioMemoryLayout*>(mappedView);
            if (layout->header.magicNumber != IPC_MAGIC_NUMBER || layout->header.version != IPC_VERSION)
            {
                close();
                return false;
            }

            localReadPos = 0;
            fractionalPhase = 0.0;
            smoothedRatio = 1.0;
            return true;
#elif defined(__APPLE__) || defined(__linux__)
            if (shmFd >= 0 && layout != nullptr)
                return true;

            const size_t totalSize = sizeof(SharedAudioMemoryLayout);
            shmFd = shm_open(IPC_SHARED_MEMORY_NAME, O_RDWR, 0666);
            if (shmFd < 0)
                return false;

            mappedView = mmap(nullptr, totalSize, PROT_READ | PROT_WRITE, MAP_SHARED, shmFd, 0);
            if (mappedView == MAP_FAILED || mappedView == nullptr)
            {
                ::close(shmFd);
                shmFd = -1;
                mappedView = nullptr;
                return false;
            }

            layout = static_cast<SharedAudioMemoryLayout*>(mappedView);
            if (layout->header.magicNumber != IPC_MAGIC_NUMBER || layout->header.version != IPC_VERSION)
            {
                close();
                return false;
            }

            localReadPos = 0;
            fractionalPhase = 0.0;
            smoothedRatio = 1.0;
            return true;
#else
            return false;
#endif
        }

        void close()
        {
#if defined(_WIN32) || defined(_WIN64)
            if (mappedView != nullptr)
            {
                UnmapViewOfFile(mappedView);
                mappedView = nullptr;
                layout = nullptr;
            }
            if (memoryMapHandle != nullptr)
            {
                CloseHandle(memoryMapHandle);
                memoryMapHandle = nullptr;
            }
#elif defined(__APPLE__) || defined(__linux__)
            if (mappedView != nullptr)
            {
                munmap(mappedView, sizeof(SharedAudioMemoryLayout));
                mappedView = nullptr;
                layout = nullptr;
            }
            if (shmFd >= 0)
            {
                ::close(shmFd);
                shmFd = -1;
            }
#endif
            localReadPos = 0;
            fractionalPhase = 0.0;
            smoothedRatio = 1.0;
        }

        uint32_t getSampleRate() const noexcept
        {
            return layout ? layout->header.sampleRate : 48000;
        }

        // Professional Jitter-Free, Click-Free Fractional Resampling Sample Consumer
        void readAudio(float* const* channelData, int numChannels, int numSamples, double hostSampleRate = 48000.0) noexcept
        {
            if (layout == nullptr || numSamples <= 0 || channelData == nullptr)
            {
                if (channelData != nullptr)
                {
                    for (int ch = 0; ch < numChannels; ++ch)
                    {
                        if (channelData[ch] != nullptr)
                            std::memset(channelData[ch], 0, sizeof(float) * static_cast<size_t>(numSamples));
                    }
                }
                return;
            }

            const uint32_t dawRate = (layout->header.sampleRate > 1000) ? layout->header.sampleRate : 48000;
            const double hostRate = (hostSampleRate > 1000.0) ? hostSampleRate : 48000.0;
            const double baseRatio = static_cast<double>(dawRate) / hostRate;

            const uint64_t w = layout->header.writeCursor.load(std::memory_order_acquire);
            uint64_t r = localReadPos;

            // Target cushion: ~1024 DAW samples (~21-23ms buffer)
            const uint64_t targetCushion = 1024;
            const uint64_t maxBacklog = 3072; // ~60ms threshold

            // If fresh connection, buffer overrun, or severe lag:
            if (w > r + maxBacklog || r > w || r == 0)
            {
                r = (w >= targetCushion) ? (w - targetCushion) : 0;
                localReadPos = r;
                fractionalPhase = 0.0;
                smoothedRatio = baseRatio;
            }

            // Dynamic Phase-Locked Loop (PLL): smooth drift compensation
            const uint64_t currentBuffered = (w >= r) ? (w - r) : 0;
            double targetRatio = baseRatio;
            if (currentBuffered > targetCushion + 128)
            {
                targetRatio = baseRatio * 1.002; // gently consume excess
            }
            else if (currentBuffered + 128 < targetCushion)
            {
                targetRatio = baseRatio * 0.998; // gently slow down
            }
            smoothedRatio = (smoothedRatio * 0.95) + (targetRatio * 0.05);

            const int chans = std::min(numChannels, static_cast<int>(MAX_CHANNELS));

            for (int i = 0; i < numSamples; ++i)
            {
                const uint64_t totalPos = r + static_cast<uint64_t>(fractionalPhase);
                const float frac = static_cast<float>(fractionalPhase - std::floor(fractionalPhase));

                const uint32_t idx0 = static_cast<uint32_t>(totalPos & RING_BUFFER_MASK);
                const uint32_t idx1 = static_cast<uint32_t>((totalPos + 1) & RING_BUFFER_MASK);

                for (int ch = 0; ch < chans; ++ch)
                {
                    if (channelData[ch] != nullptr)
                    {
                        const float s0 = layout->channelData[ch][idx0];
                        const float s1 = layout->channelData[ch][idx1];
                        channelData[ch][i] = s0 + frac * (s1 - s0);
                    }
                }

                fractionalPhase += smoothedRatio;
                if (fractionalPhase >= 1.0)
                {
                    const uint64_t steps = static_cast<uint64_t>(fractionalPhase);
                    r += steps;
                    fractionalPhase -= static_cast<double>(steps);
                }
            }

            // Zero out any remaining output channels
            for (int ch = chans; ch < numChannels; ++ch)
            {
                if (channelData[ch] != nullptr)
                    std::memset(channelData[ch], 0, sizeof(float) * static_cast<size_t>(numSamples));
            }

            localReadPos = r;
            layout->header.readCursor.store(r, std::memory_order_release);
        }

    private:
#if defined(_WIN32) || defined(_WIN64)
        HANDLE memoryMapHandle{ nullptr };
        void* mappedView{ nullptr };
#elif defined(__APPLE__) || defined(__linux__)
        int shmFd{ -1 };
        void* mappedView{ nullptr };
#endif
        SharedAudioMemoryLayout* layout{ nullptr };
        uint64_t localReadPos{ 0 };
        double fractionalPhase{ 0.0 };
        double smoothedRatio{ 1.0 };
    };
}
