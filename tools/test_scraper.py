import urllib.request
import re
import html
import sys

sys.stdout.reconfigure(encoding='utf-8')

headers = {
    'User-Agent': 'Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36'
}

url = "https://hopamviet.vn/chord/song/diem-xua/W8IU0FD8.html"
req = urllib.request.Request(url, headers=headers)
with urllib.request.urlopen(req, timeout=10) as response:
    content = response.read().decode('utf-8', errors='ignore')

# print raw HTML of the first singer block
m = re.search(r'(<div[^>]*class="[^"]*singer[^"]*"[^>]*>.*?</div>)', content, re.DOTALL | re.IGNORECASE)
if m:
    print("RAW SINGER BLOCK:")
    print(m.group(1))

# Also search for tone in the chords of the lyrics
chords = re.findall(r'\[([A-G][b#]?[m]?(?:maj7|7|sus4|m7|dim)?)\]', content)
print("Chords in song:", chords[:10])

# Search for initial tone / main chord
main_tone = re.search(r'data-tone="([^"]+)"', content)
if main_tone:
    print("data-tone:", main_tone.group(1))
tone_attr = re.findall(r'tone[^\w]*[:=][^\w]*([A-G][b#]?m?)', content, re.IGNORECASE)
print("Tone attributes:", tone_attr)
