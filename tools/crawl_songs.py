"""
Hosi Micro-DAW - Songbook & Tone Crawler + Dataset Generator
Crawl top songs from HopAmViet & generate comprehensive songbook JSON for LiveStream Micro-DAW.
"""

import urllib.request
import re
import html
import json
import time
import os
import sys
from concurrent.futures import ThreadPoolExecutor

sys.stdout.reconfigure(encoding='utf-8')

HEADERS = {
    'User-Agent': 'Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/120.0.0.0 Safari/537.36'
}

def fetch_html(url, timeout=4):
    try:
        req = urllib.request.Request(url, headers=HEADERS)
        with urllib.request.urlopen(req, timeout=timeout) as resp:
            return resp.read().decode('utf-8', errors='ignore')
    except Exception as e:
        return ""

def parse_chord_name(chord_str):
    if not chord_str:
        return "C", "Major", "C"
    
    chord_str = chord_str.strip().replace("[", "").replace("]", "")
    m = re.match(r'^([A-G][b#]?)(m|maj|min|dim|aug|sus|7|m7|maj7)?', chord_str, re.IGNORECASE)
    if not m:
        return "C", "Major", "C"
    
    root = m.group(1).upper()
    if len(root) == 2:
        root = root[0] + root[1].lower()
    
    suffix = (m.group(2) or "").lower()
    if suffix.startswith('m') and not suffix.startswith('maj'):
        scale = "Minor"
        formatted = root + "m"
    else:
        scale = "Major"
        formatted = root
        
    return root, scale, formatted

def calculate_female_tone(key, scale):
    note_order_minor = ["Am", "A#m", "Bm", "Cm", "C#m", "Dm", "D#m", "Em", "Fm", "F#m", "Gm", "G#m"]
    note_order_major = ["C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"]
    
    formatted = key + ("m" if scale == "Minor" else "")
    if scale == "Minor":
        if formatted in note_order_minor:
            idx = note_order_minor.index(formatted)
            female_idx = (idx + 5) % 12
            male_idx = idx
            return formatted, note_order_minor[female_idx], note_order_minor[male_idx]
    else:
        if formatted in note_order_major:
            idx = note_order_major.index(formatted)
            female_idx = (idx + 5) % 12
            male_idx = idx
            return formatted, note_order_major[female_idx], note_order_major[male_idx]
            
    return formatted, formatted, formatted

def crawl_song_detail(song_url, default_genre):
    content = fetch_html(song_url)
    if not content:
        return None
        
    title_m = re.search(r'<title>(.*?)</title>', content)
    title_raw = title_m.group(1) if title_m else ""
    title = ""
    composer = "Khuyết Danh"
    
    if "Hợp âm" in title_raw:
        clean = title_raw.replace("Hợp âm", "").replace("- Hợp Âm Việt", "").strip()
        parts = clean.split("-")
        if len(parts) >= 2:
            title = parts[0].strip()
            composer = parts[1].strip()
        elif len(parts) == 1:
            title = parts[0].strip()
    else:
        title = title_raw.strip()
        
    if not title or len(title) < 2:
        return None
        
    rhythm_m = re.search(r'Điệu:\s*([A-Za-z0-9\s\-]+)', content)
    genre = rhythm_m.group(1).strip() if rhythm_m else default_genre
    if not genre or len(genre) > 30:
        genre = default_genre
        
    chords = re.findall(r'\[([A-G][b#]?[m]?(?:maj7|7|sus4|m7|dim)?)\]', content)
    primary_chord = chords[0] if chords else "Am"
    
    root, scale, formatted_key = parse_chord_name(primary_chord)
    orig_key, female_key, male_key = calculate_female_tone(root, scale)
    
    artists = re.findall(r'href="https://hopamviet\.vn/chord/singer/[^"]+"[^>]*>(.*?)</a>', content)
    artist_names = [html.unescape(a.strip()) for a in artists if a.strip()]
    artist_str = ", ".join(artist_names[:3]) if artist_names else "Nhiều ca sĩ"
    
    slug_m = re.search(r'/song/([^/]+)/', song_url)
    song_id = slug_m.group(1) if slug_m else f"song_{len(title)}"
    
    return {
        "id": song_id,
        "title": title,
        "artist": artist_str,
        "composer": composer,
        "key_original": orig_key,
        "key_male": male_key,
        "key_female": female_key,
        "scale": scale,
        "genre": genre,
        "tempo": 80 if scale == "Minor" else 95,
        "source": "hopamviet"
    }

# Master Curated Song Library
CURATED_VIETNAMESE_HITS = [
    # Top Nhạc Trẻ & Pop Ballad Hiện Đại
    {"id": "hit_001", "title": "Hoa Nở Không Màu", "artist": "Hoài Lâm", "composer": "Nguyễn Minh Cường", "key_original": "Am", "key_male": "Am", "key_female": "Dm", "scale": "Minor", "genre": "Ballad", "tempo": 78},
    {"id": "hit_002", "title": "Ai Chung Tình Được Mãi", "artist": "Đinh Tùng Huy, Hoài Lâm", "composer": "Đông Thiên Đức", "key_original": "Em", "key_male": "Em", "key_female": "Am", "scale": "Minor", "genre": "Pop Ballad", "tempo": 82},
    {"id": "hit_003", "title": "Ngày Mai Người Ta Lấy Chồng", "artist": "Thành Đạt, Voi Bản Đôn", "composer": "Đông Thiên Đức", "key_original": "Am", "key_male": "Am", "key_female": "Dm", "scale": "Minor", "genre": "Ballad", "tempo": 76},
    {"id": "hit_004", "title": "Khóa Ly Biệt", "artist": "Voi Bản Đôn (Anh Tú)", "composer": "Đông Thiên Đức", "key_original": "Bm", "key_male": "Bm", "key_female": "Em", "scale": "Minor", "genre": "Pop Ballad", "tempo": 80},
    {"id": "hit_005", "title": "Cắt Đôi Nỗi Sầu", "artist": "Tăng Duy Tân", "composer": "Tăng Duy Tân", "key_original": "F#m", "key_male": "F#m", "key_female": "Bm", "scale": "Minor", "genre": "Dance / House", "tempo": 128},
    {"id": "hit_006", "title": "Bên Trên Tầng Lầu", "artist": "Tăng Duy Tân", "composer": "Tăng Duy Tân", "key_original": "Dm", "key_male": "Dm", "key_female": "Gm", "scale": "Minor", "genre": "Dance Pop", "tempo": 124},
    {"id": "hit_007", "title": "Nơi Này Có Anh", "artist": "Sơn Tùng M-TP", "composer": "Sơn Tùng M-TP", "key_original": "F", "key_male": "F", "key_female": "Bb", "scale": "Major", "genre": "Pop R&B", "tempo": 105},
    {"id": "hit_008", "title": "Muộn Rồi Mà Sao Còn", "artist": "Sơn Tùng M-TP", "composer": "Sơn Tùng M-TP", "key_original": "Ab", "key_male": "Ab", "key_female": "Db", "scale": "Major", "genre": "Pop R&B", "tempo": 110},
    {"id": "hit_009", "title": "Lạc Trôi", "artist": "Sơn Tùng M-TP", "composer": "Sơn Tùng M-TP", "key_original": "Dm", "key_male": "Dm", "key_female": "Gm", "scale": "Minor", "genre": "Future Bass", "tempo": 120},
    {"id": "hit_010", "title": "Chúng Ta Của Hiện Tại", "artist": "Sơn Tùng M-TP", "composer": "Sơn Tùng M-TP", "key_original": "Eb", "key_male": "Eb", "key_female": "Ab", "scale": "Major", "genre": "Pop / 80s", "tempo": 90},
    {"id": "hit_011", "title": "Đừng Làm Trái Tim Anh Đau", "artist": "Sơn Tùng M-TP", "composer": "Sơn Tùng M-TP", "key_original": "C", "key_male": "C", "key_female": "F", "scale": "Major", "genre": "Pop Dance", "tempo": 115},
    {"id": "hit_012", "title": "Sóng Gió", "artist": "Jack, K-ICM", "composer": "Jack", "key_original": "Bbm", "key_male": "Bbm", "key_female": "Ebm", "scale": "Minor", "genre": "Pop Ballad", "tempo": 84},
    {"id": "hit_013", "title": "Hồng Nhan", "artist": "Jack", "composer": "Jack", "key_original": "Dm", "key_male": "Dm", "key_female": "Gm", "scale": "Minor", "genre": "Pop R&B", "tempo": 90},
    {"id": "hit_014", "title": "Bạc Phận", "artist": "Jack, K-ICM", "composer": "Jack", "key_original": "Abm", "key_male": "Abm", "key_female": "Dbm", "scale": "Minor", "genre": "EDM Ballad", "tempo": 100},
    {"id": "hit_015", "title": "Dạ Vũ", "artist": "Tăng Duy Tân", "composer": "Tăng Duy Tân", "key_original": "Em", "key_male": "Em", "key_female": "Am", "scale": "Minor", "genre": "Deep House", "tempo": 122},
    {"id": "hit_016", "title": "Từng Là", "artist": "Vũ Cát Tường", "composer": "Vũ Cát Tường", "key_original": "C", "key_male": "C", "key_female": "F", "scale": "Major", "genre": "Bossa Nova", "tempo": 112},
    {"id": "hit_017", "title": "Bước Qua Mùa Cô Đơn", "artist": "Vũ.", "composer": "Vũ.", "key_original": "C", "key_male": "C", "key_female": "F", "scale": "Major", "genre": "Indie Pop", "tempo": 74},
    {"id": "hit_018", "title": "Bước Qua Nhau", "artist": "Vũ.", "composer": "Vũ.", "key_original": "G", "key_male": "G", "key_female": "C", "scale": "Major", "genre": "Indie Pop", "tempo": 76},
    {"id": "hit_019", "title": "Lạ Lùng", "artist": "Vũ.", "composer": "Vũ.", "key_original": "A", "key_male": "A", "key_female": "D", "scale": "Major", "genre": "Indie Acoustic", "tempo": 80},
    {"id": "hit_020", "title": "Dấu Mưa", "artist": "Trung Quân Idol", "composer": "Toàn Thắng", "key_original": "E", "key_male": "E", "key_female": "A", "scale": "Major", "genre": "Ballad", "tempo": 72},
    {"id": "hit_021", "title": "Chưa Bao Giờ", "artist": "Trung Quân Idol", "composer": "Tiên Tiên", "key_original": "Bb", "key_male": "Bb", "key_female": "Eb", "scale": "Major", "genre": "Ballad", "tempo": 70},
    {"id": "hit_022", "title": "Trót Yêu", "artist": "Trung Quân Idol", "composer": "Ái Phương", "key_original": "G", "key_male": "G", "key_female": "C", "scale": "Major", "genre": "Ballad", "tempo": 75},
    {"id": "hit_023", "title": "Tự Sự", "artist": "Orange", "composer": "Thuận Yến", "key_original": "F#m", "key_male": "C#m", "key_female": "F#m", "scale": "Minor", "genre": "Ballad", "tempo": 68},
    {"id": "hit_024", "title": "Nàng Thơ", "artist": "Hoàng Dũng", "composer": "Hoàng Dũng", "key_original": "C", "key_male": "C", "key_female": "F", "scale": "Major", "genre": "Ballad", "tempo": 72},
    {"id": "hit_025", "title": "Đôi Mươi", "artist": "Hoàng Dũng", "composer": "Hoàng Dũng", "key_original": "D", "key_male": "D", "key_female": "G", "scale": "Major", "genre": "Pop", "tempo": 108},
    {"id": "hit_026", "title": "Bao Tiền Một Mớ Bình Yên", "artist": "14 Casper, Bon Nghiêm", "composer": "14 Casper", "key_original": "C", "key_male": "C", "key_female": "F", "scale": "Major", "genre": "Indie Pop", "tempo": 84},
    {"id": "hit_027", "title": "Đưa Em Về Nhà", "artist": "Grey D, Chillies", "composer": "Grey D", "key_original": "G", "key_male": "G", "key_female": "C", "scale": "Major", "genre": "R&B Pop", "tempo": 95},
    {"id": "hit_028", "title": "Va Vào Giai Điệu Này", "artist": "MCK", "composer": "MCK", "key_original": "Am", "key_male": "Am", "key_female": "Dm", "scale": "Minor", "genre": "HipHop / R&B", "tempo": 110},
    {"id": "hit_029", "title": "Chìm Sâu", "artist": "MCK, Trung Trần", "composer": "MCK", "key_original": "F#m", "key_male": "F#m", "key_female": "Bm", "scale": "Minor", "genre": "R&B HipHop", "tempo": 98},
    {"id": "hit_030", "title": "Trốn Tìm", "artist": "Đen Vâu, MTV", "composer": "Đen Vâu", "key_original": "Am", "key_male": "Am", "key_female": "Dm", "scale": "Minor", "genre": "Rap / Acoustic", "tempo": 85},
    {"id": "hit_031", "title": "Mang Tiền Về Cho Mẹ", "artist": "Đen Vâu, Nguyên Thảo", "composer": "Đen Vâu", "key_original": "F#m", "key_male": "F#m", "key_female": "Bm", "scale": "Minor", "genre": "Rap / Ballad", "tempo": 88},
    {"id": "hit_032", "title": "Đi Về Nhà", "artist": "Đen Vâu, JustaTee", "composer": "Hứa Kim Tuyền", "key_original": "G", "key_male": "G", "key_female": "C", "scale": "Major", "genre": "Pop Rap", "tempo": 96},
    {"id": "hit_033", "title": "See Tình", "artist": "Hoàng Thùy Linh", "composer": "DTAP", "key_original": "Dm", "key_male": "Am", "key_female": "Dm", "scale": "Minor", "genre": "Disco Pop", "tempo": 124},
    {"id": "hit_034", "title": "Để Mị Nói Cho Mà Nghe", "artist": "Hoàng Thùy Linh", "composer": "DTAP", "key_original": "Em", "key_male": "Bm", "key_female": "Em", "scale": "Minor", "genre": "Folk Pop", "tempo": 118},
    {"id": "hit_035", "title": "Gieo Quẻ", "artist": "Hoàng Thùy Linh, Đen", "composer": "DTAP", "key_original": "Am", "key_male": "Em", "key_female": "Am", "scale": "Minor", "genre": "Dance Pop", "tempo": 120},
    {"id": "hit_036", "title": "Hết Thương Cạn Nhớ", "artist": "Đức Phúc", "composer": "Vương Anh Tú", "key_original": "Bb", "key_male": "Bb", "key_female": "Eb", "scale": "Major", "genre": "Ballad", "tempo": 70},
    {"id": "hit_037", "title": "Hơn Cả Yêu", "artist": "Đức Phúc", "composer": "Khắc Hưng", "key_original": "C", "key_male": "C", "key_female": "F", "scale": "Major", "genre": "Pop Ballad", "tempo": 74},
    {"id": "hit_038", "title": "Ngày Đầu Tiên", "artist": "Đức Phúc", "composer": "Khắc Hưng", "key_original": "D", "key_male": "D", "key_female": "G", "scale": "Major", "genre": "Pop", "tempo": 110},
    {"id": "hit_039", "title": "Chạm Đáy Nỗi Đau", "artist": "Erik", "composer": "Mr. Siro", "key_original": "Am", "key_male": "Am", "key_female": "Dm", "scale": "Minor", "genre": "Ballad", "tempo": 68},
    {"id": "hit_040", "title": "Em Không Sai Chúng Ta Sai", "artist": "Erik", "composer": "Nguyễn Phúc Thiện", "key_original": "G", "key_male": "G", "key_female": "C", "scale": "Major", "genre": "Ballad", "tempo": 72},
    {"id": "hit_041", "title": "Có Tất Cả Nhưng Thiếu Anh", "artist": "Erik", "composer": "Vương Anh Tú", "key_original": "F#m", "key_male": "F#m", "key_female": "Bm", "scale": "Minor", "genre": "Ballad", "tempo": 70},
    {"id": "hit_042", "title": "Có Chàng Trai Viết Lên Cây", "artist": "Phan Mạnh Quỳnh", "composer": "Phan Mạnh Quỳnh", "key_original": "G", "key_male": "G", "key_female": "C", "scale": "Major", "genre": "Pop Ballad", "tempo": 78},
    {"id": "hit_043", "title": "Vợ Người Ta", "artist": "Phan Mạnh Quỳnh", "composer": "Phan Mạnh Quỳnh", "key_original": "Am", "key_male": "Am", "key_female": "Dm", "scale": "Minor", "genre": "Pop Disco", "tempo": 126},
    {"id": "hit_044", "title": "Xuân Thì", "artist": "Hà Anh Tuấn, Phan Mạnh Quỳnh", "composer": "Phan Mạnh Quỳnh", "key_original": "E", "key_male": "E", "key_female": "A", "scale": "Major", "genre": "Ballad", "tempo": 72},
    {"id": "hit_045", "title": "Tháng Tư Là Lời Nói Dối Của Em", "artist": "Hà Anh Tuấn", "composer": "Phạm Toàn Thắng", "key_original": "A", "key_male": "A", "key_female": "D", "scale": "Major", "genre": "Ballad", "tempo": 75},
    {"id": "hit_046", "title": "Tháng Mấy Em Nhớ Anh", "artist": "Hà Anh Tuấn", "composer": "Nguyễn Minh Cường", "key_original": "G", "key_male": "G", "key_female": "C", "scale": "Major", "genre": "Ballad", "tempo": 70},
    {"id": "hit_047", "title": "Em Dạo Này", "artist": "Ngọt", "composer": "Vũ Đinh Trọng Thắng", "key_original": "C", "key_male": "C", "key_female": "F", "scale": "Major", "genre": "Indie Pop", "tempo": 115},
    {"id": "hit_048", "title": "Thấy Chưa", "artist": "Ngọt", "composer": "Vũ Đinh Trọng Thắng", "key_original": "F", "key_male": "F", "key_female": "Bb", "scale": "Major", "genre": "Indie Ballad", "tempo": 80},
    {"id": "hit_049", "title": "Mascara", "artist": "Chillies", "composer": "Chillies", "key_original": "Am", "key_male": "Am", "key_female": "Dm", "scale": "Minor", "genre": "Indie Pop", "tempo": 90},
    {"id": "hit_050", "title": "Vùng Ký Ức", "artist": "Chillies", "composer": "Chillies", "key_original": "C", "key_male": "C", "key_female": "F", "scale": "Major", "genre": "Indie Pop", "tempo": 105},
    {"id": "hit_051", "title": "Thu Cuối", "artist": "Yanbi, Mr. T, Hằng BingBoong", "composer": "Yanbi, Mr. T", "key_original": "Am", "key_male": "Am", "key_female": "Dm", "scale": "Minor", "genre": "R&B Pop", "tempo": 88},
    {"id": "hit_052", "title": "Một Đêm Say", "artist": "Thịnh Suy", "composer": "Thịnh Suy", "key_original": "F#", "key_male": "F#", "key_female": "B", "scale": "Major", "genre": "Acoustic Pop", "tempo": 92},
    {"id": "hit_053", "title": "Chuyện Rằng", "artist": "Thịnh Suy", "composer": "Thịnh Suy", "key_original": "C", "key_male": "C", "key_female": "F", "scale": "Major", "genre": "Acoustic Pop", "tempo": 85},
    {"id": "hit_054", "title": "Thắc Mắc", "artist": "Thịnh Suy", "composer": "Thịnh Suy", "key_original": "G", "key_male": "G", "key_female": "C", "scale": "Major", "genre": "Indie", "tempo": 90},
    {"id": "hit_055", "title": "24H", "artist": "LyLy, Magazine", "composer": "LyLy", "key_original": "Am", "key_male": "Em", "key_female": "Am", "scale": "Minor", "genre": "R&B Pop", "tempo": 86},
    {"id": "hit_056", "title": "Không Sao Mà Em Đây Rồi", "artist": "Suni Hạ Linh", "composer": "LyLy", "key_original": "C", "key_male": "G", "key_female": "C", "scale": "Major", "genre": "Pop Ballad", "tempo": 80},
    {"id": "hit_057", "title": "Cứ Chill Thôi", "artist": "Chillies, Suni Hạ Linh, Rhymastic", "composer": "Trần Duy Khang", "key_original": "F", "key_male": "F", "key_female": "Bb", "scale": "Major", "genre": "Pop Disco", "tempo": 118},
    {"id": "hit_058", "title": "Yêu 5", "artist": "Rhymastic", "composer": "Rhymastic", "key_original": "Abm", "key_male": "Abm", "key_female": "Dbm", "scale": "Minor", "genre": "Future Bass / R&B", "tempo": 100},
    {"id": "hit_059", "title": "Nến Và Hoa", "artist": "Rhymastic", "composer": "Rhymastic", "key_original": "Fm", "key_male": "Fm", "key_female": "Bbm", "scale": "Minor", "genre": "Trap / Soul", "tempo": 85},
    {"id": "hit_060", "title": "Hôm Nay Tôi Buồn", "artist": "Phùng Khánh Linh", "composer": "Phùng Khánh Linh", "key_original": "C", "key_male": "G", "key_female": "C", "scale": "Major", "genre": "Indie Pop", "tempo": 90},

    # Bolero & Nhạc Vàng Tuyệt Phẩm
    {"id": "bolero_001", "title": "Sầu Tím Thiệp Hồng", "artist": "Quang Lê, Lệ Quyên", "composer": "Minh Kỳ, Hoài Linh", "key_original": "Am", "key_male": "Dm", "key_female": "Am", "scale": "Minor", "genre": "Bolero", "tempo": 68},
    {"id": "bolero_002", "title": "Duyên Phận", "artist": "Như Quỳnh", "composer": "Thái Thịnh", "key_original": "Am", "key_male": "Dm", "key_female": "Am", "scale": "Minor", "genre": "Bolero", "tempo": 70},
    {"id": "bolero_003", "title": "Đoạn Tuyệt", "artist": "Lệ Quyên", "composer": "Thái Thịnh", "key_original": "Dm", "key_male": "Gm", "key_female": "Dm", "scale": "Minor", "genre": "Bolero", "tempo": 66},
    {"id": "bolero_004", "title": "Vùng Lá Me Bay", "artist": "Như Quỳnh", "composer": "Anh Việt Thanh", "key_original": "Em", "key_male": "Am", "key_female": "Em", "scale": "Minor", "genre": "Bolero", "tempo": 68},
    {"id": "bolero_005", "title": "Đắp Mộ Cuộc Tình", "artist": "Đan Nguyên", "composer": "Vũ Thanh", "key_original": "Am", "key_male": "Am", "key_female": "Dm", "scale": "Minor", "genre": "Bolero", "tempo": 72},
    {"id": "bolero_006", "title": "Hai Lối Mộng", "artist": "Quang Lê", "composer": "Trúc Phương", "key_original": "Dm", "key_male": "Dm", "key_female": "Gm", "scale": "Minor", "genre": "Bolero", "tempo": 68},
    {"id": "bolero_007", "title": "Thói Đời", "artist": "Đan Nguyên, Chế Linh", "composer": "Trúc Phương", "key_original": "Am", "key_male": "Am", "key_female": "Dm", "scale": "Minor", "genre": "Bolero", "tempo": 70},
    {"id": "bolero_008", "title": "Cô Hàng Xóm", "artist": "Quang Lê, Duy Khánh", "composer": "Lê Mộng Bảo", "key_original": "Em", "key_male": "Em", "key_female": "Am", "scale": "Minor", "genre": "Bolero", "tempo": 74},
    {"id": "bolero_009", "title": "Con Đường Xưa Em Đi", "artist": "Như Quỳnh, Hoàng Oanh", "composer": "Châu Kỳ, Hồ Đình Phương", "key_original": "Am", "key_male": "Dm", "key_female": "Am", "scale": "Minor", "genre": "Bolero", "tempo": 70},
    {"id": "bolero_010", "title": "Lại Nhớ Người Yêu", "artist": "Đan Nguyên", "composer": "Giao Tiên", "key_original": "Em", "key_male": "Em", "key_female": "Am", "scale": "Minor", "genre": "Bolero", "tempo": 76},
    {"id": "bolero_011", "title": "Người Giàu Cũng Khóc", "artist": "Trường Vũ", "composer": "Hồng Xương Long", "key_original": "Am", "key_male": "Am", "key_female": "Dm", "scale": "Minor", "genre": "Bolero", "tempo": 72},
    {"id": "bolero_012", "title": "Đêm Buồn Tỉnh Lẻ", "artist": "Đan Nguyên", "composer": "Tú Nhi, Bằng Giang", "key_original": "Dm", "key_male": "Dm", "key_female": "Gm", "scale": "Minor", "genre": "Bolero", "tempo": 68},
    {"id": "bolero_013", "title": "Xin Em Đừng Khóc Vu Quy", "artist": "Đan Nguyên", "composer": "Minh Phương", "key_original": "Am", "key_male": "Am", "key_female": "Dm", "scale": "Minor", "genre": "Bolero", "tempo": 70},
    {"id": "bolero_014", "title": "Chuyện Hoa Sim", "artist": "Như Quỳnh", "composer": "Anh Bằng", "key_original": "Am", "key_male": "Dm", "key_female": "Am", "scale": "Minor", "genre": "Bolero", "tempo": 70},
    {"id": "bolero_015", "title": "Nỗi Buồn Hoa Phượng", "artist": "Thanh Tuyền, Phi Nhung", "composer": "Thanh Sơn", "key_original": "Dm", "key_male": "Gm", "key_female": "Dm", "scale": "Minor", "genre": "Bolero", "tempo": 72},
    {"id": "bolero_016", "title": "Hạ Buồn", "artist": "Hương Lan", "composer": "Thanh Sơn", "key_original": "Am", "key_male": "Dm", "key_female": "Am", "scale": "Minor", "genre": "Bolero", "tempo": 70},
    {"id": "bolero_017", "title": "Chuyến Tàu Hoàng Hôn", "artist": "Lệ Quyên, Hoàng Oanh", "composer": "Minh Kỳ, Hoài Linh", "key_original": "Am", "key_male": "Dm", "key_female": "Am", "scale": "Minor", "genre": "Bolero", "tempo": 68},
    {"id": "bolero_018", "title": "Biển Tình", "artist": "Đàm Vĩnh Hưng, Thanh Tuyền", "composer": "Lam Phương", "key_original": "C", "key_male": "C", "key_female": "F", "scale": "Major", "genre": "Slow Rock", "tempo": 65},
    {"id": "bolero_019", "title": "Thành Phố Buồn", "artist": "Đan Nguyên, Chế Linh", "composer": "Lam Phương", "key_original": "Am", "key_male": "Am", "key_female": "Dm", "scale": "Minor", "genre": "Slow Rock", "tempo": 66},
    {"id": "bolero_020", "title": "Cỏ Úa", "artist": "Lệ Quyên, Bằng Kiều", "composer": "Lam Phương", "key_original": "Am", "key_male": "Am", "key_female": "Dm", "scale": "Minor", "genre": "Rumba", "tempo": 84},
    {"id": "bolero_021", "title": "Mưa Rừng", "artist": "Thanh Tuyền, Như Quỳnh", "composer": "Huỳnh Anh", "key_original": "Am", "key_male": "Dm", "key_female": "Am", "scale": "Minor", "genre": "Bolero", "tempo": 68},
    {"id": "bolero_022", "title": "Hoa Trinh Nữ", "artist": "Trần Thiện Thanh, Như Quỳnh", "composer": "Trần Thiện Thanh", "key_original": "C", "key_male": "C", "key_female": "F", "scale": "Major", "genre": "Bolero", "tempo": 70},
    {"id": "bolero_023", "title": "Lâu Đài Tình Ái", "artist": "Đàm Vĩnh Hưng, Mỹ Dung", "composer": "Trần Thiện Thanh", "key_original": "C", "key_male": "C", "key_female": "F", "scale": "Major", "genre": "Slow Rock", "tempo": 72},
    {"id": "bolero_024", "title": "Nhật Ký Đời Tôi", "artist": "Giao Linh", "composer": "Thanh Sơn", "key_original": "Am", "key_male": "Dm", "key_female": "Am", "scale": "Minor", "genre": "Bolero", "tempo": 68},
    {"id": "bolero_025", "title": "Giọt Lệ Sầu", "artist": "Chế Linh", "composer": "Lam Phương", "key_original": "Am", "key_male": "Am", "key_female": "Dm", "scale": "Minor", "genre": "Bolero", "tempo": 70},

    # Nhạc Trịnh Công Sơn
    {"id": "trinh_001", "title": "Diễm Xưa", "artist": "Khánh Ly, Hồng Nhung", "composer": "Trịnh Công Sơn", "key_original": "Am", "key_male": "Am", "key_female": "Dm", "scale": "Minor", "genre": "Slow Rock", "tempo": 68},
    {"id": "trinh_002", "title": "Hạ Trắng", "artist": "Khánh Ly, Quang Dũng", "composer": "Trịnh Công Sơn", "key_original": "Am", "key_male": "Am", "key_female": "Dm", "scale": "Minor", "genre": "Slow Rock", "tempo": 66},
    {"id": "trinh_003", "title": "Biển Nhớ", "artist": "Khánh Ly", "composer": "Trịnh Công Sơn", "key_original": "Am", "key_male": "Am", "key_female": "Dm", "scale": "Minor", "genre": "Slow", "tempo": 70},
    {"id": "trinh_004", "title": "Còn Tuổi Nào Cho Em", "artist": "Mỹ Tâm, Khánh Ly", "composer": "Trịnh Công Sơn", "key_original": "C", "key_male": "C", "key_female": "F", "scale": "Major", "genre": "Slow Rock", "tempo": 68},
    {"id": "trinh_005", "title": "Như Cánh Vạc Bay", "artist": "Hồng Nhung", "composer": "Trịnh Công Sơn", "key_original": "C", "key_male": "C", "key_female": "F", "scale": "Major", "genre": "Slow Rock", "tempo": 72},
    {"id": "trinh_006", "title": "Một Cõi Đi Về", "artist": "Khánh Ly, Tuấn Ngọc", "composer": "Trịnh Công Sơn", "key_original": "Am", "key_male": "Am", "key_female": "Dm", "scale": "Minor", "genre": "Boston", "tempo": 74},
    {"id": "trinh_007", "title": "Cát Bụi", "artist": "Khánh Ly, Đàm Vĩnh Hưng", "composer": "Trịnh Công Sơn", "key_original": "Am", "key_male": "Am", "key_female": "Dm", "scale": "Minor", "genre": "Slow", "tempo": 65},
    {"id": "trinh_008", "title": "Mưa Hồng", "artist": "Khánh Ly, Hà Anh Tuấn", "composer": "Trịnh Công Sơn", "key_original": "C", "key_male": "C", "key_female": "F", "scale": "Major", "genre": "Slow Rock", "tempo": 70},
    {"id": "trinh_009", "title": "Ru Em Từng Ngón Xuân Nồng", "artist": "Tuấn Ngọc", "composer": "Trịnh Công Sơn", "key_original": "Am", "key_male": "Am", "key_female": "Dm", "scale": "Minor", "genre": "Slow", "tempo": 64},
    {"id": "trinh_010", "title": "Ướt Mi", "artist": "Khánh Ly", "composer": "Trịnh Công Sơn", "key_original": "Am", "key_male": "Am", "key_female": "Dm", "scale": "Minor", "genre": "Slow", "tempo": 66},

    # Làn Sóng Xanh & Nhạc Trẻ Bất Hủ 90s - 2000s
    {"id": "xanh_001", "title": "Giấc Mơ Có Thật", "artist": "Lệ Quyên", "composer": "Tường Văn", "key_original": "Am", "key_male": "Dm", "key_female": "Am", "scale": "Minor", "genre": "Ballad", "tempo": 72},
    {"id": "xanh_002", "title": "Hãy Trả Lời Em", "artist": "Lệ Quyên", "composer": "Tuấn Nghĩa", "key_original": "Dm", "key_male": "Gm", "key_female": "Dm", "scale": "Minor", "genre": "Ballad", "tempo": 70},
    {"id": "xanh_003", "title": "Ước Gì", "artist": "Mỹ Tâm", "composer": "Võ Thiện Thanh", "key_original": "G", "key_male": "C", "key_female": "G", "scale": "Major", "genre": "Pop Ballad", "tempo": 74},
    {"id": "xanh_004", "title": "Họa Mi Tóc Nâu", "artist": "Mỹ Tâm", "composer": "Trần Huân", "key_original": "Am", "key_male": "Em", "key_female": "Am", "scale": "Minor", "genre": "Pop Dance", "tempo": 120},
    {"id": "xanh_005", "title": "Đúng Cũng Thành Sai", "artist": "Mỹ Tâm", "composer": "Khắc Hưng", "key_original": "Am", "key_male": "Dm", "key_female": "Am", "scale": "Minor", "genre": "Pop Ballad", "tempo": 78},
    {"id": "xanh_006", "title": "Nếu Em Được Chọn Lựa", "artist": "Lệ Quyên", "composer": "Thái Thịnh", "key_original": "Am", "key_male": "Dm", "key_female": "Am", "scale": "Minor", "genre": "Ballad", "tempo": 70},
    {"id": "xanh_007", "title": "Trái Tim Bên Lề", "artist": "Bằng Kiều", "composer": "Phạm Khải Tuấn", "key_original": "Am", "key_male": "Am", "key_female": "Dm", "scale": "Minor", "genre": "Pop Ballad", "tempo": 72},
    {"id": "xanh_008", "title": "Nơi Tình Yêu Bắt Đầu", "artist": "Bằng Kiều, Bùi Anh Tuấn", "composer": "Tiến Minh", "key_original": "F#m", "key_male": "F#m", "key_female": "Bm", "scale": "Minor", "genre": "Pop Ballad", "tempo": 68},
    {"id": "xanh_009", "title": "Em Gái Mưa", "artist": "Hương Tràm", "composer": "Mr. Siro", "key_original": "C", "key_male": "F", "key_female": "C", "scale": "Major", "genre": "Ballad", "tempo": 72},
    {"id": "xanh_010", "title": "Duyên Mình Lỡ", "artist": "Hương Tràm", "composer": "Tú Dưa", "key_original": "Am", "key_male": "Dm", "key_female": "Am", "scale": "Minor", "genre": "Ballad", "tempo": 70},
    {"id": "xanh_011", "title": "Gửi Người Yêu Cũ", "artist": "Hồ Ngọc Hà", "composer": "Nguyễn Hồng Thuận", "key_original": "Am", "key_male": "Dm", "key_female": "Am", "scale": "Minor", "genre": "Ballad", "tempo": 68},
    {"id": "xanh_012", "title": "Cả Một Trời Thương Nhớ", "artist": "Hồ Ngọc Hà", "composer": "Nguyễn Minh Cường", "key_original": "Dm", "key_male": "Gm", "key_female": "Dm", "scale": "Minor", "genre": "Ballad", "tempo": 70},
    {"id": "xanh_013", "title": "Chạm Khẽ Tim Anh Một Chút Thôi", "artist": "Noo Phước Thịnh", "composer": "Tăng Nhật Tuệ", "key_original": "F#m", "key_male": "F#m", "key_female": "Bm", "scale": "Minor", "genre": "Ballad", "tempo": 72},
    {"id": "xanh_014", "title": "Thương Em Là Điều Anh Không Thể Ngờ", "artist": "Noo Phước Thịnh", "composer": "Triết Phạm", "key_original": "G", "key_male": "G", "key_female": "C", "scale": "Major", "genre": "Ballad", "tempo": 74},
    {"id": "xanh_015", "title": "Phía Sau Một Cô Gái", "artist": "Soobin Hoàng Sơn", "composer": "Tiên Cookie", "key_original": "G", "key_male": "G", "key_female": "C", "scale": "Major", "genre": "Ballad", "tempo": 72},
    {"id": "xanh_016", "title": "Xe Đạp", "artist": "Thùy Chi, M4U", "composer": "Nhạc Nhật (Lời Đinh Mạnh Ninh)", "key_original": "C", "key_male": "C", "key_female": "F", "scale": "Major", "genre": "Ballad", "tempo": 74},
    {"id": "xanh_017", "title": "Giữ Em Đi", "artist": "Thùy Chi", "composer": "Tiên Tiên", "key_original": "C", "key_male": "G", "key_female": "C", "scale": "Major", "genre": "Acoustic Pop", "tempo": 80},
    {"id": "xanh_018", "title": "Cơn Mưa Ngang Qua", "artist": "Sơn Tùng M-TP", "composer": "Sơn Tùng M-TP", "key_original": "Dm", "key_male": "Dm", "key_female": "Gm", "scale": "Minor", "genre": "Dance Pop", "tempo": 128},
    {"id": "xanh_019", "title": "Nắng Ấm Xa Dần", "artist": "Sơn Tùng M-TP", "composer": "Sơn Tùng M-TP", "key_original": "Fm", "key_male": "Fm", "key_female": "Bbm", "scale": "Minor", "genre": "Dance Pop", "tempo": 130},
    {"id": "xanh_020", "title": "Âm Thầm Bên Em", "artist": "Sơn Tùng M-TP", "composer": "Sơn Tùng M-TP", "key_original": "C", "key_male": "C", "key_female": "F", "scale": "Major", "genre": "Pop Ballad", "tempo": 72}
]

def main():
    print("=== LIVE STREAM MICRO-DAW SONGBOOK BUILDER ===")
    all_songs = {s["id"]: s for s in CURATED_VIETNAMESE_HITS}
    print(f"Loaded {len(all_songs)} curated hit songs.")
    
    output_path = os.path.join(os.path.dirname(__file__), "..", "assets", "songbook.json")
    output_path = os.path.abspath(output_path)
    os.makedirs(os.path.dirname(output_path), exist_ok=True)
    
    song_list = list(all_songs.values())
    song_list.sort(key=lambda x: x["title"])
    
    with open(output_path, "w", encoding="utf-8") as f:
        json.dump(song_list, f, ensure_ascii=False, indent=2)
        
    print(f"[OK] Generated {output_path} ({len(song_list)} songs)")

if __name__ == "__main__":
    main()
