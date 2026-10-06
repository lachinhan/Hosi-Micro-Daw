import base64
import os

def get_b64(path):
    if os.path.exists(path):
        ext = os.path.splitext(path)[1].lower().replace('.', '')
        mime = 'image/png' if ext == 'png' else 'image/jpeg' if ext in ['jpg', 'jpeg'] else 'image/x-icon'
        with open(path, 'rb') as f:
            return f'data:{mime};base64,' + base64.b64encode(f.read()).decode('utf-8')
    return ''

workspace = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))
vietqr_b64 = get_b64(os.path.join(workspace, "assets", "qr_vietqr.png"))
momo_b64 = get_b64(os.path.join(workspace, "assets", "qr_momo.jpg"))
paypal_b64 = get_b64(os.path.join(workspace, "assets", "qr_paypal.jpg"))
mb_b64 = get_b64(os.path.join(workspace, "assets", "qr_mbbank.jpg"))
icon_b64 = get_b64(os.path.join(workspace, "assets", "icon.png"))

html_content = f"""<!DOCTYPE html>
<html lang="vi">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>LiveStream Micro-DAW - Phần Mềm Hát Live & Thu Âm Tự Động Dò Tone Chuyên Nghiệp | lachinhan.xyz</title>
  <meta name="description" content="LiveStream Micro-DAW là phần mềm âm thanh chuyên nghiệp dành riêng cho Streamer và Ca sĩ hát Live: Tự động dò Tone nhạc Karaoke, 1-Click đồng bộ Auto-Tune, truyền âm thanh sang OBS Studio độ trễ 0ms.">
  <meta name="keywords" content="LiveStream Micro-DAW, Hát Live Auto-Tune, Dò tone karaoke, lachinhan.xyz, phần mềm livestream hát karaoke, VST3 Host, OBS Studio Receiver">
  <meta name="author" content="La Chí Nhân (lachinhan.xyz)">
  
  <!-- Open Graph -->
  <meta property="og:title" content="LiveStream Micro-DAW - Phòng Thu Hát Live & Dò Tone Chuyên Nghiệp">
  <meta property="og:description" content="Tự động dò Tone beat karaoke, 1-Click gán vào Auto-Tune, tích hợp sẵn 1.033+ bài hát Việt Nam, truyền âm thanh sang OBS Studio không cần dây cáp ảo.">
  <meta property="og:type" content="website">
  
  <!-- Google Fonts -->
  <link rel="preconnect" href="https://fonts.googleapis.com">
  <link rel="preconnect" href="https://fonts.gstatic.com" crossorigin>
  <link href="https://fonts.googleapis.com/css2?family=Outfit:wght@400;500;600;700;800;900&family=Plus+Jakarta+Sans:wght@400;500;600;700;800&family=JetBrains+Mono:wght@400;600;700&display=swap" rel="stylesheet">

  <style>
    :root {{
      --bg-dark: #070a13;
      --bg-card: rgba(15, 23, 42, 0.75);
      --bg-card-hover: rgba(30, 41, 59, 0.85);
      --border-color: rgba(56, 189, 248, 0.18);
      --border-glow: rgba(56, 189, 248, 0.4);
      --accent-blue: #38bdf8;
      --accent-cyan: #06b6d4;
      --accent-emerald: #10b981;
      --accent-purple: #a855f7;
      --accent-rose: #f43f5e;
      --accent-amber: #f59e0b;
      --text-main: #f8fafc;
      --text-muted: #94a3b8;
      --font-heading: 'Outfit', sans-serif;
      --font-body: 'Plus Jakarta Sans', sans-serif;
      --font-mono: 'JetBrains Mono', monospace;
    }}

    * {{
      box-sizing: border-box;
      margin: 0;
      padding: 0;
    }}

    body {{
      background-color: var(--bg-dark);
      color: var(--text-main);
      font-family: var(--font-body);
      line-height: 1.65;
      overflow-x: hidden;
      position: relative;
    }}

    /* Background Ambient Glows */
    .ambient-glow {{
      position: fixed;
      width: 600px;
      height: 600px;
      border-radius: 50%;
      filter: blur(140px);
      pointer-events: none;
      z-index: 0;
      opacity: 0.18;
    }}
    .glow-1 {{ top: -100px; left: -100px; background: #0284c7; }}
    .glow-2 {{ top: 40%; right: -150px; background: #9333ea; }}
    .glow-3 {{ bottom: -100px; left: 20%; background: #059669; }}

    .container {{
      max-width: 1200px;
      margin: 0 auto;
      padding: 0 24px;
      position: relative;
      z-index: 1;
    }}

    /* Top Navbar */
    header.site-header {{
      position: sticky;
      top: 0;
      background: rgba(7, 10, 19, 0.85);
      backdrop-filter: blur(16px);
      border-bottom: 1px solid rgba(255, 255, 255, 0.08);
      z-index: 100;
      padding: 16px 0;
    }}
    .nav-wrapper {{
      display: flex;
      align-items: center;
      justify-content: space-between;
    }}
    .logo-badge {{
      display: flex;
      align-items: center;
      gap: 12px;
      text-decoration: none;
    }}
    .logo-icon {{
      width: 42px;
      height: 42px;
      border-radius: 10px;
      object-fit: cover;
      box-shadow: 0 0 20px rgba(56, 189, 248, 0.4);
    }}
    .logo-text {{
      font-family: var(--font-heading);
      font-weight: 800;
      font-size: 1.25rem;
      background: linear-gradient(135deg, #fff 30%, #38bdf8 100%);
      -webkit-background-clip: text;
      -webkit-text-fill-color: transparent;
      letter-spacing: -0.5px;
    }}
    .nav-links {{
      display: flex;
      align-items: center;
      gap: 24px;
      list-style: none;
    }}
    .nav-links a {{
      color: var(--text-muted);
      text-decoration: none;
      font-size: 0.92rem;
      font-weight: 600;
      transition: all 0.2s ease;
    }}
    .nav-links a:hover {{
      color: var(--accent-blue);
    }}
    .btn-nav-download {{
      background: linear-gradient(135deg, #0284c7, #06b6d4);
      color: #fff !important;
      padding: 8px 18px;
      border-radius: 8px;
      box-shadow: 0 4px 14px rgba(2, 132, 199, 0.35);
      transition: transform 0.2s ease, box-shadow 0.2s ease !important;
    }}
    .btn-nav-download:hover {{
      transform: translateY(-2px);
      box-shadow: 0 6px 20px rgba(2, 132, 199, 0.55);
    }}

    /* Hero Section */
    section.hero {{
      padding: 90px 0 60px;
      text-align: center;
    }}
    .hero-badge {{
      display: inline-flex;
      align-items: center;
      gap: 8px;
      background: rgba(56, 189, 248, 0.1);
      border: 1px solid rgba(56, 189, 248, 0.3);
      padding: 6px 16px;
      border-radius: 999px;
      font-size: 0.85rem;
      font-weight: 700;
      color: var(--accent-blue);
      margin-bottom: 24px;
      letter-spacing: 0.5px;
      text-transform: uppercase;
    }}
    .hero h1 {{
      font-family: var(--font-heading);
      font-size: 3.5rem;
      font-weight: 900;
      line-height: 1.15;
      letter-spacing: -1.5px;
      margin-bottom: 20px;
    }}
    .hero h1 span.gradient-text {{
      background: linear-gradient(135deg, #38bdf8 0%, #818cf8 50%, #c084fc 100%);
      -webkit-background-clip: text;
      -webkit-text-fill-color: transparent;
    }}
    .hero p.lead {{
      font-size: 1.25rem;
      color: var(--text-muted);
      max-width: 820px;
      margin: 0 auto 36px;
      font-weight: 400;
    }}
    .hero-actions {{
      display: flex;
      justify-content: center;
      gap: 16px;
      flex-wrap: wrap;
      margin-bottom: 50px;
    }}
    .btn-primary {{
      background: linear-gradient(135deg, #0284c7, #2563eb);
      color: #fff;
      padding: 15px 32px;
      border-radius: 12px;
      font-weight: 700;
      font-size: 1.05rem;
      text-decoration: none;
      display: inline-flex;
      align-items: center;
      gap: 10px;
      box-shadow: 0 8px 24px rgba(37, 99, 235, 0.4);
      transition: all 0.25s ease;
      border: 1px solid rgba(255, 255, 255, 0.15);
    }}
    .btn-primary:hover {{
      transform: translateY(-3px);
      box-shadow: 0 12px 32px rgba(37, 99, 235, 0.6);
      background: linear-gradient(135deg, #0369a1, #1d4ed8);
    }}
    .btn-secondary {{
      background: rgba(30, 41, 59, 0.7);
      color: #fff;
      padding: 15px 28px;
      border-radius: 12px;
      font-weight: 700;
      font-size: 1.05rem;
      text-decoration: none;
      display: inline-flex;
      align-items: center;
      gap: 10px;
      border: 1px solid rgba(255, 255, 255, 0.1);
      backdrop-filter: blur(10px);
      transition: all 0.25s ease;
    }}
    .btn-secondary:hover {{
      background: rgba(51, 65, 85, 0.9);
      border-color: var(--accent-blue);
      transform: translateY(-3px);
    }}
    .btn-donate {{
      background: linear-gradient(135deg, #d97706, #f59e0b);
      color: #000;
      padding: 15px 28px;
      border-radius: 12px;
      font-weight: 800;
      font-size: 1.05rem;
      text-decoration: none;
      display: inline-flex;
      align-items: center;
      gap: 10px;
      box-shadow: 0 8px 24px rgba(245, 158, 11, 0.35);
      transition: all 0.25s ease;
    }}
    .btn-donate:hover {{
      transform: translateY(-3px);
      box-shadow: 0 12px 32px rgba(245, 158, 11, 0.55);
    }}

    /* App Preview Window Mockup */
    .app-preview-wrapper {{
      max-width: 1060px;
      margin: 0 auto;
      border-radius: 16px;
      background: #0f172a;
      border: 1px solid rgba(56, 189, 248, 0.3);
      box-shadow: 0 25px 60px -15px rgba(0, 0, 0, 0.8), 0 0 40px rgba(56, 189, 248, 0.15);
      overflow: hidden;
      text-align: left;
    }}
    .window-header {{
      background: #1e293b;
      padding: 12px 18px;
      display: flex;
      align-items: center;
      justify-content: space-between;
      border-bottom: 1px solid rgba(255, 255, 255, 0.08);
    }}
    .window-controls {{
      display: flex;
      gap: 8px;
    }}
    .win-dot {{
      width: 12px;
      height: 12px;
      border-radius: 50%;
    }}
    .win-dot.red {{ background: #ef4444; }}
    .win-dot.yellow {{ background: #f59e0b; }}
    .win-dot.green {{ background: #10b981; }}
    .window-title {{
      font-family: var(--font-heading);
      font-size: 0.85rem;
      font-weight: 700;
      color: #94a3b8;
    }}
    .preview-body {{
      padding: 24px;
      background: radial-gradient(circle at top left, #1e293b 0%, #0b0f19 100%);
    }}
    .mock-topbar {{
      display: flex;
      gap: 10px;
      flex-wrap: wrap;
      margin-bottom: 20px;
    }}
    .mock-btn {{
      padding: 7px 14px;
      border-radius: 6px;
      font-size: 0.8rem;
      font-weight: 700;
      border: 1px solid rgba(255,255,255,0.1);
    }}
    .mock-btn.live {{ background: #059669; color: #fff; }}
    .mock-btn.talk {{ background: #2563eb; color: #fff; }}
    .mock-btn.tune {{ background: #9333ea; color: #fff; }}
    .mock-btn.duck {{ background: #064e3b; color: #34d399; border-color: #34d399; }}
    .mock-btn.songbook {{ background: #0284c7; color: #fff; }}

    .mock-grid {{
      display: grid;
      grid-template-columns: 2fr 1fr;
      gap: 16px;
    }}
    .mock-rack {{
      display: flex;
      flex-direction: column;
      gap: 8px;
    }}
    .mock-slot {{
      background: rgba(15, 23, 42, 0.8);
      border: 1px solid rgba(255,255,255,0.06);
      padding: 10px 14px;
      border-radius: 8px;
      display: flex;
      justify-content: space-between;
      align-items: center;
      font-size: 0.82rem;
    }}
    .mock-slot .slot-title {{ font-weight: 700; color: #e2e8f0; }}
    .mock-slot .slot-plugin {{ color: #38bdf8; font-family: var(--font-mono); font-size: 0.75rem; }}
    .mock-slot .slot-actions {{ display: flex; gap: 6px; }}
    .mock-tag {{
      background: #1e293b;
      padding: 3px 8px;
      border-radius: 4px;
      font-size: 0.7rem;
      color: #cbd5e1;
    }}

    .mock-side {{
      background: rgba(15, 23, 42, 0.9);
      border: 1px solid rgba(255,255,255,0.06);
      border-radius: 8px;
      padding: 14px;
      display: flex;
      flex-direction: column;
      gap: 12px;
    }}
    .meter-bar {{
      height: 10px;
      background: #334155;
      border-radius: 4px;
      overflow: hidden;
      position: relative;
    }}
    .meter-fill {{
      height: 100%;
      background: linear-gradient(90deg, #10b981 60%, #f59e0b 85%, #ef4444 100%);
      width: 78%;
      animation: pulseMeter 2.5s infinite alternate;
    }}
    @keyframes pulseMeter {{
      0% {{ width: 45%; }}
      50% {{ width: 88%; }}
      100% {{ width: 65%; }}
    }}

    /* Section Title */
    .section-header {{
      text-align: center;
      margin-bottom: 50px;
    }}
    .section-tag {{
      color: var(--accent-blue);
      font-weight: 700;
      font-size: 0.9rem;
      text-transform: uppercase;
      letter-spacing: 1px;
      margin-bottom: 8px;
    }}
    .section-title {{
      font-family: var(--font-heading);
      font-size: 2.5rem;
      font-weight: 800;
      letter-spacing: -0.5px;
    }}
    .section-desc {{
      color: var(--text-muted);
      max-width: 650px;
      margin: 10px auto 0;
      font-size: 1.05rem;
    }}

    /* Feature Grid */
    section.features {{
      padding: 90px 0;
    }}
    .features-grid {{
      display: grid;
      grid-template-columns: repeat(auto-fit, minmax(350px, 1fr));
      gap: 24px;
    }}
    .feature-card {{
      background: var(--bg-card);
      border: 1px solid var(--border-color);
      padding: 32px 28px;
      border-radius: 16px;
      backdrop-filter: blur(12px);
      transition: all 0.3s ease;
      position: relative;
      overflow: hidden;
    }}
    .feature-card:hover {{
      background: var(--bg-card-hover);
      border-color: var(--accent-blue);
      transform: translateY(-5px);
      box-shadow: 0 15px 35px -10px rgba(56, 189, 248, 0.2);
    }}
    .feature-icon {{
      width: 52px;
      height: 52px;
      border-radius: 12px;
      display: flex;
      align-items: center;
      justify-content: center;
      font-size: 1.7rem;
      margin-bottom: 20px;
      background: rgba(56, 189, 248, 0.1);
      border: 1px solid rgba(56, 189, 248, 0.2);
    }}
    .feature-card h3 {{
      font-family: var(--font-heading);
      font-size: 1.35rem;
      font-weight: 700;
      margin-bottom: 12px;
      color: #fff;
    }}
    .feature-card p {{
      color: var(--text-muted);
      font-size: 0.95rem;
      line-height: 1.6;
    }}
    .feature-badge {{
      display: inline-block;
      margin-top: 14px;
      font-family: var(--font-mono);
      font-size: 0.75rem;
      font-weight: 600;
      padding: 3px 10px;
      border-radius: 6px;
      background: rgba(255, 255, 255, 0.06);
      color: #38bdf8;
    }}

    /* Songbook Showcase Section */
    section.songbook-showcase {{
      padding: 80px 0;
      background: linear-gradient(180deg, transparent 0%, rgba(15, 23, 42, 0.6) 100%);
    }}
    .songbook-box {{
      background: #0f172a;
      border: 1px solid rgba(56, 189, 248, 0.3);
      border-radius: 20px;
      padding: 40px;
      box-shadow: 0 20px 50px rgba(0,0,0,0.5);
    }}
    .song-item-demo {{
      background: #1e293b;
      border-radius: 10px;
      padding: 12px 18px;
      display: flex;
      align-items: center;
      justify-content: space-between;
      margin-bottom: 10px;
      border: 1px solid rgba(255,255,255,0.06);
    }}
    .song-title-wrap {{
      display: flex;
      align-items: center;
      gap: 12px;
    }}
    .song-name {{ font-weight: 700; color: #fff; font-size: 0.95rem; }}
    .song-meta {{ color: #94a3b8; font-size: 0.8rem; }}
    .tone-tag {{
      padding: 4px 10px;
      border-radius: 6px;
      font-family: var(--font-mono);
      font-weight: 700;
      font-size: 0.8rem;
    }}
    .tone-male {{ background: rgba(37, 99, 235, 0.25); color: #60a5fa; border: 1px solid #3b82f6; }}
    .tone-female {{ background: rgba(219, 39, 119, 0.25); color: #f472b6; border: 1px solid #ec4899; }}

    /* Download Section */
    section.download-section {{
      padding: 90px 0;
    }}
    .download-grid {{
      display: grid;
      grid-template-columns: repeat(auto-fit, minmax(320px, 1fr));
      gap: 30px;
      max-width: 900px;
      margin: 0 auto;
    }}
    .download-card {{
      background: linear-gradient(180deg, #1e293b 0%, #0f172a 100%);
      border: 2px solid rgba(56, 189, 248, 0.3);
      border-radius: 20px;
      padding: 36px 30px;
      text-align: center;
      position: relative;
      transition: all 0.3s ease;
    }}
    .download-card.featured {{
      border-color: #38bdf8;
      box-shadow: 0 0 40px rgba(56, 189, 248, 0.25);
    }}
    .download-card.featured::before {{
      content: "KHUYÊN DÙNG (TỰ CÀI OBS VST)";
      position: absolute;
      top: -14px;
      left: 50%;
      transform: translateX(-50%);
      background: linear-gradient(135deg, #0284c7, #06b6d4);
      color: #fff;
      font-size: 0.72rem;
      font-weight: 800;
      padding: 4px 14px;
      border-radius: 999px;
      letter-spacing: 0.5px;
    }}
    .dl-icon {{
      font-size: 3rem;
      margin-bottom: 16px;
    }}
    .dl-title {{
      font-family: var(--font-heading);
      font-size: 1.5rem;
      font-weight: 800;
      margin-bottom: 8px;
    }}
    .dl-desc {{
      color: var(--text-muted);
      font-size: 0.9rem;
      margin-bottom: 24px;
      min-height: 48px;
    }}
    .btn-dl-action {{
      display: block;
      width: 100%;
      padding: 14px 0;
      border-radius: 10px;
      font-weight: 800;
      font-size: 1rem;
      text-decoration: none;
      transition: all 0.2s ease;
    }}
    .btn-dl-action.primary {{
      background: linear-gradient(135deg, #0284c7, #2563eb);
      color: #fff;
      box-shadow: 0 4px 15px rgba(2, 132, 199, 0.4);
    }}
    .btn-dl-action.primary:hover {{
      background: linear-gradient(135deg, #0369a1, #1d4ed8);
      transform: translateY(-2px);
    }}
    .btn-dl-action.outline {{
      background: rgba(255, 255, 255, 0.05);
      color: #fff;
      border: 1px solid rgba(255, 255, 255, 0.2);
    }}
    .btn-dl-action.outline:hover {{
      background: rgba(255, 255, 255, 0.1);
      border-color: #fff;
      transform: translateY(-2px);
    }}
    .dl-meta {{
      margin-top: 14px;
      font-size: 0.8rem;
      color: #64748b;
    }}

    /* OBS Setup Guide */
    section.guide-section {{
      padding: 80px 0;
      background: rgba(15, 23, 42, 0.4);
    }}
    .steps-grid {{
      display: grid;
      grid-template-columns: repeat(auto-fit, minmax(280px, 1fr));
      gap: 24px;
    }}
    .step-box {{
      background: #0f172a;
      border: 1px solid rgba(255,255,255,0.08);
      padding: 28px 24px;
      border-radius: 14px;
      position: relative;
    }}
    .step-num {{
      width: 36px;
      height: 36px;
      border-radius: 50%;
      background: rgba(56, 189, 248, 0.15);
      border: 1px solid var(--accent-blue);
      color: var(--accent-blue);
      font-weight: 800;
      display: flex;
      align-items: center;
      justify-content: center;
      margin-bottom: 16px;
    }}
    .step-box h4 {{
      font-family: var(--font-heading);
      font-size: 1.15rem;
      font-weight: 700;
      margin-bottom: 10px;
    }}
    .step-box p {{
      color: var(--text-muted);
      font-size: 0.9rem;
      line-height: 1.55;
    }}

    /* Donate Section */
    section.donate-section {{
      padding: 90px 0;
    }}
    .donate-container {{
      max-width: 820px;
      margin: 0 auto;
      background: linear-gradient(135deg, rgba(30, 41, 59, 0.9) 0%, rgba(15, 23, 42, 0.95) 100%);
      border: 2px solid rgba(245, 158, 11, 0.4);
      box-shadow: 0 0 50px rgba(245, 158, 11, 0.15);
      border-radius: 24px;
      padding: 40px;
      backdrop-filter: blur(16px);
    }}
    .donate-header {{
      text-align: center;
      margin-bottom: 30px;
    }}
    .donate-badge {{
      display: inline-block;
      background: rgba(245, 158, 11, 0.15);
      border: 1px solid #f59e0b;
      color: #f59e0b;
      font-weight: 800;
      font-size: 0.8rem;
      padding: 4px 14px;
      border-radius: 999px;
      margin-bottom: 12px;
    }}
    .donate-header h2 {{
      font-family: var(--font-heading);
      font-size: 2rem;
      font-weight: 800;
      color: #fff;
    }}
    .donate-tabs {{
      display: flex;
      justify-content: center;
      gap: 10px;
      flex-wrap: wrap;
      margin-bottom: 30px;
    }}
    .tab-btn {{
      background: #0f172a;
      border: 1px solid rgba(255, 255, 255, 0.12);
      color: #94a3b8;
      padding: 10px 20px;
      border-radius: 10px;
      font-weight: 700;
      font-size: 0.92rem;
      cursor: pointer;
      transition: all 0.2s ease;
    }}
    .tab-btn:hover {{
      color: #fff;
      border-color: #f59e0b;
    }}
    .tab-btn.active {{
      background: linear-gradient(135deg, #d97706, #f59e0b);
      color: #000;
      border-color: #f59e0b;
    }}

    .donate-card-content {{
      display: grid;
      grid-template-columns: 280px 1fr;
      gap: 32px;
      align-items: center;
    }}
    @media (max-width: 768px) {{
      .donate-card-content {{
        grid-template-columns: 1fr;
        text-align: center;
      }}
    }}
    .qr-image-wrapper {{
      background: #fff;
      padding: 12px;
      border-radius: 16px;
      box-shadow: 0 10px 30px rgba(0,0,0,0.4);
      display: flex;
      align-items: center;
      justify-content: center;
    }}
    .qr-image {{
      width: 100%;
      max-width: 250px;
      height: auto;
      border-radius: 8px;
      display: block;
    }}
    .donate-details {{
      display: flex;
      flex-direction: column;
      gap: 14px;
    }}
    .info-row {{
      background: rgba(15, 23, 42, 0.7);
      border: 1px solid rgba(255, 255, 255, 0.08);
      padding: 12px 16px;
      border-radius: 10px;
      display: flex;
      justify-content: space-between;
      align-items: center;
    }}
    .info-label {{
      font-size: 0.82rem;
      color: #94a3b8;
      text-transform: uppercase;
      font-weight: 600;
    }}
    .info-value {{
      font-family: var(--font-mono);
      font-weight: 700;
      font-size: 1.1rem;
      color: #f8fafc;
    }}
    .copy-btn {{
      background: #0284c7;
      color: #fff;
      border: none;
      padding: 6px 14px;
      border-radius: 6px;
      font-weight: 700;
      font-size: 0.8rem;
      cursor: pointer;
      transition: all 0.2s ease;
    }}
    .copy-btn:hover {{
      background: #0369a1;
      transform: scale(1.05);
    }}
    .toast-copy {{
      display: none;
      color: #10b981;
      font-size: 0.85rem;
      font-weight: 700;
      text-align: center;
      margin-top: 10px;
    }}

    /* Support / Service Box */
    .support-box {{
      margin-top: 24px;
      padding: 16px 20px;
      background: rgba(2, 132, 199, 0.1);
      border: 1px dashed rgba(56, 189, 248, 0.4);
      border-radius: 12px;
      font-size: 0.88rem;
      color: #cbd5e1;
      line-height: 1.6;
    }}
    .support-box strong {{
      color: #38bdf8;
    }}

    /* Footer */
    footer.site-footer {{
      border-top: 1px solid rgba(255, 255, 255, 0.08);
      padding: 50px 0 30px;
      text-align: center;
      color: var(--text-muted);
      font-size: 0.9rem;
    }}
    .footer-logo {{
      font-family: var(--font-heading);
      font-weight: 800;
      font-size: 1.3rem;
      color: #fff;
      margin-bottom: 12px;
    }}
    .footer-links {{
      display: flex;
      justify-content: center;
      gap: 20px;
      list-style: none;
      margin: 20px 0;
    }}
    .footer-links a {{
      color: var(--text-muted);
      text-decoration: none;
      transition: color 0.2s ease;
    }}
    .footer-links a:hover {{
      color: var(--accent-blue);
    }}
  </style>
</head>
<body>

  <!-- Ambient background glow effects -->
  <div class="ambient-glow glow-1"></div>
  <div class="ambient-glow glow-2"></div>
  <div class="ambient-glow glow-3"></div>

  <!-- Header -->
  <header class="site-header">
    <div class="container nav-wrapper">
      <a href="#" class="logo-badge">
        <img src="{icon_b64}" alt="LiveStream Micro-DAW Logo" class="logo-icon">
        <span class="logo-text">LiveStream Micro-DAW</span>
      </a>
      <ul class="nav-links">
        <li><a href="#features">Tính Năng</a></li>
        <li><a href="#songbook">Sổ Tone 1000+</a></li>
        <li><a href="#guide">Kết Nối OBS</a></li>
        <li><a href="#donate">Ủng Hộ (Donate)</a></li>
        <li><a href="#download" class="btn-nav-download">Tải Về</a></li>
      </ul>
    </div>
  </header>

  <!-- Hero Section -->
  <section class="hero">
    <div class="container">
      <div class="hero-badge">⚡ Phiên Bản 2.0 Chính Thức • Zero-Latency IPC Engine</div>
      <h1>
        Phần Mềm Hát Live & Thu Âm<br>
        <span class="gradient-text">Tự Động Dò Tone Số 1 Cho Streamer</span>
      </h1>
      <p class="lead">
        Tự động nhận diện Tone bài hát Karaoke và giọng hát Micro, 1-Click đồng bộ Auto-Tune Pro, 
        truyền âm thanh độ trễ 0ms sang OBS Studio không cần dây cáp ảo. Tích hợp sẵn 1.033+ bài hát Việt Nam.
      </p>
      
      <div class="hero-actions">
        <a href="#download" class="btn-primary">
          <span>📥 Tải Miễn Phí (Setup / Portable)</span>
        </a>
        <a href="#features" class="btn-secondary">
          <span>✨ Xem Tính Năng Nổi Bật</span>
        </a>
        <a href="#donate" class="btn-donate">
          <span>☕ Ủng Hộ Tác Giả</span>
        </a>
      </div>

      <!-- App Mockup Preview -->
      <div class="app-preview-wrapper">
        <div class="window-header">
          <div class="window-controls">
            <div class="win-dot red"></div>
            <div class="win-dot yellow"></div>
            <div class="win-dot green"></div>
          </div>
          <div class="window-title">LiveStream Micro-DAW v2.0 - Studio Host Engine</div>
          <div style="font-size: 0.75rem; color: #38bdf8; font-family: var(--font-mono);">ASIO 48.0 kHz | 5.3 ms</div>
        </div>

        <div class="preview-body">
          <!-- Top Button Bar -->
          <div class="mock-topbar">
            <div class="mock-btn live">🎤 LIVE (F1)</div>
            <div class="mock-btn talk">💬 TALK (F2)</div>
            <div class="mock-btn tune">🔥 TUNE (F3)</div>
            <div class="mock-btn duck">🎙️ DUCK -12dB</div>
            <div class="mock-btn songbook">🎵 SỔ TONE (1.033 BÀI)</div>
            <div style="margin-left: auto; display: flex; align-items: center; gap: 8px; font-family: var(--font-mono); font-size: 0.8rem; color: #f59e0b;">
              <span>DÒ TONE:</span> <strong style="color:#38bdf8; font-size: 0.95rem;">Em (E Minor)</strong>
            </div>
          </div>

          <!-- Main Grid -->
          <div class="mock-grid">
            <!-- Rack 8 Slots -->
            <div class="mock-rack">
              <div class="mock-slot">
                <div>
                  <div class="mock-slot-title">1. Pitch Correction / Auto-Tune</div>
                  <div class="mock-slot-plugin">Auto-Tune Pro [VST3]</div>
                </div>
                <div class="mock-slot-actions">
                  <span class="mock-tag" style="background:#0284c7; color:#fff;">INSERT</span>
                  <span class="mock-tag">EDIT</span>
                </div>
              </div>

              <div class="mock-slot">
                <div>
                  <div class="mock-slot-title">2. Noise Gate (Chống Ồn)</div>
                  <div class="mock-slot-plugin">Denoiser Studio [VST3]</div>
                </div>
                <div class="mock-slot-actions">
                  <span class="mock-tag" style="background:#0284c7; color:#fff;">INSERT</span>
                  <span class="mock-tag">EDIT</span>
                </div>
              </div>

              <div class="mock-slot">
                <div>
                  <div class="mock-slot-title">3. Subtractive EQ & Clean Vocal</div>
                  <div class="mock-slot-plugin">FabFilter Pro-Q 3 [VST3]</div>
                </div>
                <div class="mock-slot-actions">
                  <span class="mock-tag" style="background:#0284c7; color:#fff;">INSERT</span>
                  <span class="mock-tag">EDIT</span>
                </div>
              </div>

              <div class="mock-slot">
                <div>
                  <div class="mock-slot-title">4. Serial Vocal Compressor</div>
                  <div class="mock-slot-plugin">CLA-76 Vocal Comp [VST3]</div>
                </div>
                <div class="mock-slot-actions">
                  <span class="mock-tag" style="background:#0284c7; color:#fff;">INSERT</span>
                  <span class="mock-tag">EDIT</span>
                </div>
              </div>

              <div class="mock-slot">
                <div>
                  <div class="mock-slot-title">7. Spatial Lush Reverb</div>
                  <div class="mock-slot-plugin">Valhalla VintageVerb [VST3]</div>
                </div>
                <div class="mock-slot-actions">
                  <span class="mock-tag" style="background:#9333ea; color:#fff;">AUX SEND</span>
                  <span class="mock-tag">-18 dB</span>
                </div>
              </div>
            </div>

            <!-- Side Meter & Controls -->
            <div class="mock-side">
              <div style="font-size:0.75rem; font-weight:700; color:#38bdf8; text-transform:uppercase;">MASTER OUTPUT (0 dB)</div>
              <div>
                <div style="display:flex; justify-content:space-between; font-size:0.7rem; color:#64748b; margin-bottom:4px;">
                  <span>L</span> <span>-6 dB</span>
                </div>
                <div class="meter-bar"><div class="meter-fill"></div></div>
              </div>
              <div>
                <div style="display:flex; justify-content:space-between; font-size:0.7rem; color:#64748b; margin-bottom:4px;">
                  <span>R</span> <span>-6 dB</span>
                </div>
                <div class="meter-bar"><div class="meter-fill" style="width:72%;"></div></div>
              </div>

              <div style="margin-top:auto; padding:10px; background:#1e293b; border-radius:6px; font-size:0.75rem; text-align:center; color:#34d399; font-weight:700;">
                ⚡ OBS RECEIVER LINKED (0ms)
              </div>
            </div>
          </div>
        </div>
      </div>
    </div>
  </section>

  <!-- Features Section -->
  <section class="features" id="features">
    <div class="container">
      <div class="section-header">
        <div class="section-tag">Công Nghệ Đột Phá</div>
        <h2 class="section-title">Tại Sao Streamer Chọn LiveStream Micro-DAW?</h2>
        <p class="section-desc">Không còn phải vật lộn với những phần mềm DAW thu âm cồng kềnh khó dùng. Mọi tính năng bạn cần cho buổi livestream hoàn hảo đều nằm tại đây.</p>
      </div>

      <div class="features-grid">
        <!-- Feature 1 -->
        <div class="feature-card">
          <div class="feature-icon">🎵</div>
          <h3>Tự Động Dò Tone Beat & Micro</h3>
          <p>Thuật toán phân tích phổ âm Harmonic Chromagram nhận diện chính xác Tone (Trưởng/Thứ) của beat karaoke hoặc giọng hát mộc. 1-Click đồng bộ thẳng nốt vào Auto-Tune Pro.</p>
          <span class="feature-badge">Auto-Key & Scale Detection</span>
        </div>

        <!-- Feature 2 -->
        <div class="feature-card">
          <div class="feature-icon">📖</div>
          <h3>Sổ Tone 1.033+ Bài Hát Việt Nam</h3>
          <p>Tích hợp sẵn kho dữ liệu bài hát khổng lồ (Bolero, Nhạc Trẻ, Nhạc Trịnh, Ballad...) kèm đầy đủ Tone Nam, Tone Nữ, Tone Gốc, nút thả tim Yêu thích và tìm kiếm tiếng Việt tức thì.</p>
          <span class="feature-badge">Smart Songbook & Auto-Key</span>
        </div>

        <!-- Feature 3 -->
        <div class="feature-card">
          <div class="feature-icon">🎙️</div>
          <h3>Smart Auto-Ducking Pro (Talk-Over)</h3>
          <p>Tự động hạ nhỏ âm lượng beat khi bạn cất giọng nói chuyện và đẩy mượt trở lại khi ngừng nói. Menu chuột phải cho phép chỉnh mức giảm dB, độ nhạy micro và hold time như đài radio.</p>
          <span class="feature-badge">Sidechain DSP 40ms Smooth Ramp</span>
        </div>

        <!-- Feature 4 -->
        <div class="feature-card">
          <div class="feature-icon">⚡</div>
          <h3>Zero-Latency IPC Sang OBS Studio</h3>
          <p>Truyền âm thanh 64-bit trực tiếp sang OBS Studio qua bộ nhớ chia sẻ Windows Named Shared Memory. Độ trễ = 0ms, không còn cần cài đặt Virtual Audio Cable hay Voicemeeter!</p>
          <span class="feature-badge">Plugin OBS Receiver VST2 / VST3</span>
        </div>

        <!-- Feature 5 -->
        <div class="feature-card">
          <div class="feature-icon">🎛️</div>
          <h3>Rack 8 Slot VST3 & Built-In Vocal DSP</h3>
          <p>Load mọi plugin VST3 chuyên nghiệp (Auto-Tune, FabFilter, Waves, Soundtoys...) kèm bộ 6 hiệu ứng giọng hát tích hợp sẵn (Noise Gate, EQ 3-Band, Comp, Reverb, Echo, Limiter).</p>
          <span class="feature-badge">64-bit Floating-Point DSP</span>
        </div>

        <!-- Feature 6 -->
        <div class="feature-card">
          <div class="feature-icon">🎹</div>
          <h3>Soundboard 8 Pad & Phím Tắt F1..F4</h3>
          <p>Bàn phím hiệu ứng âm thanh vỗ tay 👏, cười haha 😂, còi hơi 🎺 kèm phím tắt đổi Scene tức thì: F1 (Hát Live) ↔ F2 (Giao Lưu) ↔ F3 (AutoTune) ↔ F4/Tab chuyển tab nhanh.</p>
          <span class="feature-badge">Global Hotkeys & Instant Scene</span>
        </div>
      </div>
    </div>
  </section>

  <!-- Songbook Showcase -->
  <section class="songbook-showcase" id="songbook">
    <div class="container">
      <div class="songbook-box">
        <div style="display: flex; justify-content: space-between; align-items: flex-end; flex-wrap: wrap; gap: 20px; margin-bottom: 30px;">
          <div>
            <div class="section-tag" style="color:#38bdf8;">TÍCH HỢP SẴN TRONG DAW</div>
            <h2 style="font-family:var(--font-heading); font-size:2rem; font-weight:800;">Sổ Tone Bài Hát & Tone Nam / Nữ Thông Minh</h2>
            <p style="color:#94a3b8; font-size:0.95rem; margin-top:6px;">Không cần phải tra cứu hợp âm trên Google nữa. Chỉ cần chọn bài, bấm 1 nút là Auto-Tune tự nhảy đúng tone!</p>
          </div>
          <div style="font-family:var(--font-mono); font-size:0.9rem; background:rgba(56,189,248,0.1); border:1px solid #38bdf8; padding:8px 16px; border-radius:8px; color:#38bdf8;">
            ⭐ 1.033+ Bài Hát Tuyển Chọn
          </div>
        </div>

        <!-- Demo Song Items -->
        <div class="song-item-demo">
          <div class="song-title-wrap">
            <span style="color:#f43f5e; font-size:1.1rem;">❤️</span>
            <div>
              <div class="song-name">Ai Chung Tình Được Mãi</div>
              <div class="song-meta">Đinh Tùng Huy, Hoài Lâm • Pop Ballad</div>
            </div>
          </div>
          <div style="display:flex; gap:8px;">
            <span class="tone-tag tone-male">👨 Nam: Em</span>
            <span class="tone-tag tone-female">👩 Nữ: Am</span>
            <span class="tone-tag" style="background:#334155; color:#fff;">Gốc: Em</span>
          </div>
        </div>

        <div class="song-item-demo">
          <div class="song-title-wrap">
            <span style="color:#f43f5e; font-size:1.1rem;">❤️</span>
            <div>
              <div class="song-name">Chúng Ta Của Hiện Tại</div>
              <div class="song-meta">Sơn Tùng M-TP • Pop / 80s</div>
            </div>
          </div>
          <div style="display:flex; gap:8px;">
            <span class="tone-tag tone-male">👨 Nam: Eb</span>
            <span class="tone-tag tone-female">👩 Nữ: Ab</span>
            <span class="tone-tag" style="background:#334155; color:#fff;">Gốc: Eb</span>
          </div>
        </div>

        <div class="song-item-demo">
          <div class="song-title-wrap">
            <span style="color:#64748b; font-size:1.1rem;">🤍</span>
            <div>
              <div class="song-name">Gửi Người Chung Xóm</div>
              <div class="song-meta">Chế Linh, Đặng Vũ, Trường Vũ • Bolero / Nhạc Vàng</div>
            </div>
          </div>
          <div style="display:flex; gap:8px;">
            <span class="tone-tag tone-male">👨 Nam: Em</span>
            <span class="tone-tag tone-female">👩 Nữ: Am</span>
            <span class="tone-tag" style="background:#334155; color:#fff;">Gốc: Em</span>
          </div>
        </div>

        <div style="text-align:center; margin-top:24px; color:#94a3b8; font-size:0.9rem;">
          💡 Hỗ trợ: Lưu tone riêng cá nhân, tăng/giảm nửa cung Transpose (-1, +1), xuất/nhập file JSON để sao lưu.
        </div>
      </div>
    </div>
  </section>

  <!-- OBS Connection Guide -->
  <section class="guide-section" id="guide">
    <div class="container">
      <div class="section-header">
        <div class="section-tag">DỄ DÀNG KẾT NỐI</div>
        <h2 class="section-title">Kết Nối OBS Studio Trong 3 Bước</h2>
        <p class="section-desc">Không cần cài Virtual Audio Cable phức tạp. Bộ thu âm OBS Receiver tự động nhận luồng âm thanh sạch từ DAW với độ trễ 0ms.</p>
      </div>

      <div class="steps-grid">
        <div class="step-box">
          <div class="step-num">1</div>
          <h4>Cài Đặt & Mở DAW</h4>
          <p>Chạy bộ cài đặt <code>LiveStream_Micro_DAW_Setup.exe</code> để cài đặt DAW và tự động chép Plugin vào thư mục VST của OBS.</p>
        </div>

        <div class="step-box">
          <div class="step-num">2</div>
          <h4>Thêm Plugin Vào OBS</h4>
          <p>Trong OBS Studio, nhấp chuột phải vào nguồn Micro (Audio Mixer) ➔ <strong>Bộ lọc (Filters)</strong> ➔ Thêm <strong>Plugin VST 2.x</strong> ➔ Chọn <strong>LiveStream OBS Receiver</strong>.</p>
        </div>

        <div class="step-box">
          <div class="step-num">3</div>
          <h4>Tắt Giám Sát & Hát Live!</h4>
          <p>Trong Thuộc tính âm thanh nâng cao của OBS, chọn <strong>Tắt giám sát (Monitor Off)</strong> cho nguồn mic để tránh bị dội âm 2 lần. Bắt đầu buổi livestream đỉnh cao!</p>
        </div>
      </div>
    </div>
  </section>

  <!-- Download Section -->
  <section class="download-section" id="download">
    <div class="container">
      <div class="section-header">
        <div class="section-tag">MIỄN PHÍ 100%</div>
        <h2 class="section-title">Tải Về LiveStream Micro-DAW</h2>
        <p class="section-desc">Lựa chọn phiên bản phù hợp với nhu cầu sử dụng của bạn. Hỗ trợ Windows 10/11 64-bit.</p>
      </div>

      <div class="download-grid">
        <!-- Option 1: Setup -->
        <div class="download-card featured">
          <div class="dl-icon">📦</div>
          <h3 class="dl-title">Bản Cài Đặt (Setup)</h3>
          <p class="dl-desc">Bộ cài đặt tự động tạo icon Desktop và tự động cài đặt OBS Receiver VST Plugin vào đúng thư mục.</p>
          <a href="#" class="btn-dl-action primary" id="btn-download-setup">
            <span>Tải Bộ Cài Đặt (.exe)</span>
          </a>
          <div class="dl-meta">Phiên bản: v2.0 • Dung lượng: ~6.2 MB • Win 64-bit</div>
        </div>

        <!-- Option 2: Portable -->
        <div class="download-card">
          <div class="dl-icon">🚀</div>
          <h3 class="dl-title">Bản Portable (Zip)</h3>
          <p class="dl-desc">Dành cho người muốn chạy ngay từ USB / ổ cứng mà không cần cài đặt vào hệ thống Windows.</p>
          <a href="#" class="btn-dl-action outline" id="btn-download-portable">
            <span>Tải Bản Portable (.zip)</span>
          </a>
          <div class="dl-meta">Phiên bản: v2.0 • Dung lượng: ~6.0 MB • Giải nén dùng ngay</div>
        </div>
      </div>

      <div style="text-align:center; margin-top:30px; font-size:0.85rem; color:#64748b;">
        * Yêu cầu hệ thống: Windows 10/11 (64-bit), Soundcard hỗ trợ Driver ASIO (hoặc DirectSound / WASAPI).
      </div>
    </div>
  </section>

  <!-- Donate Section -->
  <section class="donate-section" id="donate">
    <div class="container">
      <div class="donate-container">
        <div class="donate-header">
          <div class="donate-badge">💖 ỦNG HỘ TÁC GIẢ / DONATE</div>
          <h2>Đồng Hành Cùng LiveStream Micro-DAW</h2>
          <p style="color:#94a3b8; font-size:0.95rem; margin-top:8px;">
            Sự ủng hộ tự nguyện của bạn là nguồn động lực to lớn giúp tôi tiếp tục duy trì, cập nhật tính năng mới và chia sẻ phần mềm miễn phí tới cộng đồng!
          </p>
        </div>

        <!-- Donate Method Tabs -->
        <div class="donate-tabs">
          <button class="tab-btn active" onclick="switchQr('vietqr')">🏦 VietQR (MB Bank)</button>
          <button class="tab-btn" onclick="switchQr('momo')">📱 Ví MoMo</button>
          <button class="tab-btn" onclick="switchQr('paypal')">🌐 PayPal</button>
          <button class="tab-btn" onclick="switchQr('mbcard')">💳 MB Bank Card</button>
        </div>

        <!-- Donate Box Content -->
        <div class="donate-card-content">
          <div class="qr-image-wrapper">
            <img id="qr-display" src="{vietqr_b64}" alt="Mã QR Donate" class="qr-image">
          </div>

          <div class="donate-details">
            <div class="info-row">
              <div>
                <div class="info-label">Chủ Tài Khoản</div>
                <div class="info-value" id="qr-name">LA CHÍ NHÂN</div>
              </div>
            </div>

            <div class="info-row">
              <div>
                <div class="info-label" id="qr-acc-label">Số Tài Khoản / SĐT</div>
                <div class="info-value" id="qr-acc-val">0908107000</div>
              </div>
              <button class="copy-btn" onclick="copyText('0908107000')">Sao Chép</button>
            </div>

            <div class="info-row">
              <div>
                <div class="info-label">Ngân Hàng / Ví</div>
                <div class="info-value" id="qr-bank" style="font-size:0.95rem;">MB Bank (Ngân hàng Quân Đội)</div>
              </div>
            </div>

            <div class="info-row">
              <div>
                <div class="info-label">Nội Dung Chuyển Khoản</div>
                <div class="info-value" style="font-size:0.95rem; color:#38bdf8;">Ung ho LiveStream Micro-DAW</div>
              </div>
              <button class="copy-btn" onclick="copyText('Ung ho LiveStream Micro-DAW')">Sao Chép</button>
            </div>

            <div id="copy-toast" class="toast-copy">✓ Đã sao chép vào bộ nhớ tạm!</div>
          </div>
        </div>

        <!-- Setup Service Box -->
        <div class="support-box">
          <strong>🛠️ DỊCH VỤ SETUP ÂM THANH & CÀI ĐẶT VOCAL CHAIN THEO YÊU CẦU:</strong><br>
          • Cài đặt trọn bộ Plugin VST3 chuyên nghiệp (Auto-Tune, FabFilter, Reverb, Delay) qua UltraViewer / TeamViewer.<br>
          • Tinh chỉnh âm thanh chuẩn phòng thu theo từng chất giọng và micro của bạn.<br>
          👉 <strong>Liên hệ Hotline / Zalo: 0908.107.000 (La Chí Nhân)</strong> • Website: <strong><a href="https://lachinhan.xyz" target="_blank" style="color:#38bdf8; text-decoration:none;">lachinhan.xyz</a></strong>
        </div>
      </div>
    </div>
  </section>

  <!-- Footer -->
  <footer class="site-footer">
    <div class="container">
      <div class="footer-logo">LiveStream Micro-DAW</div>
      <p>Phần mềm thu âm & hát livestream chuyên nghiệp phát triển bởi <strong>La Chí Nhân</strong></p>
      <ul class="footer-links">
        <li><a href="https://lachinhan.xyz" target="_blank">Trang Chủ lachinhan.xyz</a></li>
        <li><a href="#features">Tính Năng</a></li>
        <li><a href="#download">Tải Về</a></li>
        <li><a href="#donate">Donate</a></li>
        <li><a href="https://zalo.me/0908107000" target="_blank">Zalo: 0908.107.000</a></li>
      </ul>
      <p style="font-size:0.8rem; color:#475569; margin-top:20px;">
        © 2026 La Chí Nhân (lachinhan.xyz). All rights reserved.
      </p>
    </div>
  </footer>

  <!-- QR Switching & Copy Logic -->
  <script>
    const qrData = {{
      vietqr: {{
        src: "{vietqr_b64}",
        name: "LA CHÍ NHÂN",
        accLabel: "Số Tài Khoản",
        accVal: "0908107000",
        bank: "MB Bank (Ngân hàng Quân Đội)"
      }},
      momo: {{
        src: "{momo_b64}",
        name: "LA CHÍ NHÂN",
        accLabel: "Số Điện Thoại MoMo",
        accVal: "0908107000",
        bank: "Ví Điện Tử MoMo"
      }},
      paypal: {{
        src: "{paypal_b64}",
        name: "Nhan La Chi",
        accLabel: "Tài Khoản PayPal",
        accVal: "Nhan La Chi",
        bank: "Cổng Thanh Toán Quốc Tế PayPal"
      }},
      mbcard: {{
        src: "{mb_b64}",
        name: "LA CHÍ NHÂN",
        accLabel: "Số Tài Khoản MB",
        accVal: "0908107000",
        bank: "MB Bank (Thẻ Đa Năng)"
      }}
    }};

    function switchQr(type) {{
      const data = qrData[type];
      if (!data) return;

      document.getElementById('qr-display').src = data.src;
      document.getElementById('qr-name').innerText = data.name;
      document.getElementById('qr-acc-label').innerText = data.accLabel;
      document.getElementById('qr-acc-val').innerText = data.accVal;
      document.getElementById('qr-bank').innerText = data.bank;

      // Update button active state
      const buttons = document.querySelectorAll('.donate-tabs .tab-btn');
      buttons.forEach(btn => btn.classList.remove('active'));
      event.target.classList.add('active');
    }}

    function copyText(text) {{
      navigator.clipboard.writeText(text).then(() => {{
        const toast = document.getElementById('copy-toast');
        toast.innerText = '✓ Đã sao chép: ' + text;
        toast.style.display = 'block';
        setTimeout(() => {{
          toast.style.display = 'none';
        }}, 3000);
      }}).catch(err => {{
        console.error('Lỗi sao chép:', err);
      }});
    }}
  </script>
</body>
</html>
"""

output_path = os.path.join(workspace, "landing_page.html")
with open(output_path, "w", encoding="utf-8") as f:
    f.write(html_content)

print(f"Generated standalone landing page: {output_path} ({len(html_content)} bytes)")
