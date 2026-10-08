#pragma once

#include <juce_core/juce_core.h>
#include <juce_data_structures/juce_data_structures.h>
#include "../audio/GraphManager.h"
#include <vector>
#include <functional>

struct ArtistPresetItem
{
    juce::String id;
    juce::String name;
    juce::String artist;
    juce::String category;
    juce::String description;
    juce::var dspSettings;

    static ArtistPresetItem fromVar(const juce::var& v)
    {
        ArtistPresetItem item;
        item.id = v["id"].toString();
        item.name = v["name"].toString();
        item.artist = v["artist"].toString();
        item.category = v["category"].toString();
        item.description = v["description"].toString();
        item.dspSettings = v["dsp"];
        return item;
    }
};

class PresetManager
{
public:
    enum class QuickPresetType
    {
        LiveSinging,   // Full vocal chain + Auto-Tune + Reverb Aux
        TalkStream,    // Gate + EQ + Compression only, Reverb Bypassed, Auto-Tune Bypassed
        HighEnergyTrap // Fast Retune Auto-Tune + Heavy Compression + Delay
    };

    explicit PresetManager(GraphManager& graphManager);
    ~PresetManager() = default;

    void applyQuickPreset(QuickPresetType type);
    void toggleTalkMode();

    bool isTalkModeActive() const noexcept { return isTalkMode; }
    QuickPresetType getCurrentPreset() const noexcept { return currentPreset; }

    bool savePresetToFile(const juce::File& file);
    bool loadPresetFromFile(const juce::File& file);

    void saveSessionState();
    void restoreSessionState();
    juce::File getSessionFile() const;
    juce::File getPresetsDirectory() const;

    int getSavedWindowWidth() const noexcept { return savedWindowWidth; }
    int getSavedWindowHeight() const noexcept { return savedWindowHeight; }
    void setWindowSize(int width, int height) noexcept { savedWindowWidth = width; savedWindowHeight = height; }

    float getSavedUiScale() const noexcept { return savedUiScale; }
    void setUiScale(float scale) noexcept { savedUiScale = scale; }

    juce::ValueTree exportStateAsValueTree() const;
    void restoreStateFromValueTree(const juce::ValueTree& tree);

    // ☁️ Artist Preset Cloud
    void fetchArtistPresetsCloudAsync(std::function<void(bool success, const std::vector<ArtistPresetItem>& presets, const juce::String& msg)> callback = nullptr);
    bool applyArtistPreset(const ArtistPresetItem& preset);
    const std::vector<ArtistPresetItem>& getCachedArtistPresets() const noexcept { return cachedArtistPresets; }
    bool isFetchingCloudPresets() const noexcept { return isFetchingPresets.load(); }

private:
    GraphManager& graphManager;
    QuickPresetType currentPreset{ QuickPresetType::LiveSinging };
    bool isTalkMode{ false };
    std::vector<bool> preTalkBypassStates;

    std::vector<ArtistPresetItem> cachedArtistPresets;
    std::atomic<bool> isFetchingPresets{ false };

    int savedWindowWidth{ 1140 };
    int savedWindowHeight{ 760 };
    float savedUiScale{ 1.0f };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PresetManager)
};

