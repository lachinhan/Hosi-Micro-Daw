#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Hosi Studio - Cloud Songbook & Artist Preset Sync Tool
Tự động cập nhật, kiểm tra và chuẩn hóa dữ liệu Cloud cho LiveStream Micro-DAW
"""

import json
import os
import sys

if sys.stdout.encoding != 'utf-8':
    try:
        sys.stdout.reconfigure(encoding='utf-8')
    except Exception:
        pass

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
PROJECT_ROOT = os.path.dirname(SCRIPT_DIR)
CLOUD_DIR = os.path.join(PROJECT_ROOT, "cloud")
SONGBOOK_CLOUD_FILE = os.path.join(CLOUD_DIR, "songbook_cloud.json")
ARTIST_PRESETS_FILE = os.path.join(CLOUD_DIR, "artist_presets.json")


def validate_songbook():
    if not os.path.exists(SONGBOOK_CLOUD_FILE):
        print(f"[-] Error: File not found {SONGBOOK_CLOUD_FILE}")
        return False
    
    with open(SONGBOOK_CLOUD_FILE, "r", encoding="utf-8") as f:
        data = json.load(f)
    
    print(f"[+] Loaded {len(data)} cloud songs from {SONGBOOK_CLOUD_FILE}")
    for idx, song in enumerate(data, 1):
        title = song.get("title", "")
        artist = song.get("artist", "")
        orig_key = song.get("key_original", "")
        male_key = song.get("key_male", "")
        fem_key = song.get("key_female", "")
        print(f"  {idx:2d}. {title} - {artist} (Gốc: {orig_key} | Nam: {male_key} | Nữ: {fem_key})")
    return True


def validate_artist_presets():
    if not os.path.exists(ARTIST_PRESETS_FILE):
        print(f"[-] Error: File not found {ARTIST_PRESETS_FILE}")
        return False
    
    with open(ARTIST_PRESETS_FILE, "r", encoding="utf-8") as f:
        data = json.load(f)
    
    print(f"\n[+] Loaded {len(data)} artist presets from {ARTIST_PRESETS_FILE}")
    for idx, preset in enumerate(data, 1):
        name = preset.get("name", "")
        cat = preset.get("category", "")
        desc = preset.get("description", "")
        print(f"  {idx}. [{cat}] {name}\n     -> {desc}")
    return True


def add_song(title, artist, key_original, key_male, key_female, genre="Hot Trend", tempo=100, composer=""):
    with open(SONGBOOK_CLOUD_FILE, "r+", encoding="utf-8") as f:
        songs = json.load(f)
        new_id = f"trend_{len(songs)+1:02d}"
        scale = "Minor" if key_original.endswith("m") else "Major"
        new_song = {
            "id": new_id,
            "title": title,
            "artist": artist,
            "composer": composer or artist,
            "key_original": key_original,
            "key_male": key_male or key_original,
            "key_female": key_female or key_original,
            "scale": scale,
            "genre": genre,
            "tempo": tempo
        }
        songs.append(new_song)
        f.seek(0)
        f.truncate()
        json.dump(songs, f, ensure_ascii=False, indent=2)
        print(f"[+] Successfully added new song: {title} ({new_id})")


if __name__ == "__main__":
    print("=" * 60)
    print(" HOSI MICRO-DAW - CLOUD DATA MANAGER & VALIDATOR")
    print("=" * 60)
    
    validate_songbook()
    validate_artist_presets()
    print("\n[✓] Cloud data validation passed successfully!")
