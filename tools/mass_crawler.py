"""
Hosi Micro-DAW - Mass Songbook & Tone Crawler
Fast Multi-Threaded Crawler for HopAmViet.vn with Auto-Saving
"""

import urllib.request
import re
import html
import json
import time
import os
import sys
from concurrent.futures import ThreadPoolExecutor, as_completed

sys.stdout.reconfigure(encoding='utf-8')

HEADERS = {
    'User-Agent': 'Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/120.0.0.0 Safari/537.36'
}

def fetch_html(url, timeout=6):
    for attempt in range(2):
        try:
            req = urllib.request.Request(url, headers=HEADERS)
            with urllib.request.urlopen(req, timeout=timeout) as resp:
                return resp.read().decode('utf-8', errors='ignore')
        except Exception:
            time.sleep(0.05)
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

def parse_song_page(song_url, default_genre):
    content = fetch_html(song_url, timeout=6)
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
        
    title = html.unescape(title)
    composer = html.unescape(composer)
        
    if not title or len(title) < 2 or title.lower() in ["đăng nhập", "hợp âm việt"]:
        return None
        
    rhythm_m = re.search(r'Điệu:\s*([A-Za-z0-9\s\-]+)', content)
    genre = rhythm_m.group(1).strip() if rhythm_m else default_genre
    if not genre or len(genre) > 30:
        genre = default_genre
        
    chords = re.findall(r'\[([A-G][b#]?[m]?(?:maj7|7|sus4|m7|dim)?)\]', content)
    primary_chord = chords[0] if chords else ("Am" if "bolero" in genre.lower() else "C")
    
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
        "tempo": 75 if scale == "Minor" else 95,
        "source": "hopamviet"
    }

def save_database(all_songs, output_path):
    song_list = list(all_songs.values())
    song_list.sort(key=lambda x: x["title"])
    with open(output_path, "w", encoding="utf-8") as f:
        json.dump(song_list, f, ensure_ascii=False, indent=2)
    print(f"-> [Saved] {len(song_list)} songs written to JSON.", flush=True)

def main():
    print("=== LIVE STREAM MICRO-DAW MASS CRAWLER 2.0 ===", flush=True)
    
    category_configs = [
        ("https://hopamviet.vn/chord/category/3/nhac-tre?sort=view", "Nhạc Trẻ", 30),
        ("https://hopamviet.vn/chord/category/1/nhac-vang?sort=view", "Bolero / Nhạc Vàng", 30),
        ("https://hopamviet.vn/chord/category/2/nhac-tru-tinh?sort=view", "Nhạc Trữ Tình", 25),
        ("https://hopamviet.vn/chord/category/4/nhac-que-huong?sort=view", "Quê Hương", 15),
        ("https://hopamviet.vn/chord/category/5/nhac-trinh?sort=view", "Nhạc Trịnh", 15),
        ("https://hopamviet.vn/chord/composer/1/trinh-cong-son", "Nhạc Trịnh", 10),
        ("https://hopamviet.vn/chord/category/6/nhac-tien-chien?sort=view", "Tiền Chiến", 15),
        ("https://hopamviet.vn/chord/category/8/nhac-quoc-te?sort=view", "Nhạc Ngoại Lời Việt", 15),
        ("https://hopamviet.vn/chord/latest", "Mới Nhất", 20)
    ]
    
    output_path = os.path.join(os.path.dirname(__file__), "..", "assets", "songbook.json")
    output_path = os.path.abspath(output_path)
    all_songs = {}
    
    if os.path.exists(output_path):
        try:
            with open(output_path, "r", encoding="utf-8") as f:
                existing = json.load(f)
                for s in existing:
                    clean_k = re.sub(r'[^a-zA-Z0-9]', '', s["title"].lower())
                    if clean_k:
                        all_songs[clean_k] = s
            print(f"-> Loaded {len(all_songs)} existing base songs.", flush=True)
        except Exception:
            pass

    print("[1] Gathering URLs across categories...", flush=True)
    song_urls_map = {}
    
    def fetch_cat_page(cat_url, genre, page_num):
        sep = "&" if "?" in cat_url else "?"
        url = f"{cat_url}{sep}page={page_num}"
        content = fetch_html(url, timeout=6)
        if not content:
            return []
        found_links = re.findall(r'href="(https://hopamviet\.vn/chord/song/[^"]+)"', content)
        return [(l, genre) for l in found_links]

    cat_tasks = []
    with ThreadPoolExecutor(max_workers=30) as executor:
        for cat_url, genre, max_pages in category_configs:
            for p in range(1, max_pages + 1):
                cat_tasks.append(executor.submit(fetch_cat_page, cat_url, genre, p))
                
        for fut in as_completed(cat_tasks):
            for link, genre in fut.result():
                if link not in song_urls_map:
                    song_urls_map[link] = genre
                    
    print(f"-> Discovered {len(song_urls_map)} unique song URLs.", flush=True)
    
    # Filter out already crawled songs
    to_crawl = {}
    for url, genre in song_urls_map.items():
        slug_m = re.search(r'/song/([^/]+)/', url)
        slug = slug_m.group(1).replace("-", "") if slug_m else ""
        if slug and slug not in all_songs:
            to_crawl[url] = genre

    print(f"[2] Crawling {len(to_crawl)} new songs using 50 concurrent workers...", flush=True)
    completed_count = 0
    try:
        with ThreadPoolExecutor(max_workers=50) as executor:
            future_to_url = {
                executor.submit(parse_song_page, url, genre): url 
                for url, genre in to_crawl.items()
            }
            
            for fut in as_completed(future_to_url):
                try:
                    res = fut.result()
                    if res and res["title"]:
                        clean_key = re.sub(r'[^a-zA-Z0-9]', '', res["title"].lower())
                        if clean_key and clean_key not in all_songs:
                            all_songs[clean_key] = res
                            completed_count += 1
                            if completed_count % 100 == 0:
                                print(f"   [Crawl Progress] +{completed_count} new songs (Total: {len(all_songs)})...", flush=True)
                                save_database(all_songs, output_path)
                except Exception:
                    pass
    finally:
        save_database(all_songs, output_path)
        print(f"\n[OK] Completed! Total database has {len(all_songs)} songs.", flush=True)

if __name__ == "__main__":
    main()
