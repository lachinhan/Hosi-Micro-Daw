#include "SongbookManager.h"
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

        if (cleanTitle.contains(cleanQuery) || cleanArtist.contains(cleanQuery) || cleanComposer.contains(cleanQuery))
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
