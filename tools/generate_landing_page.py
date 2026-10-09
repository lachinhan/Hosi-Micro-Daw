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

# Links & Metadata
zalo_trial_group_url = "https://zalo.me/g/uvkmqjnxp5ncos07ov0y"
zalo_personal_url = "https://zalo.me/0908107000"
github_repo_url = "https://github.com/lachinhan/Hosi-Micro-Daw"
github_setup_zip_url = "https://github.com/lachinhan/Hosi-Micro-Daw/raw/main/Hosi%20Micro%20Daw%20Setup.zip"
github_setup_exe_url = "https://github.com/lachinhan/Hosi-Micro-Daw/raw/main/LiveStream_Micro_DAW_Setup.exe"
github_portable_zip_url = "https://github.com/lachinhan/Hosi-Micro-Daw/raw/main/Hosi%20Micro%20Daw%20Portable.zip"
github_portable_rar_url = "https://github.com/lachinhan/Hosi-Micro-Daw/raw/main/Hosi%20Micro%20Daw%20Portable.rar"
github_macos_url = "https://github.com/lachinhan/Hosi-Micro-Daw/raw/main/LiveStream_Micro_DAW_macOS.zip"

landing_html = f"""<!DOCTYPE html>
<html lang="vi">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>LiveStream Micro-DAW v2.0.5 - Phần Mềm Hát Live & Thu Âm Tự Động Dò Tone Chuyên Nghiệp | lachinhan.xyz</title>
  <meta name="description" content="LiveStream Micro-DAW là phần mềm âm thanh chuyên nghiệp dành riêng cho Streamer và Ca sĩ hát Live: Tự động dò Tone nhạc Karaoke, 1-Click đồng bộ Auto-Tune, truyền âm thanh sang OBS Studio độ trễ 0ms.">
  <meta name="keywords" content="LiveStream Micro-DAW, Hát Live Auto-Tune, Dò tone karaoke, lachinhan.xyz, phần mềm livestream hát karaoke, VST3 Host, OBS Studio Receiver">
  <meta name="author" content="La Chí Nhân (lachinhan.xyz)">
  
  <!-- Open Graph -->
  <meta property="og:title" content="LiveStream Micro-DAW v2.0.5 - Phòng Thu Hát Live & Dò Tone Chuyên Nghiệp">
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
      --accent-red: #ff1919;
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
      backdrop-filter: blur(16px);
      background: rgba(7, 10, 19, 0.85);
      border-bottom: 1px solid rgba(255, 255, 255, 0.08);
      z-index: 100;
      padding: 16px 0;
    }}
    .nav-wrapper {{
      display: flex;
      align-items: center;
      justify-content: space-between;
    }}
    .brand-logo {{
      display: flex;
      align-items: center;
      gap: 12px;
      text-decoration: none;
      color: var(--text-main);
    }}
    .brand-logo img {{
      width: 36px;
      height: 36px;
      border-radius: 8px;
    }}
    .brand-logo span {{
      font-family: var(--font-heading);
      font-size: 20px;
      font-weight: 800;
      letter-spacing: -0.5px;
      background: linear-gradient(135deg, #ffffff 0%, #38bdf8 100%);
      -webkit-background-clip: text;
      -webkit-text-fill-color: transparent;
    }}
    .nav-links {{
      display: flex;
      gap: 24px;
      align-items: center;
    }}
    .nav-links a {{
      color: var(--text-muted);
      text-decoration: none;
      font-size: 14.5px;
      font-weight: 600;
      transition: color 0.2s ease;
    }}
    .nav-links a:hover {{
      color: var(--accent-blue);
    }}
    .nav-btn-pro {{
      background: linear-gradient(135deg, #ff1919 0%, #dc2626 100%);
      color: #ffffff !important;
      padding: 8px 18px;
      border-radius: 8px;
      font-size: 13.5px;
      font-weight: 700;
      text-decoration: none;
      box-shadow: 0 4px 14px rgba(255, 25, 25, 0.35);
      transition: transform 0.2s, box-shadow 0.2s;
    }}
    .nav-btn-pro:hover {{
      transform: translateY(-2px);
      box-shadow: 0 6px 20px rgba(255, 25, 25, 0.5);
    }}

    /* Hero Section */
    section.hero {{
      padding: 80px 0 60px 0;
      text-align: center;
    }}
    .hero-badge {{
      display: inline-flex;
      align-items: center;
      gap: 8px;
      background: rgba(56, 189, 248, 0.12);
      border: 1px solid rgba(56, 189, 248, 0.3);
      color: var(--accent-blue);
      font-size: 13px;
      font-weight: 700;
      letter-spacing: 0.5px;
      padding: 6px 16px;
      border-radius: 20px;
      margin-bottom: 24px;
      text-transform: uppercase;
    }}
    .hero-title {{
      font-family: var(--font-heading);
      font-size: 52px;
      font-weight: 900;
      line-height: 1.15;
      letter-spacing: -1px;
      margin-bottom: 20px;
      max-width: 900px;
      margin-left: auto;
      margin-right: auto;
    }}
    .hero-title span.gradient-red {{
      background: linear-gradient(135deg, #ff4b4b 0%, #ff1919 100%);
      -webkit-background-clip: text;
      -webkit-text-fill-color: transparent;
    }}
    .hero-subtitle {{
      font-size: 18px;
      color: var(--text-muted);
      max-width: 780px;
      margin: 0 auto 36px auto;
      line-height: 1.7;
    }}
    .hero-cta-group {{
      display: flex;
      gap: 16px;
      justify-content: center;
      flex-wrap: wrap;
    }}
    .btn-primary {{
      background: linear-gradient(135deg, #ff1919 0%, #dc2626 100%);
      color: #ffffff;
      padding: 14px 28px;
      border-radius: 10px;
      font-weight: 700;
      font-size: 15px;
      text-decoration: none;
      display: inline-flex;
      align-items: center;
      gap: 10px;
      box-shadow: 0 4px 20px rgba(255, 25, 25, 0.4);
      transition: all 0.2s ease;
    }}
    .btn-primary:hover {{
      transform: translateY(-2px);
      box-shadow: 0 8px 30px rgba(255, 25, 25, 0.6);
    }}
    .btn-secondary {{
      background: rgba(30, 41, 59, 0.8);
      color: #ffffff;
      border: 1px solid rgba(255, 255, 255, 0.15);
      padding: 14px 28px;
      border-radius: 10px;
      font-weight: 600;
      font-size: 15px;
      text-decoration: none;
      display: inline-flex;
      align-items: center;
      gap: 10px;
      transition: all 0.2s ease;
    }}
    .btn-secondary:hover {{
      background: rgba(51, 65, 85, 0.9);
      border-color: rgba(255, 255, 255, 0.3);
      transform: translateY(-2px);
    }}

    /* Section Headers */
    .section-header {{
      text-align: center;
      margin-bottom: 48px;
    }}
    .section-tag {{
      color: var(--accent-blue);
      font-size: 13px;
      font-weight: 800;
      letter-spacing: 1px;
      text-transform: uppercase;
      margin-bottom: 8px;
      display: block;
    }}
    .section-title {{
      font-family: var(--font-heading);
      font-size: 36px;
      font-weight: 800;
      letter-spacing: -0.5px;
    }}

    /* 8 Features Grid */
    .features-grid {{
      display: grid;
      grid-template-columns: repeat(auto-fit, minmax(320px, 1fr));
      gap: 24px;
      margin-bottom: 80px;
    }}
    .feature-card {{
      background: var(--bg-card);
      border: 1px solid var(--border-color);
      border-radius: 16px;
      padding: 30px 26px;
      transition: all 0.3s ease;
      backdrop-filter: blur(12px);
    }}
    .feature-card:hover {{
      background: var(--bg-card-hover);
      border-color: var(--border-glow);
      transform: translateY(-4px);
      box-shadow: 0 12px 30px rgba(0, 0, 0, 0.3);
    }}
    .feature-icon {{
      font-size: 36px;
      margin-bottom: 16px;
      display: inline-block;
    }}
    .feature-title {{
      font-family: var(--font-heading);
      font-size: 20px;
      font-weight: 700;
      margin-bottom: 10px;
      color: #ffffff;
    }}
    .feature-desc {{
      color: var(--text-muted);
      font-size: 14.5px;
      line-height: 1.6;
    }}

    /* Songbook Showcase Box */
    .songbook-box {{
      background: linear-gradient(135deg, rgba(15, 23, 42, 0.9) 0%, rgba(30, 41, 59, 0.8) 100%);
      border: 1px solid rgba(56, 189, 248, 0.3);
      border-radius: 20px;
      padding: 36px;
      margin-bottom: 80px;
      box-shadow: 0 10px 40px rgba(0,0,0,0.3);
    }}
    .song-item {{
      background: rgba(15, 23, 42, 0.6);
      border: 1px solid rgba(255, 255, 255, 0.06);
      border-radius: 12px;
      padding: 14px 20px;
      display: flex;
      align-items: center;
      justify-content: space-between;
      margin-bottom: 12px;
    }}
    .tone-badge {{
      padding: 5px 12px;
      border-radius: 6px;
      font-family: var(--font-mono);
      font-size: 12.5px;
      font-weight: 700;
    }}

    /* Pricing Section */
    section.pricing {{
      padding: 60px 0 80px 0;
    }}
    .pricing-grid {{
      display: grid;
      grid-template-columns: repeat(auto-fit, minmax(250px, 1fr));
      gap: 20px;
    }}
    .pricing-card {{
      background: var(--bg-card);
      border: 1px solid var(--border-color);
      border-radius: 18px;
      padding: 32px 24px;
      text-align: center;
      position: relative;
      display: flex;
      flex-direction: column;
      justify-content: space-between;
      transition: all 0.3s ease;
    }}
    .pricing-card.featured {{
      border: 2px solid #f59e0b;
      background: linear-gradient(180deg, rgba(245, 158, 11, 0.1) 0%, rgba(15, 23, 42, 0.8) 100%);
      box-shadow: 0 10px 40px rgba(245, 158, 11, 0.2);
      transform: scale(1.03);
    }}
    .pricing-badge {{
      position: absolute;
      top: -14px;
      left: 50%;
      transform: translateX(-50%);
      background: #f59e0b;
      color: #0f172a;
      font-size: 11px;
      font-weight: 800;
      padding: 4px 14px;
      border-radius: 12px;
      letter-spacing: 0.5px;
      text-transform: uppercase;
    }}
    .plan-name {{
      font-family: var(--font-heading);
      font-size: 22px;
      font-weight: 700;
      margin-bottom: 10px;
    }}
    .plan-price {{
      font-family: var(--font-heading);
      font-size: 40px;
      font-weight: 900;
      color: var(--accent-blue);
      line-height: 1;
      margin: 12px 0 6px 0;
    }}
    .pricing-card.featured .plan-price {{
      color: #f59e0b;
    }}
    .plan-duration {{
      font-size: 13px;
      color: var(--text-muted);
      margin-bottom: 24px;
    }}
    .plan-features {{
      list-style: none !important;
      text-align: left;
      font-size: 14px;
      color: #cbd5e1;
      line-height: 1.8;
      margin-bottom: 28px;
      padding: 0 !important;
    }}
    .plan-features li {{
      display: flex;
      align-items: flex-start;
      gap: 10px;
      margin-bottom: 6px;
    }}
    .plan-features li span.plan-chk {{
      color: var(--accent-emerald);
      font-weight: 800;
      flex-shrink: 0;
    }}

    .btn-plan {{
      width: 100%;
      padding: 12px 20px;
      border-radius: 8px;
      font-weight: 700;
      font-size: 14px;
      text-decoration: none;
      display: block;
      text-align: center;
      transition: all 0.2s ease;
    }}

    /* Affiliate Box */
    .affiliate-box {{
      background: linear-gradient(135deg, rgba(15, 23, 42, 0.95) 0%, rgba(2, 6, 23, 0.95) 100%);
      border: 1px solid rgba(245, 158, 11, 0.3);
      border-radius: 20px;
      padding: 40px 32px;
      margin: 60px 0;
      box-shadow: 0 10px 40px rgba(0,0,0,0.4);
    }}
    .affiliate-table {{
      width: 100%;
      border-collapse: collapse;
      margin: 24px 0;
      font-size: 14.5px;
    }}
    .affiliate-table th, .affiliate-table td {{
      padding: 14px 16px;
      border: 1px solid rgba(255, 255, 255, 0.1);
      text-align: left;
    }}
    .affiliate-table th {{
      background: rgba(30, 41, 59, 0.8);
      color: var(--accent-blue);
      font-family: var(--font-heading);
      font-size: 15px;
    }}
    .steps-grid {{
      display: grid;
      grid-template-columns: repeat(auto-fit, minmax(230px, 1fr));
      gap: 16px;
      margin: 24px 0;
    }}
    .step-card {{
      background: rgba(30, 41, 59, 0.6);
      border-radius: 12px;
      padding: 20px;
      border-left: 4px solid var(--accent-blue);
    }}
    .step-badge {{
      font-family: var(--font-mono);
      font-size: 12px;
      color: var(--accent-blue);
      font-weight: 700;
      margin-bottom: 6px;
    }}
    .step-title {{
      font-size: 15px;
      font-weight: 700;
      color: #ffffff;
      margin-bottom: 8px;
    }}
    .step-desc {{
      font-size: 13px;
      color: var(--text-muted);
      line-height: 1.5;
    }}

    /* Download Cards */
    .dl-grid {{
      display: grid;
      grid-template-columns: repeat(auto-fit, minmax(280px, 1fr));
      gap: 24px;
      margin-bottom: 60px;
    }}
    .dl-card {{
      background: var(--bg-card);
      border: 1px solid var(--border-color);
      border-radius: 16px;
      padding: 28px;
      text-align: center;
    }}

    /* QR Grid */
    .qr-grid {{
      display: grid;
      grid-template-columns: repeat(auto-fit, minmax(220px, 1fr));
      gap: 20px;
      margin-top: 28px;
    }}
    .qr-card {{
      background: rgba(15, 23, 42, 0.7);
      border: 1px solid var(--border-color);
      border-radius: 14px;
      padding: 20px;
      text-align: center;
    }}
    .qr-img {{
      width: 160px;
      height: 160px;
      object-fit: contain;
      border-radius: 8px;
      margin-bottom: 12px;
      background: #ffffff;
      padding: 6px;
    }}

    /* Footer */
    footer.site-footer {{
      border-top: 1px solid rgba(255, 255, 255, 0.08);
      padding: 40px 0;
      text-align: center;
      color: var(--text-muted);
      font-size: 14px;
    }}
    footer.site-footer a {{
      color: var(--accent-blue);
      text-decoration: none;
    }}

    @media (max-width: 768px) {{
      .hero-title {{ font-size: 36px; }}
      .hero-subtitle {{ font-size: 16px; }}
      .nav-links {{ display: none; }}
    }}
  </style>
</head>
<body>

  <!-- Background Ambient Glows -->
  <div class="ambient-glow glow-1"></div>
  <div class="ambient-glow glow-2"></div>
  <div class="ambient-glow glow-3"></div>

  <!-- Header -->
  <header class="site-header">
    <div class="container nav-wrapper">
      <a href="https://www.lachinhan.xyz" class="brand-logo">
        <img src="{icon_b64}" alt="LiveStream Micro-DAW Logo">
        <span>LiveStream Micro-DAW</span>
      </a>
      <nav class="nav-links">
        <a href="#features">Tính Năng</a>
        <a href="#songbook">Sổ Tone</a>
        <a href="#pricing">Bảng Giá</a>
        <a href="#affiliate">Cộng Tác Viên</a>
        <a href="#download">Tải Về</a>
        <a href="https://byvn.net/hosiguide" target="_blank">AI Guide</a>
        <a href="{zalo_personal_url}" target="_blank" class="nav-btn-pro">💬 LIÊN HỆ MUA KEY PRO</a>
      </nav>
    </div>
  </header>

  <!-- Hero Section -->
  <section class="hero">
    <div class="container">
      <div class="hero-badge">⚡ LIVESTREAM MICRO-DAW v2.0.5 • ZERO-LATENCY IPC</div>
      <h1 class="hero-title">
        Phần Mềm Hát Live & Thu Âm<br>
        <span class="gradient-red">Tự Động Dò Tone Số 1 Cho Streamer</span>
      </h1>
      <p class="hero-subtitle">
        Giải pháp trạm âm thanh số (DAW) siêu nhẹ do <strong>Hosi Prod (La Chí Nhân)</strong> phát triển. Tích hợp sẵn <strong>Mini YouTube Karaoke Player chặn quảng cáo</strong>, <strong>Lá chắn AI Noise & De-Reverb</strong>, <strong>Dò Tone Beat & Micro</strong> truyền thẳng Auto-Tune và truyền âm sang OBS Studio độ trễ <strong>0ms</strong>.
      </p>

      <div class="hero-cta-group">
        <a href="#download" class="btn-primary">📥 TẢI PHẦN MỀM MIỄN PHÍ</a>
        <a href="{zalo_personal_url}" target="_blank" class="btn-secondary">💬 LIÊN HỆ MUA KEY PRO</a>
        <a href="https://byvn.net/hosiguide" target="_blank" class="btn-secondary" style="color:var(--accent-blue);">🤖 TRỢ LÝ AI HƯỚNG DẪN ↗</a>
      </div>
    </div>
  </section>

  <!-- Features Grid -->
  <section class="features" id="features">
    <div class="container">
      <div class="section-header">
        <span class="section-tag">CÔNG NGHỆ ÂM THANH ĐỘT PHÁ</span>
        <h2 class="section-title">9 Trụ Cột Tính Năng Đỉnh Cao</h2>
      </div>

      <div class="features-grid">
        <div class="feature-card">
          <div class="feature-icon">📺</div>
          <h3 class="feature-title">Mini YouTube Karaoke Player</h3>
          <p class="feature-desc">Trình phát Beat YouTube siêu nhẹ (~40MB RAM - WebView2 / WebKit), tự động bỏ qua 100% quảng cáo trong 0.1s, tăng/giảm Tone Beat trực tiếp và ghim nổi Always-On-Top khi live.</p>
        </div>

        <div class="feature-card">
          <div class="feature-icon">🛡️</div>
          <h3 class="feature-title">AI Real-Time Noise &amp; De-Reverb</h3>
          <p class="feature-desc">Mạng nơ-ron AI (DeepFilter GRU) thời gian thực, khử sạch tiếng quạt gió, ve kêu, tiếng còi xe và triệt tiêu dội âm phòng ngủ/hội trường tức thì mà không làm méo giọng.</p>
        </div>

        <div class="feature-card">
          <div class="feature-icon">🎙️</div>
          <h3 class="feature-title">AI Vocal Profiler &amp; Auto EQ</h3>
          <p class="feature-desc">Đo mẫu giọng 5 giây, tự động phân tích Formant và sinh đường cong 7-Band EQ lý tưởng (Studio Master, Bolero, Pop Remix, Talkshow) giúp giọng sáng và ấm.</p>
        </div>

        <div class="feature-card">
          <div class="feature-icon">🎵</div>
          <h3 class="feature-title">Tự Động Dò Tone Beat &amp; Micro</h3>
          <p class="feature-desc">Thuật toán Chromagram nhận diện chính xác nốt Root &amp; Scale (Trưởng/Thứ) của beat karaoke. 1-Click gán chuẩn xác vào Auto-Tune Pro trong Rack.</p>
        </div>

        <div class="feature-card">
          <div class="feature-icon">📖</div>
          <h3 class="feature-title">Sổ Tone 1.033+ Bài Hát &amp; AI Match</h3>
          <p class="feature-desc">Kho dữ liệu 1.033+ bài hát Việt Nam kèm đầy đủ Tone Nam, Tone Nữ, Tone Gốc. Tự động quét âm vực giọng (C2-C6) để gợi ý bài hát vừa vặn nhất (Fit Score 100%).</p>
        </div>

        <div class="feature-card">
          <div class="feature-icon">⏱️</div>
          <h3 class="feature-title">Smart BPM-Synced Reverb &amp; Delay</h3>
          <p class="feature-desc">Tự động nhận diện Tempo (BPM) bài hát, đồng bộ bước nhại Delay nảy tanh tách theo phách và khép đuôi Reverb mượt mà cuối ô nhịp.</p>
        </div>

        <div class="feature-card">
          <div class="feature-icon">⚡</div>
          <h3 class="feature-title">Zero-Latency Sang OBS Studio (0ms)</h3>
          <p class="feature-desc">Truyền âm thanh trực tiếp sang OBS qua Windows Named Shared Memory (0ms delay). Không cần cài đặt Virtual Audio Cable hay Voicemeeter!</p>
        </div>

        <div class="feature-card">
          <div class="feature-icon">🎛️</div>
          <h3 class="feature-title">Rack 8 Slot VST3 &amp; Vocal DSP</h3>
          <p class="feature-desc">Hỗ trợ mọi plugin VST3 chuyên nghiệp (Auto-Tune, FabFilter, Waves...) kèm bộ 6 hiệu ứng giọng hát Studio tích hợp sẵn (Gate, EQ, Comp, Reverb, Echo, Limiter).</p>
        </div>

        <div class="feature-card">
          <div class="feature-icon">☁️</div>
          <h3 class="feature-title">Cloud Songbook &amp; Artist Preset Shop</h3>
          <p class="feature-desc">Tự động đồng bộ các bài hát Hot Trend TikTok/YouTube mới nhất vào Sổ Tone mỗi tuần qua Cloud. Kèm kho Preset phong cách ca sĩ/streamer nổi tiếng nạp tức thì chỉ bằng 1-Click.</p>
        </div>
      </div>
    </div>
  </section>

  <!-- Songbook Showcase -->
  <section class="songbook" id="songbook">
    <div class="container">
      <div class="songbook-box">
        <div style="display: flex; justify-content: space-between; align-items: center; margin-bottom: 24px; flex-wrap: wrap; gap: 12px;">
          <div>
            <span class="section-tag">TÍCH HỢP SẴN TRONG DAW</span>
            <h3 style="font-family: var(--font-heading); font-size: 26px;">Sổ Tone 1.033+ Bài Hát Việt Nam</h3>
          </div>
          <span style="background: rgba(255, 25, 25, 0.15); border: 1px solid rgba(255, 25, 25, 0.4); color: #ff4b4b; padding: 6px 14px; border-radius: 8px; font-family: var(--font-mono); font-size: 13px; font-weight: 700;">
            ⭐ TONE NAM • NỮ • GỐC
          </span>
        </div>

        <div class="song-item">
          <div>
            <strong style="font-size: 16px; color: #ffffff;">Ai Chung Tình Được Mãi</strong>
            <span style="color: var(--text-muted); font-size: 13.5px; margin-left: 12px;">Đinh Tùng Huy, Hoài Lâm • Pop Ballad</span>
          </div>
          <div style="display: flex; gap: 8px;">
            <span class="tone-badge" style="background: rgba(56, 189, 248, 0.15); color: #38bdf8; border: 1px solid rgba(56, 189, 248, 0.3);">👨 Nam: Em</span>
            <span class="tone-badge" style="background: rgba(244, 63, 94, 0.15); color: #f43f5e; border: 1px solid rgba(244, 63, 94, 0.3);">👩 Nữ: Am</span>
            <span class="tone-badge" style="background: rgba(255, 255, 255, 0.08); color: #e2e8f0;">Gốc: Em</span>
          </div>
        </div>

        <div class="song-item">
          <div>
            <strong style="font-size: 16px; color: #ffffff;">Chúng Ta Của Hiện Tại</strong>
            <span style="color: var(--text-muted); font-size: 13.5px; margin-left: 12px;">Sơn Tùng M-TP • Pop / 80s</span>
          </div>
          <div style="display: flex; gap: 8px;">
            <span class="tone-badge" style="background: rgba(56, 189, 248, 0.15); color: #38bdf8; border: 1px solid rgba(56, 189, 248, 0.3);">👨 Nam: Eb</span>
            <span class="tone-badge" style="background: rgba(244, 63, 94, 0.15); color: #f43f5e; border: 1px solid rgba(244, 63, 94, 0.3);">👩 Nữ: Ab</span>
            <span class="tone-badge" style="background: rgba(255, 255, 255, 0.08); color: #e2e8f0;">Gốc: Eb</span>
          </div>
        </div>
      </div>
    </div>
  </section>

  <!-- Pricing Section -->
  <section class="pricing" id="pricing">
    <div class="container">
      <div class="section-header">
        <span class="section-tag">BẢNG GIÁ &amp; GÓI BẢN QUYỀN</span>
        <h2 class="section-title">Lựa Chọn Gói Bản Quyền Phù Hợp</h2>
      </div>

      <div class="pricing-grid">
        <!-- 7 Days -->
        <div class="pricing-card">
          <div>
            <div class="plan-name">🎁 Dùng Thử 7 Ngày</div>
            <div class="plan-price">0đ</div>
            <div class="plan-duration">Full tính năng PRO VIP</div>
            <ul class="plan-features">
              <li><span class="plan-chk">✓</span><span>Trải nghiệm 100% tính năng PRO</span></li>
              <li><span class="plan-chk">✓</span><span>YouTube Player chặn quảng cáo</span></li>
              <li><span class="plan-chk">✓</span><span>Lá chắn AI Denoise &amp; De-Reverb</span></li>
              <li><span class="plan-chk">✓</span><span>Đồng bộ BPM Reverb/Delay</span></li>
              <li><span class="plan-chk">✓</span><span>Hỗ trợ qua Zalo cộng đồng</span></li>
            </ul>
          </div>
          <a href="{zalo_trial_group_url}" target="_blank" class="btn-plan" style="background:#334155; color:#ffffff;">NHẬN KEY DÙNG THỬ</a>
        </div>

        <!-- 1 Month -->
        <div class="pricing-card">
          <div>
            <div class="plan-name">📅 Gói 1 Tháng</div>
            <div class="plan-price">99.000đ</div>
            <div class="plan-duration">Thời hạn 30 ngày sử dụng</div>
            <ul class="plan-features">
              <li><span class="plan-chk">✓</span><span>Full tính năng PRO VIP v2.0.5</span></li>
              <li><span class="plan-chk">✓</span><span>YouTube Karaoke chặn quảng cáo</span></li>
              <li><span class="plan-chk">✓</span><span>AI Denoise &amp; Vocal Profiler</span></li>
              <li><span class="plan-chk">✓</span><span>Dò tone &amp; đồng bộ Auto-Tune</span></li>
              <li><span class="plan-chk">✓</span><span>Phù hợp biểu diễn ngắn hạn</span></li>
            </ul>
          </div>
          <a href="{zalo_personal_url}" target="_blank" class="btn-plan" style="background:#0284c7; color:#ffffff;">MUA GÓI 1 THÁNG</a>
        </div>

        <!-- 1 Year -->
        <div class="pricing-card">
          <div>
            <div class="plan-name">⏱️ Gói 1 Năm</div>
            <div class="plan-price">590.000đ</div>
            <div class="plan-duration">Thời hạn 365 ngày (Tiết kiệm ~50%)</div>
            <ul class="plan-features">
              <li><span class="plan-chk">✓</span><span>Full tính năng PRO VIP</span></li>
              <li><span class="plan-chk">✓</span><span>Cập nhật tính năng mới liên tục</span></li>
              <li><span class="plan-chk">✓</span><span>Hỗ trợ kỹ thuật 1-1 UltraView</span></li>
              <li><span class="plan-chk">✓</span><span>Đồng bộ Sổ Tone Cloud VIP</span></li>
              <li><span class="plan-chk">✓</span><span>Chuyển đổi máy miễn phí</span></li>
            </ul>
          </div>
          <a href="{zalo_personal_url}" target="_blank" class="btn-plan" style="background:#059669; color:#ffffff;">MUA GÓI 1 NĂM</a>
        </div>

        <!-- Lifetime -->
        <div class="pricing-card featured">
          <div class="pricing-badge">🔥 PHỔ BIẾN NHẤT</div>
          <div>
            <div class="plan-name">👑 PRO Trọn Đời</div>
            <div class="plan-price">990.000đ</div>
            <div class="plan-duration">Sở hữu Vĩnh Viễn • Nâng cấp trọn đời</div>
            <ul class="plan-features">
              <li><span class="plan-chk">✓</span><span>Mở khóa VĨNH VIỄN toàn bộ tính năng</span></li>
              <li><span class="plan-chk">✓</span><span>Mọi bản cập nhật trong tương lai</span></li>
              <li><span class="plan-chk">✓</span><span>Hỗ trợ setup phòng thu live từ xa</span></li>
              <li><span class="plan-chk">✓</span><span>Cấp lại Key trọn đời khi đổi máy</span></li>
              <li><span class="plan-chk">✓</span><span>Ưu tiên hỗ trợ kỹ thuật VIP 24/7</span></li>
            </ul>
          </div>
          <a href="{zalo_personal_url}" target="_blank" class="btn-plan" style="background:#f59e0b; color:#0f172a; font-weight:800;">SỞ HỮU TRỌN ĐỜI</a>
        </div>
      </div>
    </div>
  </section>

  <!-- Affiliate Section -->
  <section class="affiliate" id="affiliate">
    <div class="container">
      <div class="affiliate-box">
        <div class="section-header" style="text-align: left; margin-bottom: 24px;">
          <span class="section-tag">CHƯƠNG TRÌNH ĐỐI TÁC</span>
          <h2 class="section-title">Chính Sách Cộng Tác Viên &amp; Đại Lý (Affiliate Partner)</h2>
        </div>
        <p style="color: #94a3b8; font-size: 15px; margin-bottom: 24px;">
          Dành riêng cho <strong>Reviewer Âm Thanh</strong>, <strong>Thợ Cài Đặt Project</strong> và <strong>Admin Cộng Đồng Livestream</strong> với chính sách hoa hồng cao nhất thị trường:
        </p>

        <h3 style="color:#38bdf8; font-family:var(--font-heading); font-size:18px; margin-bottom:14px;">1. BẢNG HOA HỒNG CHIẾT KHẤU THEO DOANH SỐ (KEY TRỌN ĐỜI 990K)</h3>
        <div style="overflow-x: auto;">
          <table class="affiliate-table">
            <thead>
              <tr>
                <th>Doanh Số Tháng</th>
                <th>Cấp Bậc</th>
                <th>Mức Hoa Hồng</th>
                <th>Thưởng KPI Cuối Tháng</th>
                <th>Thu Nhập Thực Tế / Key</th>
              </tr>
            </thead>
            <tbody>
              <tr>
                <td><strong>Dưới 10 Key</strong></td>
                <td>Cộng Tác Viên</td>
                <td><strong style="color:#4ade80;">30%</strong></td>
                <td>Thu tiền &amp; Giữ lại 300k/Key ngay trong ngày</td>
                <td><strong>300.000 VNĐ / Key</strong></td>
              </tr>
              <tr>
                <td><strong>Từ 10 – 29 Key</strong></td>
                <td>Đối Tác Tiềm Năng</td>
                <td><strong style="color:#facc15;">35%</strong></td>
                <td>Thưởng thêm <strong>+5% (50.000đ/Key)</strong> cuối tháng</td>
                <td><strong>350.000 VNĐ / Key</strong></td>
              </tr>
              <tr>
                <td><strong>Từ 30 Key trở lên</strong></td>
                <td>Đại Lý VIP</td>
                <td><strong style="color:#f87171;">40%</strong></td>
                <td>Thưởng thêm <strong>+10% (100.000đ/Key)</strong> cuối tháng</td>
                <td><strong>400.000 VNĐ / Key</strong></td>
              </tr>
            </tbody>
          </table>
        </div>

        <h3 style="color:#38bdf8; font-family:var(--font-heading); font-size:18px; margin-top:32px; margin-bottom:14px;">2. QUY TRÌNH PHỄU "TẶNG KEY 7 NGÀY" ➔ CHỐT ĐƠN TỰ ĐỘNG (HOW TO WORK)</h3>
        <div class="steps-grid">
          <div class="step-card">
            <div class="step-badge">BƯỚC 1</div>
            <div class="step-title">Tặng Key 7 Ngày</div>
            <p class="step-desc">Khách tải app, gửi Machine ID ➔ Bạn yêu cầu cấp Key Trial 7 Ngày miễn phí qua nhóm Zalo.</p>
          </div>
          <div class="step-card">
            <div class="step-badge">BƯỚC 2</div>
            <div class="step-title">Trải Nghiệm Đỉnh Cao</div>
            <p class="step-desc">Khách hàng hát live với âm thanh AI và YouTube chặn quảng cáo cực thích.</p>
          </div>
          <div class="step-card">
            <div class="step-badge">BƯỚC 3</div>
            <div class="step-title">Chốt Đơn Tự Động</div>
            <p class="step-desc">Hết 7 ngày, app quay về Free ➔ Khách hụt hẫng và chủ động mua gói PRO ngay.</p>
          </div>
          <div class="step-card">
            <div class="step-badge">BƯỚC 4</div>
            <div class="step-title">Nhận Hoa Hồng Ngay</div>
            <p class="step-desc">Khách thanh toán 990k ➔ Bạn giữ lại 300k - 400k hoa hồng vào tài khoản.</p>
          </div>
        </div>

        <h3 style="color:#38bdf8; font-family:var(--font-heading); font-size:18px; margin-top:32px; margin-bottom:14px;">3. BA MÔ HÌNH HỢP TÁC LINH HOẠT</h3>
        <div class="steps-grid">
          <div class="step-card">
            <div class="step-badge">MÔ HÌNH 1</div>
            <div class="step-title">Reviewer Thu Tiền Trực Tiếp</div>
            <p class="step-desc">Gắn link Zalo/Fanpage dưới video. Khách chuyển 990k ➔ Giữ lại 300k hoa hồng ngay trong ngày.</p>
          </div>
          <div class="step-card">
            <div class="step-badge">MÔ HÌNH 2</div>
            <div class="step-title">Bán Sỉ Key Cho Kỹ Thuật Viên</div>
            <p class="step-desc">Mua sỉ gói 5 - 20 Key với giá ưu đãi (450k - 600k/Key) để tự cài đặt và thu tiền công của khách.</p>
          </div>
          <div class="step-card">
            <div class="step-badge">MÔ HÌNH 3</div>
            <div class="step-title">Sub-Keygen Cho Đại Lý Lớn</div>
            <p class="step-desc">Cấp tool sinh key riêng với số lượng tín dụng nạp trước, tự động xuất key cho khách hàng 24/7.</p>
          </div>
        </div>

        <div style="text-align: center; margin-top: 32px;">
          <a href="{zalo_personal_url}" target="_blank" class="btn-primary" style="background:#f59e0b; color:#0f172a; font-weight:800; font-size:16px;">
            🚀 ĐĂNG KÝ CỘNG TÁC VIÊN / ĐẠI LÝ TRỰC TIẾP
          </a>
        </div>
      </div>
    </div>
  </section>

  <!-- Download Section -->
  <section class="download" id="download">
    <div class="container">
      <div class="section-header">
        <span class="section-tag">DOWNLOAD PHẦN MỀM</span>
        <h2 class="section-title">Tải Về LiveStream Micro-DAW (v2.0.5)</h2>
      </div>

      <div class="dl-grid">
        <div class="dl-card" style="border-top: 4px solid var(--accent-red);">
          <div style="font-size: 40px; margin-bottom: 12px;">📦</div>
          <h3 style="font-family: var(--font-heading); font-size: 22px; margin-bottom: 8px;">Windows Setup</h3>
          <p style="color: var(--text-muted); font-size: 14px; margin-bottom: 20px;">Tự động cài Plugin OBS Receiver vào hệ thống, tạo icon Desktop và đính kèm đầy đủ tài liệu hướng dẫn.</p>
          <a href="{github_setup_zip_url}" class="btn-primary" style="width: 100%; justify-content: center; margin-bottom: 10px;" target="_blank">📥 TẢI BỘ SETUP (.ZIP)</a>
          <div style="font-size: 12.5px; color: var(--text-muted);">Hoặc tải trực tiếp: <a href="{github_setup_exe_url}" style="color: var(--accent-blue);" target="_blank">LiveStream_Micro_DAW_Setup.exe</a></div>
        </div>

        <div class="dl-card" style="border-top: 4px solid #64748b;">
          <div style="font-size: 40px; margin-bottom: 12px;">🚀</div>
          <h3 style="font-family: var(--font-heading); font-size: 22px; margin-bottom: 8px;">Windows Portable</h3>
          <p style="color: var(--text-muted); font-size: 14px; margin-bottom: 20px;">Chạy ngay không cần cài đặt, tiện lợi sao chép vào USB mang đi phòng thu khác, đính kèm tài liệu.</p>
          <a href="{github_portable_zip_url}" class="btn-secondary" style="width: 100%; justify-content: center; margin-bottom: 10px;" target="_blank">🚀 TẢI PORTABLE (.ZIP)</a>
          <div style="font-size: 12.5px; color: var(--text-muted);">Hoặc tải bản nén: <a href="{github_portable_rar_url}" style="color: var(--accent-blue);" target="_blank">Portable (.rar)</a></div>
        </div>

        <div class="dl-card" style="border-top: 4px solid var(--accent-blue);">
          <div style="font-size: 40px; margin-bottom: 12px;">🍏</div>
          <h3 style="font-family: var(--font-heading); font-size: 22px; margin-bottom: 8px;">macOS Universal</h3>
          <p style="color: var(--text-muted); font-size: 14px; margin-bottom: 20px;">Hỗ trợ trọn vẹn cả chip Apple Silicon (M1/M2/M3/M4) và Intel. Tích hợp CoreAudio &amp; OBS Plugin.</p>
          <a href="{github_macos_url}" class="btn-secondary" style="width: 100%; justify-content: center; margin-bottom: 10px;" target="_blank">🍏 TẢI BẢN MACOS (.ZIP)</a>
          <div style="font-size: 12.5px; color: var(--text-muted);">Tương thích macOS 10.15+ (Catalina ➔ Sequoia)</div>
        </div>
      </div>
    </div>
  </section>

  <!-- Payment / Donate Section -->
  <section class="payment" id="payment">
    <div class="container">
      <div class="section-header">
        <span class="section-tag">THANH TOÁN &amp; ĐỒNG HÀNH</span>
        <h2 class="section-title">Thông Tin Chuyển Khoản Mua Key &amp; Donate</h2>
      </div>

      <div class="qr-grid">
        <div class="qr-card">
          <img src="{vietqr_b64}" alt="VietQR MB Bank" class="qr-img">
          <div style="font-weight: 700; color: #ffffff;">VIETQR MB BANK</div>
          <div style="font-family: var(--font-mono); font-size: 14px; color: var(--accent-blue); font-weight: 700; margin: 4px 0;">0908107000</div>
          <div style="font-size: 12.5px; color: var(--text-muted);">LA CHÍ NHÂN</div>
        </div>

        <div class="qr-card">
          <img src="{momo_b64}" alt="Ví MoMo" class="qr-img">
          <div style="font-weight: 700; color: #ffffff;">VÍ MOMO</div>
          <div style="font-family: var(--font-mono); font-size: 14px; color: #d946ef; font-weight: 700; margin: 4px 0;">0908107000</div>
          <div style="font-size: 12.5px; color: var(--text-muted);">LA CHÍ NHÂN</div>
        </div>

        <div class="qr-card">
          <img src="{paypal_b64}" alt="PayPal" class="qr-img">
          <div style="font-weight: 700; color: #ffffff;">PAYPAL (QUỐC TẾ)</div>
          <div style="font-family: var(--font-mono); font-size: 13px; color: var(--accent-blue); font-weight: 700; margin: 4px 0;">lachinhan@gmail.com</div>
          <div style="font-size: 12.5px; color: var(--text-muted);">Hỗ trợ thẻ Visa/Mastercard</div>
        </div>

        <div class="qr-card">
          <img src="{mb_b64}" alt="MB Bank" class="qr-img">
          <div style="font-weight: 700; color: #ffffff;">THẺ MB BANK</div>
          <div style="font-family: var(--font-mono); font-size: 14px; color: var(--accent-blue); font-weight: 700; margin: 4px 0;">0908107000</div>
          <div style="font-size: 12.5px; color: var(--text-muted);">LA CHÍ NHÂN</div>
        </div>
      </div>
    </div>
  </section>

  <!-- Footer -->
  <footer class="site-footer">
    <div class="container">
      <p style="margin-bottom: 8px;">© 2026 <strong>LiveStream Micro-DAW v2.0.5</strong> by <strong>La Chí Nhân</strong> (Hosi Studio). All rights reserved.</p>
      <p>Trang chủ: <a href="https://www.lachinhan.xyz" target="_blank">www.lachinhan.xyz</a> • Hotline/Zalo: <strong>0908.107.000</strong> • <a href="{github_repo_url}" target="_blank">GitHub Repository</a></p>
    </div>
  </footer>

</body>
</html>
"""

output_path = os.path.join(workspace, "landing_page.html")
with open(output_path, "w", encoding="utf-8") as f:
    f.write(landing_html)

print(f"[SUCCESS] Generated landing_page.html successfully at: {output_path}")
