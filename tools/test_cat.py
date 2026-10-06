import urllib.request
import re
import html
import sys

sys.stdout.reconfigure(encoding='utf-8')

headers = {
    'User-Agent': 'Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36'
}

url = "https://hopamviet.vn/chord/category/3/nhac-tre.html"
req = urllib.request.Request(url, headers=headers)
with urllib.request.urlopen(req, timeout=10) as resp:
    content = resp.read().decode('utf-8', errors='ignore')

links = re.findall(r'href="([^"]+)"', content)
print(f"Total hrefs: {len(links)}")
chord_links = [l for l in links if 'chord' in l or '.html' in l]
print("Sample chord links:")
for l in list(set(chord_links))[:25]:
    print(l)
