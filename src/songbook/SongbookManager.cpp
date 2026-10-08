#include "SongbookManager.h"
#include "../audio/KeyDetector.h"
#include "../audio/VocalRangeDetector.h"
#include <BinaryData.h>

SongbookManager::SongbookManager()
{
    auto appDataDir = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
        .getChildFile("HosiStudio")
        .getChildFile("HosiMicroDAW");
    
    if (!appDataDir.exists())
        appDataDir.createDirectory();

    userStorageFile = appDataDir.getChildFile("songbook_user.json");

    loadDatabase();
}

void SongbookManager::loadDatabase()
{
    songs.clear();

    // 1. First load bundled / embedded database
    loadDefaultEmbeddedDatabase();

    // 2. Load and overlay user database (custom songs & custom tone settings)
    loadUserDatabase();

    if (onDatabaseChanged)
        onDatabaseChanged();
}

void SongbookManager::loadDefaultEmbeddedDatabase()
{
    juce::String jsonContent;

    // Try reading binary data first
    #if defined(JUCE_TARGET_HAS_BINARY_DATA)
    if (BinaryData::songbook_json != nullptr && BinaryData::songbook_jsonSize > 0)
    {
        jsonContent = juce::String::createStringFromData(BinaryData::songbook_json, BinaryData::songbook_jsonSize);
    }
    #endif

    // If not in binary data or empty, check assets/songbook.json directly
    if (jsonContent.isEmpty())
    {
        auto assetsFile = juce::File::getCurrentWorkingDirectory().getChildFile("assets").getChildFile("songbook.json");
        if (assetsFile.existsAsFile())
            jsonContent = assetsFile.loadFileAsString();
    }

    if (jsonContent.isNotEmpty())
    {
        auto parsed = juce::JSON::parse(jsonContent);
        if (parsed.isArray())
        {
            auto* arr = parsed.getArray();
            for (const auto& itemVar : *arr)
            {
                if (itemVar.isObject())
                {
                    songs.push_back(SongItem::fromVar(itemVar));
                }
            }
        }
    }
}

void SongbookManager::loadUserDatabase()
{
    if (!userStorageFile.existsAsFile())
        return;

    const juce::String jsonStr = userStorageFile.loadFileAsString();
    auto parsed = juce::JSON::parse(jsonStr);
    if (!parsed.isArray())
        return;

    auto* arr = parsed.getArray();
    for (const auto& itemVar : *arr)
    {
        if (itemVar.isObject())
        {
            SongItem userItem = SongItem::fromVar(itemVar);
            
            // Check if it's an override of an existing song
            bool found = false;
            for (auto& existing : songs)
            {
                if (existing.id == userItem.id || (existing.title.equalsIgnoreCase(userItem.title) && existing.artist.equalsIgnoreCase(userItem.artist)))
                {
                    existing.customKey = userItem.customKey;
                    existing.isFavorite = userItem.isFavorite;
                    if (userItem.keyMale.isNotEmpty()) existing.keyMale = userItem.keyMale;
                    if (userItem.keyFemale.isNotEmpty()) existing.keyFemale = userItem.keyFemale;
                    found = true;
                    break;
                }
            }

            if (!found)
            {
                userItem.isCustom = true;
                songs.push_back(userItem);
            }
        }
    }
}

bool SongbookManager::saveDatabase()
{
    // Save only user customizations, added songs, and favorites
    juce::Array<juce::var> userArr;

    for (const auto& s : songs)
    {
        if (s.isCustom || s.customKey.isNotEmpty() || s.isFavorite)
        {
            userArr.add(s.toVar());
        }
    }

    const juce::String jsonStr = juce::JSON::toString(juce::var(userArr), true);
    return userStorageFile.replaceWithText(jsonStr);
}

std::vector<SongItem> SongbookManager::searchSongs(const juce::String& query, const juce::String& genreFilter) const
{
    std::vector<SongItem> results;
    const juce::String cleanQuery = removeVietnameseAccents(query).trim().toLowerCase();
    const juce::String filterType = genreFilter.trim().toUpperCase();

    for (const auto& song : songs)
    {
        // 1. Check genre category filter
        if (filterType != "ALL" && filterType != "TAT CA" && filterType.isNotEmpty())
        {
            const juce::String songGenre = removeVietnameseAccents(song.genre).toLowerCase();
            const juce::String songComposer = removeVietnameseAccents(song.composer).toLowerCase();
            const juce::String songArtist = removeVietnameseAccents(song.artist).toLowerCase();
            const juce::String songId = song.id.toLowerCase();

            bool genreMatches = false;

            if (filterType == "FAVORITES" || filterType == "YEU_THICH")
            {
                if (song.isFavorite)
                    genreMatches = true;
            }
            else if (filterType == "NHAC_TRE" || filterType == "POP")
            {
                // Matches modern hits, pop, dance, rap, indie, R&B
                if (songId.startsWith("hit_") ||
                    songGenre.contains("pop") || songGenre.contains("dance") || songGenre.contains("house") ||
                    songGenre.contains("r&b") || songGenre.contains("edm") || songGenre.contains("indie") ||
                    songGenre.contains("disco") || songGenre.contains("rap") || songGenre.contains("hiphop") ||
                    songGenre.contains("nhac tre"))
                {
                    // Exclude Trinh songs
                    if (!songComposer.contains("trinh cong son") && !songId.startsWith("trinh_"))
                        genreMatches = true;
                }
            }
            else if (filterType == "BOLERO" || filterType == "NHAC_VANG")
            {
                if (songId.startsWith("bolero_") ||
                    songGenre.contains("bolero") || songGenre.contains("vang") || songGenre.contains("rumba") ||
                    songGenre.contains("chachacha") || songGenre.contains("que huong") ||
                    songArtist.contains("quang le") || songArtist.contains("nhu quynh") ||
                    songArtist.contains("dan nguyen") || songArtist.contains("che linh") ||
                    songArtist.contains("truong vu") || songArtist.contains("giao linh") ||
                    songArtist.contains("thanh tuyen") || songArtist.contains("huong lan"))
                {
                    genreMatches = true;
                }
            }
            else if (filterType == "TRINH")
            {
                if (songId.startsWith("trinh_") || songComposer.contains("trinh cong son") || songGenre.contains("trinh"))
                {
                    genreMatches = true;
                }
            }
            else if (filterType == "TRU_TINH" || filterType == "BALLAD")
            {
                if (songId.startsWith("xanh_") || songGenre.contains("ballad") || songGenre.contains("tru tinh") ||
                    songGenre.contains("slow") || songGenre.contains("boston") ||
                    songComposer.contains("lam phuong") || songComposer.contains("truc phuong") ||
                    songComposer.contains("tuong van") || songComposer.contains("mr. siro") ||
                    songArtist.contains("le quyen") || songArtist.contains("bang kieu") ||
                    songArtist.contains("huong tram") || songArtist.contains("ho ngoc ha") ||
                    songArtist.contains("my tam"))
                {
                    genreMatches = true;
                }
            }
            else if (filterType == "CUSTOM_USER")
            {
                if (song.isCustom || song.customKey.isNotEmpty())
                {
                    genreMatches = true;
                }
            }
            else if (filterType == "HOT_TREND" || filterType == "CLOUD" || filterType == "TIKTOK")
            {
                if (songId.startsWith("trend_") || songId.startsWith("cloud_") ||
                    songGenre.contains("trend") || songGenre.contains("tiktok") || songGenre.contains("hot"))
                {
                    genreMatches = true;
                }
            }
            else
            {
                // Generic fallback contains
                const juce::String rawFilter = removeVietnameseAccents(genreFilter).toLowerCase();
                if (songGenre.contains(rawFilter) || songComposer.contains(rawFilter) || songArtist.contains(rawFilter))
                    genreMatches = true;
            }

            if (!genreMatches)
                continue;
        }

        // 2. Check query match
        if (cleanQuery.isEmpty())
        {
            results.push_back(song);
            continue;
        }

        const juce::String cleanTitle = removeVietnameseAccents(song.title).toLowerCase();
        const juce::String cleanArtist = removeVietnameseAccents(song.artist).toLowerCase();
        const juce::String cleanComposer = removeVietnameseAccents(song.composer).toLowerCase();

        if (cleanTitle.contains(cleanQuery) || cleanArtist.contains(cleanQuery) || cleanComposer.contains(cleanQuery)
            || (cleanQuery.length() >= 4 && cleanTitle.length() >= 4 && cleanQuery.contains(cleanTitle)))
        {
            results.push_back(song);
        }
    }

    return results;
}

bool SongbookManager::addSong(const SongItem& item)
{
    SongItem newItem = item;
    if (newItem.id.isEmpty())
        newItem.id = "user_" + juce::String(juce::Time::currentTimeMillis());
    newItem.isCustom = true;

    songs.push_back(newItem);
    saveDatabase();

    if (onDatabaseChanged)
        onDatabaseChanged();

    return true;
}

bool SongbookManager::updateSong(const SongItem& item)
{
    for (auto& s : songs)
    {
        if (s.id == item.id)
        {
            s = item;
            saveDatabase();
            if (onDatabaseChanged)
                onDatabaseChanged();
            return true;
        }
    }
    return false;
}

bool SongbookManager::deleteSong(const juce::String& songId)
{
    for (auto it = songs.begin(); it != songs.end(); ++it)
    {
        if (it->id == songId)
        {
            songs.erase(it);
            saveDatabase();
            if (onDatabaseChanged)
                onDatabaseChanged();
            return true;
        }
    }
    return false;
}

bool SongbookManager::setCustomTone(const juce::String& songId, const juce::String& customTone)
{
    for (auto& s : songs)
    {
        if (s.id == songId)
        {
            s.customKey = customTone;
            saveDatabase();
            if (onDatabaseChanged)
                onDatabaseChanged();
            return true;
        }
    }
    return false;
}

bool SongbookManager::toggleFavorite(const juce::String& songId)
{
    for (auto& s : songs)
    {
        if (s.id == songId)
        {
            s.isFavorite = !s.isFavorite;
            saveDatabase();
            if (onDatabaseChanged)
                onDatabaseChanged();
            return s.isFavorite;
        }
    }
    return false;
}

bool SongbookManager::setFavorite(const juce::String& songId, bool isFav)
{
    for (auto& s : songs)
    {
        if (s.id == songId)
        {
            s.isFavorite = isFav;
            saveDatabase();
            if (onDatabaseChanged)
                onDatabaseChanged();
            return true;
        }
    }
    return false;
}

bool SongbookManager::exportToJson(const juce::File& destinationFile) const
{
    juce::Array<juce::var> arr;
    for (const auto& s : songs)
        arr.add(s.toVar());

    const juce::String jsonStr = juce::JSON::toString(juce::var(arr), true);
    return destinationFile.replaceWithText(jsonStr);
}

bool SongbookManager::importFromJson(const juce::File& sourceFile)
{
    if (!sourceFile.existsAsFile())
        return false;

    const juce::String jsonStr = sourceFile.loadFileAsString();
    auto parsed = juce::JSON::parse(jsonStr);
    if (!parsed.isArray())
        return false;

    auto* arr = parsed.getArray();
    int addedCount = 0;
    for (const auto& itemVar : *arr)
    {
        if (itemVar.isObject())
        {
            SongItem imported = SongItem::fromVar(itemVar);
            bool exists = false;
            for (const auto& s : songs)
            {
                if (s.title.equalsIgnoreCase(imported.title) && s.artist.equalsIgnoreCase(imported.artist))
                {
                    exists = true;
                    break;
                }
            }
            if (!exists)
            {
                imported.isCustom = true;
                songs.push_back(imported);
                addedCount++;
            }
        }
    }

    if (addedCount > 0)
    {
        saveDatabase();
        if (onDatabaseChanged)
            onDatabaseChanged();
    }

    return true;
}

juce::String SongbookManager::removeVietnameseAccents(const juce::String& input)
{
    juce::String result = input;
    
    // Convert Vietnamese UTF-8 accented characters to standard ASCII
    struct AccentMap {
        const char* pattern;
        char replacement;
    };

    static const AccentMap maps[] = {
        {"á", 'a'}, {"à", 'a'}, {"ả", 'a'}, {"ã", 'a'}, {"ạ", 'a'},
        {"ă", 'a'}, {"ắ", 'a'}, {"ằ", 'a'}, {"ẳ", 'a'}, {"ẵ", 'a'}, {"ặ", 'a'},
        {"â", 'a'}, {"ấ", 'a'}, {"ầ", 'a'}, {"ẩ", 'a'}, {"ẫ", 'a'}, {"ậ", 'a'},
        {"Á", 'A'}, {"À", 'A'}, {"Ả", 'A'}, {"Ã", 'A'}, {"Ạ", 'A'},
        {"Ă", 'A'}, {"Ắ", 'A'}, {"Ằ", 'A'}, {"Ẳ", 'A'}, {"Ẵ", 'A'}, {"Ặ", 'A'},
        {"Â", 'A'}, {"Ấ", 'A'}, {"Ầ", 'A'}, {"Ẩ", 'A'}, {"Ẫ", 'A'}, {"Ậ", 'A'},

        {"đ", 'd'}, {"Đ", 'D'},

        {"é", 'e'}, {"è", 'e'}, {"ẻ", 'e'}, {"ẽ", 'e'}, {"ẹ", 'e'},
        {"ê", 'e'}, {"ế", 'e'}, {"ề", 'e'}, {"ể", 'e'}, {"ễ", 'e'}, {"ệ", 'e'},
        {"É", 'E'}, {"È", 'E'}, {"Ẻ", 'E'}, {"Ẽ", 'E'}, {"Ẹ", 'E'},
        {"Ê", 'E'}, {"Ế", 'E'}, {"Ề", 'E'}, {"Ể", 'E'}, {"Ễ", 'E'}, {"Ệ", 'E'},

        {"í", 'i'}, {"ì", 'i'}, {"ỉ", 'i'}, {"ĩ", 'i'}, {"ị", 'i'},
        {"Í", 'I'}, {"Ì", 'I'}, {"Ỉ", 'I'}, {"Ĩ", 'I'}, {"Ị", 'I'},

        {"ó", 'o'}, {"ò", 'o'}, {"ỏ", 'o'}, {"õ", 'o'}, {"ọ", 'o'},
        {"ô", 'o'}, {"ố", 'o'}, {"ồ", 'o'}, {"ổ", 'o'}, {"ỗ", 'o'}, {"ộ", 'o'},
        {"ơ", 'o'}, {"ớ", 'o'}, {"ờ", 'o'}, {"ở", 'o'}, {"ỡ", 'o'}, {"ợ", 'o'},
        {"Ó", 'O'}, {"Ò", 'O'}, {"Ỏ", 'O'}, {"Õ", 'O'}, {"Ọ", 'O'},
        {"Ô", 'O'}, {"Ố", 'O'}, {"Ồ", 'O'}, {"Ổ", 'O'}, {"Ỗ", 'O'}, {"Ộ", 'O'},
        {"Ơ", 'O'}, {"Ớ", 'O'}, {"Ờ", 'O'}, {"Ở", 'O'}, {"Ỡ", 'O'}, {"Ợ", 'O'},

        {"ú", 'u'}, {"ù", 'u'}, {"ủ", 'u'}, {"ũ", 'u'}, {"ụ", 'u'},
        {"ư", 'u'}, {"ứ", 'u'}, {"ừ", 'u'}, {"ử", 'u'}, {"ữ", 'u'}, {"ự", 'u'},
        {"Ú", 'U'}, {"Ù", 'U'}, {"Ủ", 'U'}, {"Ũ", 'U'}, {"Ụ", 'U'},
        {"Ư", 'U'}, {"Ứ", 'U'}, {"Ừ", 'U'}, {"Ử", 'U'}, {"Ữ", 'U'}, {"Ự", 'U'},

        {"ý", 'y'}, {"ỳ", 'y'}, {"ỷ", 'y'}, {"ỹ", 'y'}, {"ỵ", 'y'},
        {"Ý", 'Y'}, {"Ỳ", 'Y'}, {"Ỷ", 'Y'}, {"Ỹ", 'Y'}, {"Ỵ", 'Y'}
    };

    for (const auto& m : maps)
    {
        result = result.replace(juce::String::fromUTF8(m.pattern), juce::String::charToString(m.replacement));
    }

    return result;
}

void SongbookManager::parseKeyAndScale(const juce::String& toneStr, int& outRootNote, bool& outIsMinor)
{
    outRootNote = 0; // C
    outIsMinor = false;

    juce::String clean = toneStr.trim();
    if (clean.isEmpty())
        return;

    // Check minor
    if (clean.endsWithIgnoreCase("m") && !clean.endsWithIgnoreCase("maj"))
    {
        outIsMinor = true;
        clean = clean.substring(0, clean.length() - 1);
    }

    // Root note mapping (0 = C, 1 = C#, 2 = D, 3 = D#, 4 = E, 5 = F, 6 = F#, 7 = G, 8 = G#, 9 = A, 10 = Bb, 11 = B)
    clean = clean.toUpperCase();
    if (clean == "C") outRootNote = 0;
    else if (clean == "C#" || clean == "DB") outRootNote = 1;
    else if (clean == "D") outRootNote = 2;
    else if (clean == "D#" || clean == "EB") outRootNote = 3;
    else if (clean == "E") outRootNote = 4;
    else if (clean == "F") outRootNote = 5;
    else if (clean == "F#" || clean == "GB") outRootNote = 6;
    else if (clean == "G") outRootNote = 7;
    else if (clean == "G#" || clean == "AB") outRootNote = 8;
    else if (clean == "A") outRootNote = 9;
    else if (clean == "A#" || clean == "BB") outRootNote = 10;
    else if (clean == "B") outRootNote = 11;
}

juce::String SongbookManager::transposeKey(const juce::String& toneStr, int semitoneShift)
{
    int root = 0;
    bool isMinor = false;
    parseKeyAndScale(toneStr, root, isMinor);
    int newRoot = (root + semitoneShift) % 12;
    if (newRoot < 0) newRoot += 12;
    juce::String note = KeyDetector::getNoteName(newRoot);
    return isMinor ? (note + "m") : note;
}

SongbookManager::SongFitResult SongbookManager::evaluateSongFit(const SongItem& item, int userLowestMidi, int userHighestMidi) const
{
    SongFitResult result;
    
    // Safety clamp user ranges
    int uLow = std::clamp(userLowestMidi, 36, 84);
    int uHigh = std::clamp(userHighestMidi, uLow + 4, 96);
    int uMid = (uLow + uHigh) / 2;

    // Determine target primary key based on user vocal profile
    bool isUserMale = (uMid < 64);
    juce::String chosenKey = isUserMale ? (item.keyMale.isNotEmpty() ? item.keyMale : item.keyOriginal)
                                        : (item.keyFemale.isNotEmpty() ? item.keyFemale : item.keyOriginal);

    if (chosenKey.isEmpty())
        chosenKey = item.getEffectiveTone();

    result.baseKey = chosenKey;

    int root = 0;
    bool isMinor = false;
    parseKeyAndScale(chosenKey, root, isMinor);

    // Calculate estimated song range
    int baseMidiRoot = 0;
    if (isUserMale)
    {
        // Male root in MIDI 43-55 (G2 to G3)
        baseMidiRoot = (root >= 7) ? (36 + root) : (48 + root);
    }
    else
    {
        // Female root in MIDI 48-60 (C3 to C4)
        baseMidiRoot = (root >= 7) ? (41 + root) : (53 + root);
    }

    int songLow = isMinor ? (baseMidiRoot - 2) : (baseMidiRoot - 4);
    int songHigh = isMinor ? (baseMidiRoot + 22) : (baseMidiRoot + 19);

    result.songLowestMidi = songLow;
    result.songHighestMidi = songHigh;

    int deltaHigh = songHigh - uHigh;
    int deltaLow = uLow - songLow;

    int shift = 0;
    if (deltaHigh > 0)
    {
        // Song is too high for user -> suggest lowering
        shift = -std::min(4, deltaHigh);
    }
    else if (deltaLow > 2 && deltaHigh < -3)
    {
        // Song is too low and has plenty of headroom at the top -> suggest raising
        shift = std::min(3, (deltaLow + 1) / 2);
    }

    result.recommendedShift = shift;
    result.recommendedTone = (shift == 0) ? chosenKey : transposeKey(chosenKey, shift);

    if (shift == 0)
    {
        if (songHigh <= uHigh && songLow >= uLow)
        {
            result.fitScore = 100;
            result.fitBadge = juce::String::fromUTF8(u8"💯 100% Vừa Vặn");
            result.advice = juce::String::fromUTF8(u8"Cực kỳ vừa vặn! Nốt cao nhất (") + VocalRangeDetector::midiToNoteName(songHigh) + juce::String::fromUTF8(u8") nằm hoàn hảo trong tầm giọng của bạn.");
        }
        else
        {
            result.fitScore = 95;
            result.fitBadge = juce::String::fromUTF8(u8"⭐ 95% Rất Hợp");
            result.advice = juce::String::fromUTF8(u8"Quãng giọng rất đẹp và phù hợp hoàn hảo với bài hát này.");
        }
    }
    else if (shift < 0)
    {
        result.fitScore = std::max(65, 100 - std::abs(shift) * 8);
        result.fitBadge = juce::String::fromUTF8(u8"🎯 Hạ ") + juce::String(shift) + juce::String::fromUTF8(u8" Tone");
        result.advice = juce::String::fromUTF8(u8"Nốt cao nhất đạt ") + VocalRangeDetector::midiToNoteName(songHigh) + juce::String::fromUTF8(u8". Hãy hạ ") + juce::String(shift) + juce::String::fromUTF8(u8" semitones để hát tròn vành rõ chữ không bị với!");
    }
    else
    {
        result.fitScore = std::max(70, 100 - shift * 7);
        result.fitBadge = juce::String::fromUTF8(u8"⚡ Tăng +") + juce::String(shift) + juce::String::fromUTF8(u8" Tone");
        result.advice = juce::String::fromUTF8(u8"Tăng +") + juce::String(shift) + juce::String::fromUTF8(u8" semitones giúp giọng sáng bay bổng hơn và không bị quá trầm.");
    }

    return result;
}

std::vector<std::pair<SongItem, SongbookManager::SongFitResult>> SongbookManager::getAiRecommendedSongs(
    int userLowestMidi,
    int userHighestMidi,
    const juce::String& query,
    const juce::String& genreFilter
) const
{
    // First retrieve filtered songs matching search query
    auto candidateSongs = searchSongs(query, genreFilter);

    std::vector<std::pair<SongItem, SongFitResult>> rankedList;
    rankedList.reserve(candidateSongs.size());

    for (const auto& song : candidateSongs)
    {
        auto fit = evaluateSongFit(song, userLowestMidi, userHighestMidi);
        rankedList.push_back({ song, fit });
    }

    // Sort by fit score descending, then by favorite, then alphabetically
    std::sort(rankedList.begin(), rankedList.end(), [](const auto& a, const auto& b) {
        if (a.second.fitScore != b.second.fitScore)
            return a.second.fitScore > b.second.fitScore;
        if (a.first.isFavorite != b.first.isFavorite)
            return a.first.isFavorite > b.first.isFavorite;
        return a.first.title < b.first.title;
    });

    return rankedList;
}

void SongbookManager::syncFromCloudAsync(std::function<void(bool success, int newSongsAdded, const juce::String& statusMsg)> callback)
{
    if (isCloudSyncing.exchange(true))
    {
        if (callback)
            callback(false, 0, juce::String::fromUTF8(u8"Đang trong quá trình đồng bộ..."));
        return;
    }

    // Launch background thread
    juce::Thread::launch([this, callback]() {
        juce::String jsonText;

        const std::vector<juce::String> cloudUrls = {
            "https://raw.githubusercontent.com/lachinhan/Hosi-Micro-Daw/main/cloud/songbook_cloud.json",
            "https://raw.githubusercontent.com/lachinhan/Hosi-Micro-Daw-Private/main/cloud/songbook_cloud.json",
            "https://api.lachinhan.xyz/microdaw/v3/songbook_cloud.json"
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
            auto localCloud = juce::File::getCurrentWorkingDirectory().getChildFile("cloud").getChildFile("songbook_cloud.json");
            if (localCloud.existsAsFile())
                jsonText = localCloud.loadFileAsString();
        }

        if (jsonText.isNotEmpty())
        {
            auto parsed = juce::JSON::parse(jsonText);
            if (parsed.isArray())
            {
                auto* arr = parsed.getArray();
                std::vector<SongItem> cloudSongs;
                for (const auto& itemVar : *arr)
                {
                    if (itemVar.isObject())
                        cloudSongs.push_back(SongItem::fromVar(itemVar));
                }

                // Switch to message thread to safely update database
                juce::MessageManager::callAsync([this, cloudSongs, callback]() {
                    int addedCount = 0;
                    for (const auto& cSong : cloudSongs)
                    {
                        bool exists = false;
                        for (auto& existing : songs)
                        {
                            if (existing.id.equalsIgnoreCase(cSong.id) ||
                                (existing.title.equalsIgnoreCase(cSong.title) && existing.artist.equalsIgnoreCase(cSong.artist)))
                            {
                                exists = true;
                                if (existing.keyMale.isEmpty() && cSong.keyMale.isNotEmpty()) existing.keyMale = cSong.keyMale;
                                if (existing.keyFemale.isEmpty() && cSong.keyFemale.isNotEmpty()) existing.keyFemale = cSong.keyFemale;
                                break;
                            }
                        }

                        if (!exists)
                        {
                            songs.push_back(cSong);
                            addedCount++;
                        }
                    }

                    if (addedCount > 0)
                    {
                        saveDatabase();
                    }

                    isCloudSyncing.store(false);

                    if (onDatabaseChanged)
                        onDatabaseChanged();

                    juce::String statusMessage = juce::String::fromUTF8(u8"✓ Đồng bộ thành công! ") + 
                        (addedCount > 0 ? (juce::String::fromUTF8(u8"Đã nạp thêm ") + juce::String(addedCount) + juce::String::fromUTF8(u8" bài hát Hot Trend mới."))
                                        : juce::String::fromUTF8(u8"Kho bài hát của bạn đã là mới nhất."));

                    if (callback)
                        callback(true, addedCount, statusMessage);
                });
                return;
            }
        }

        juce::MessageManager::callAsync([this, callback]() {
            isCloudSyncing.store(false);
            if (callback)
                callback(false, 0, juce::String::fromUTF8(u8"Không thể kết nối đến máy chủ Cloud. Vui lòng kiểm tra Internet."));
        });
    });
}

