#pragma once

#include <juce_core/juce_core.h>
#include <juce_data_structures/juce_data_structures.h>
#include <vector>
#include <functional>

struct SongItem
{
    juce::String id;
    juce::String title;
    juce::String artist;
    juce::String composer;
    juce::String keyOriginal{ "Am" };
    juce::String keyMale{ "Am" };
    juce::String keyFemale{ "Dm" };
    juce::String scale{ "Minor" }; // "Minor" or "Major"
    juce::String customKey;        // User custom saved key
    juce::String genre{ "Ballad" };
    int tempo{ 80 };
    bool isCustom{ false };
    bool isFavorite{ false };

    juce::String getEffectiveTone() const
    {
        if (customKey.isNotEmpty())
            return customKey;
        return keyOriginal;
    }

    juce::var toVar() const
    {
        auto* obj = new juce::DynamicObject();
        obj->setProperty("id", id);
        obj->setProperty("title", title);
        obj->setProperty("artist", artist);
        obj->setProperty("composer", composer);
        obj->setProperty("key_original", keyOriginal);
        obj->setProperty("key_male", keyMale);
        obj->setProperty("key_female", keyFemale);
        obj->setProperty("scale", scale);
        obj->setProperty("custom_key", customKey);
        obj->setProperty("genre", genre);
        obj->setProperty("tempo", tempo);
        obj->setProperty("is_custom", isCustom);
        obj->setProperty("is_favorite", isFavorite);
        return juce::var(obj);
    }

    static juce::String cleanHtmlEntities(const juce::String& text)
    {
        juce::String res = text;
        res = res.replace("&#039;", "'")
                 .replace("&#39;", "'")
                 .replace("&apos;", "'")
                 .replace("&quot;", "\"")
                 .replace("&amp;", "&")
                 .replace("&lt;", "<")
                 .replace("&gt;", ">")
                 .replace("&nbsp;", " ")
                 .replace("&#8217;", "'")
                 .replace("&#8220;", "\"")
                 .replace("&#8221;", "\"")
                 .replace("&#8211;", "-")
                 .replace("&#8212;", "-");
        return res.trim();
    }

    static SongItem fromVar(const juce::var& v)
    {
        SongItem item;
        item.id = v["id"].toString();
        item.title = cleanHtmlEntities(v["title"].toString());
        item.artist = cleanHtmlEntities(v["artist"].toString());
        item.composer = cleanHtmlEntities(v["composer"].toString());
        item.keyOriginal = v["key_original"].toString().trim();
        item.keyMale = v["key_male"].toString().trim();
        item.keyFemale = v["key_female"].toString().trim();
        item.scale = v["scale"].toString().trim();
        item.customKey = v["custom_key"].toString().trim();
        item.genre = cleanHtmlEntities(v["genre"].toString());
        item.tempo = static_cast<int>(v["tempo"]);
        item.isCustom = static_cast<bool>(v["is_custom"]);
        item.isFavorite = static_cast<bool>(v["is_favorite"]);

        if (item.keyOriginal.isEmpty()) item.keyOriginal = "Am";
        if (item.keyMale.isEmpty()) item.keyMale = item.keyOriginal;
        if (item.keyFemale.isEmpty()) item.keyFemale = "Dm";
        if (item.scale.isEmpty()) item.scale = item.keyOriginal.endsWithIgnoreCase("m") ? "Minor" : "Major";

        return item;
    }
};

class SongbookManager
{
public:
    SongbookManager();
    ~SongbookManager() = default;

    void loadDatabase();
    bool saveDatabase();

    const std::vector<SongItem>& getAllSongs() const { return songs; }
    
    // Filter and search (supports Vietnamese unaccented search)
    std::vector<SongItem> searchSongs(const juce::String& query, const juce::String& genreFilter = "ALL") const;

    bool addSong(const SongItem& item);
    bool updateSong(const SongItem& item);
    bool deleteSong(const juce::String& songId);
    bool setCustomTone(const juce::String& songId, const juce::String& customTone);
    bool toggleFavorite(const juce::String& songId);
    bool setFavorite(const juce::String& songId, bool isFav);

    bool exportToJson(const juce::File& destinationFile) const;
    bool importFromJson(const juce::File& sourceFile);

    static juce::String removeVietnameseAccents(const juce::String& input);
    static void parseKeyAndScale(const juce::String& toneStr, int& outRootNote, bool& outIsMinor);

    std::function<void()> onDatabaseChanged;

private:
    std::vector<SongItem> songs;
    juce::File userStorageFile;

    void loadDefaultEmbeddedDatabase();
    void loadUserDatabase();
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SongbookManager)
};
