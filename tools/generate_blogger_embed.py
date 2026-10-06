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

# GitHub Direct Download Links
github_repo_url = "https://github.com/lachinhan/Hosi-Micro-Daw"
github_setup_zip_url = "https://github.com/lachinhan/Hosi-Micro-Daw/raw/main/Hosi%20Micro%20Daw%20Setup.zip"
github_setup_exe_url = "https://github.com/lachinhan/Hosi-Micro-Daw/raw/main/LiveStream_Micro_DAW_Setup.exe"
github_portable_zip_url = "https://github.com/lachinhan/Hosi-Micro-Daw/raw/main/Hosi%20Micro%20Daw%20Portable.zip"
github_macos_url = "https://github.com/lachinhan/Hosi-Micro-Daw"

# Blogger Post HTML (Perfect Balanced Width 88vw/1080px matching lachinhan.xyz About Me)
blogger_html = f"""<!-- =======================================================
     HOSI PROD - LIVESTREAM MICRO-DAW (ABOUT ME MATCHING STYLE)
     Hướng dẫn: Chuyển bài viết Blogger sang chế độ 'Xem HTML' (<>) và dán toàn bộ mã này vào
     ======================================================= -->

<link rel="preconnect" href="https://fonts.googleapis.com">
<link rel="preconnect" href="https://fonts.gstatic.com" crossorigin>
<link href="https://fonts.googleapis.com/css2?family=Open+Sans:ital,wght@0,400;0,600;0,700;0,800;1,400&family=Oswald:wght@400;500;600;700&family=JetBrains+Mono:wght@500;700&display=swap" rel="stylesheet">

<style>
  /* 1. Cho phép khung cha Blogger bung rộng không bị cắt */
  .post-body, .entry-content, .post-body-container, .item-view .post-body, .post, .item-post {{
    overflow: visible !important;
  }}

  /* 2. BUNG RỘNG CÂN ĐỐI & CĂN GIỮA CHUẨN XÁC GIỐNG HỆT TRANG ABOUT ME */
  #micro-daw-root {{
    position: relative;
    left: 50%;
    transform: translateX(-50%);
    width: 88vw;
    max-width: 1080px;
    box-sizing: border-box;
    margin: 20px 0 40px 0;
    font-family: 'Open Sans', Arial, 'Helvetica Neue', sans-serif;
    color: #333333;
    line-height: 1.7;
  }}

  #micro-daw-root * {{
    box-sizing: border-box;
  }}

  /* Header Box */
  #micro-daw-root .hosi-daw-hero {{
    background: #ffffff;
    border-radius: 16px;
    padding: 35px 30px;
    border: 1px solid #eaeaea;
    border-top: 6px solid rgb(255, 25, 25);
    box-shadow: 0 10px 30px rgba(0, 0, 0, 0.06);
    margin-bottom: 30px;
    position: relative;
  }}

  #micro-daw-root .hosi-badge-tag {{
    display: inline-block;
    background: rgba(255, 25, 25, 0.1);
    color: rgb(255, 25, 25);
    font-size: 12px;
    font-weight: 700;
    letter-spacing: 1px;
    padding: 4px 14px;
    border-radius: 20px;
    text-transform: uppercase;
    margin-bottom: 12px;
  }}

  #micro-daw-root .hosi-heading-main {{
    font-family: 'Oswald', Helvetica, Arial, sans-serif;
    font-size: 32px;
    font-weight: 600;
    color: #111111;
    text-transform: uppercase;
    letter-spacing: 0.5px;
    line-height: 1.2;
    margin: 0 0 12px 0;
  }}

  #micro-daw-root .hosi-heading-about {{
    background-color: #fafafa;
    border-right: 6px solid rgb(255, 25, 25);
    display: inline-block;
    font-family: 'Oswald', Helvetica, Arial, sans-serif;
    font-size: 26px;
    font-weight: 500;
    letter-spacing: 0.5px;
    line-height: 1.1;
    margin: 0 0 14px 0;
    padding: 12px 18px;
    text-transform: uppercase;
    color: #222;
  }}

  #micro-daw-root .hosi-lead-desc {{
    font-size: 15px;
    color: #555555;
    margin-bottom: 25px;
    text-align: justify;
  }}

  /* CTA Buttons (Balanced 2x2 Grid) */
  #micro-daw-root .hosi-btn-grid {{
    display: grid;
    grid-template-columns: repeat(2, 1fr);
    gap: 12px;
    max-width: 660px;
    margin: 24px auto 0 auto;
  }}
  @media (max-width: 600px) {{
    #micro-daw-root .hosi-btn-grid {{
      grid-template-columns: 1fr;
    }}
  }}

  #micro-daw-root .hosi-btn-studio {{
    background: rgb(255, 25, 25);
    color: #ffffff !important;
    padding: 13px 20px;
    border-radius: 8px;
    font-weight: 700;
    font-size: 13.5px;
    text-decoration: none !important;
    text-transform: uppercase;
    letter-spacing: 0.4px;
    transition: all 0.2s ease;
    display: inline-flex;
    align-items: center;
    justify-content: center;
    gap: 8px;
    width: 100%;
    text-align: center;
  }}
  #micro-daw-root .hosi-btn-studio:hover {{
    background: #d61414;
    box-shadow: 0 4px 14px rgba(255, 25, 25, 0.35);
    transform: translateY(-2px);
  }}

  #micro-daw-root .hosi-btn-ai {{
    background: #0284c7;
    color: #ffffff !important;
    padding: 13px 20px;
    border-radius: 8px;
    font-weight: 700;
    font-size: 13.5px;
    text-decoration: none !important;
    text-transform: uppercase;
    letter-spacing: 0.4px;
    transition: all 0.2s ease;
    display: inline-flex;
    align-items: center;
    justify-content: center;
    gap: 8px;
    width: 100%;
    text-align: center;
  }}
  #micro-daw-root .hosi-btn-ai:hover {{
    background: #0369a1;
    box-shadow: 0 4px 14px rgba(2, 132, 199, 0.35);
    transform: translateY(-2px);
  }}

  #micro-daw-root .hosi-btn-github {{
    background: #24292f;
    color: #ffffff !important;
    border: 1px solid #444c56;
    padding: 13px 20px;
    border-radius: 8px;
    font-weight: 700;
    font-size: 13.5px;
    text-decoration: none !important;
    text-transform: uppercase;
    letter-spacing: 0.4px;
    transition: all 0.2s ease;
    display: inline-flex;
    align-items: center;
    justify-content: center;
    gap: 8px;
    width: 100%;
    text-align: center;
  }}
  #micro-daw-root .hosi-btn-github:hover {{
    background: #2f363d;
    border-color: #8b949e;
    transform: translateY(-2px);
    box-shadow: 0 4px 14px rgba(0, 0, 0, 0.3);
  }}

  #micro-daw-root .hosi-btn-portfolio {{
    background: #fafafa;
    color: #444444 !important;
    border: 1px solid #ddd;
    padding: 13px 20px;
    border-radius: 8px;
    font-weight: 600;
    font-size: 13.5px;
    text-decoration: none !important;
    text-transform: uppercase;
    letter-spacing: 0.4px;
    transition: all 0.2s ease;
    display: inline-flex;
    align-items: center;
    justify-content: center;
    gap: 8px;
    width: 100%;
    text-align: center;
  }}
  #micro-daw-root .hosi-btn-portfolio:hover {{
    background: #f0f0f0;
    border-color: #bbb;
    transform: translateY(-2px);
  }}

  /* 6 Features Grid */
  #micro-daw-root .hosi-features-grid {{
    display: grid;
    grid-template-columns: repeat(auto-fit, minmax(310px, 1fr));
    gap: 20px;
    margin: 25px 0 35px 0;
  }}

  #micro-daw-root .hosi-feature-card {{
    background: #ffffff;
    border: 1px solid #eaeaea;
    border-top: 4px solid #444;
    border-radius: 12px;
    padding: 22px 20px;
    box-shadow: 0 4px 15px rgba(0, 0, 0, 0.04);
    transition: all 0.25s ease;
  }}
  #micro-daw-root .hosi-feature-card:hover {{
    border-top-color: rgb(255, 25, 25);
    transform: translateY(-3px);
    box-shadow: 0 8px 25px rgba(0, 0, 0, 0.08);
  }}

  #micro-daw-root .hosi-feature-icon {{
    font-size: 28px;
    margin-bottom: 10px;
    display: inline-block;
  }}

  #micro-daw-root .hosi-feature-title {{
    font-family: 'Oswald', sans-serif;
    font-size: 19px;
    font-weight: 600;
    color: #111;
    text-transform: uppercase;
    margin: 0 0 8px 0;
    letter-spacing: 0.3px;
  }}

  #micro-daw-root .hosi-feature-desc {{
    font-size: 14px;
    color: #666;
    line-height: 1.6;
    margin: 0;
  }}

  /* AI Assistant Section */
  #micro-daw-root .hosi-ai-section {{
    background: linear-gradient(135deg, #f0f9ff 0%, #e0f2fe 100%);
    border-radius: 16px;
    padding: 35px 30px;
    border: 1px solid #bae6fd;
    border-left: 6px solid #0284c7;
    box-shadow: 0 10px 30px rgba(2, 132, 199, 0.08);
    margin: 35px 0;
    position: relative;
  }}

  /* Songbook Box */
  #micro-daw-root .hosi-songbook-box {{
    background: #ffffff;
    border-radius: 16px;
    padding: 30px 25px;
    border: 1px solid #eaeaea;
    border-left: 6px solid rgb(255, 25, 25);
    box-shadow: 0 6px 20px rgba(0, 0, 0, 0.04);
    margin: 30px 0;
  }}

  #micro-daw-root .hosi-song-row {{
    background: #fafafa;
    border-radius: 8px;
    padding: 12px 16px;
    display: flex;
    align-items: center;
    justify-content: space-between;
    margin-bottom: 10px;
    border: 1px solid #eee;
  }}

  #micro-daw-root .hosi-tone-tag {{
    padding: 4px 10px;
    border-radius: 6px;
    font-family: 'JetBrains Mono', monospace;
    font-weight: 700;
    font-size: 12px;
    display: inline-block;
  }}

  /* Download Box */
  #micro-daw-root .hosi-download-section {{
    background: #fafafa;
    border-radius: 16px;
    padding: 35px 25px;
    border: 1px solid #eaeaea;
    border-top: 6px solid rgb(255, 25, 25);
    box-shadow: 0 10px 30px rgba(0, 0, 0, 0.06);
    margin: 35px 0;
    text-align: center;
  }}

  #micro-daw-root .hosi-dl-grid {{
    display: grid;
    grid-template-columns: repeat(auto-fit, minmax(300px, 1fr));
    gap: 20px;
    margin-top: 25px;
    text-align: left;
  }}

  #micro-daw-root .hosi-dl-card {{
    background: #ffffff;
    border: 1px solid #e2e8f0;
    border-radius: 12px;
    padding: 24px;
    box-shadow: 0 4px 12px rgba(0,0,0,0.03);
    text-align: center;
  }}

  /* Donate Section */
  #micro-daw-root .hosi-donate-section {{
    background: #ffffff;
    border-radius: 16px;
    padding: 35px 25px;
    border: 1px solid #eaeaea;
    border-top: 6px solid rgb(255, 25, 25);
    box-shadow: 0 10px 30px rgba(0, 0, 0, 0.06);
    margin: 35px 0 20px 0;
  }}

  #micro-daw-root .hosi-donate-tabs {{
    display: flex;
    justify-content: center;
    gap: 8px;
    flex-wrap: wrap;
    margin: 20px 0;
  }}

  #micro-daw-root .hosi-tab-btn {{
    background: #f4f4f4;
    border: 1px solid #ddd;
    color: #444;
    padding: 8px 16px;
    border-radius: 6px;
    font-family: 'Oswald', sans-serif;
    font-weight: 500;
    font-size: 14px;
    text-transform: uppercase;
    letter-spacing: 0.5px;
    cursor: pointer;
    transition: all 0.2s ease;
  }}
  #micro-daw-root .hosi-tab-btn:hover {{
    background: #eaeaea;
    border-color: #bbb;
  }}
  #micro-daw-root .hosi-tab-btn.active {{
    background: rgb(255, 25, 25);
    color: #ffffff;
    border-color: rgb(255, 25, 25);
  }}

  #micro-daw-root .hosi-donate-content {{
    display: grid;
    grid-template-columns: 240px 1fr;
    gap: 30px;
    align-items: center;
    background: #fafafa;
    padding: 25px;
    border-radius: 12px;
    border: 1px solid #eee;
  }}

  #micro-daw-root .hosi-qr-img {{
    width: 100%;
    max-width: 220px;
    height: auto;
    border-radius: 8px;
    box-shadow: 0 4px 15px rgba(0,0,0,0.1);
    border: 3px solid #fff;
    display: block;
    margin: 0 auto;
  }}

  #micro-daw-root .hosi-info-row {{
    background: #ffffff;
    border: 1px solid #e5e7eb;
    padding: 10px 14px;
    border-radius: 8px;
    margin-bottom: 10px;
    display: flex;
    justify-content: space-between;
    align-items: center;
  }}

  #micro-daw-root .hosi-copy-btn {{
    background: #222;
    color: #fff;
    border: none;
    padding: 5px 12px;
    border-radius: 6px;
    font-family: 'Oswald', sans-serif;
    font-size: 13px;
    text-transform: uppercase;
    cursor: pointer;
    transition: all 0.2s ease;
  }}
  #micro-daw-root .hosi-copy-btn:hover {{
    background: rgb(255, 25, 25);
  }}

  #micro-daw-root .hosi-quote-box {{
    background: #fafafa;
    border-left: 4px solid rgb(255, 25, 25);
    padding: 15px 20px;
    font-style: italic;
    color: #555;
    margin-top: 25px;
    border-radius: 0 8px 8px 0;
  }}

  /* Mobile Responsive */
  @media (max-width: 768px) {{
    #micro-daw-root {{
      width: 100% !important;
      left: 0 !important;
      transform: none !important;
      padding: 0 !important;
    }}
    #micro-daw-root .hosi-heading-main {{ font-size: 24px; }}
    #micro-daw-root .hosi-donate-content {{
      grid-template-columns: 1fr;
      text-align: center;
    }}
    #micro-daw-root .hosi-song-row {{
      flex-direction: column;
      align-items: flex-start;
      gap: 8px;
    }}
  }}
</style>

<div id="micro-daw-root">

  <!-- HERO SECTION -->
  <div class="hosi-daw-hero">
    <span class="hosi-badge-tag">⚡ LiveStream Micro-DAW v2.0 • Zero-Latency IPC</span>
    <h1 class="hosi-heading-main">
      PHẦN MỀM HÁT LIVE & THU ÂM CHUYÊN NGHIỆP<br>
      <span style="color: rgb(255, 25, 25);">TỰ ĐỘNG DÒ TONE SỐ 1 CHO STREAMER</span>
    </h1>

    <div class="hosi-lead-desc">
      <strong>LiveStream Micro-DAW</strong> là giải pháp phần mềm Audio Router & VST3 Host gọn nhẹ do <strong>Hosi Prod (La Chí Nhân)</strong> nghiên cứu và phát triển độc quyền. Tự động nhận diện Tone beat bài hát & giọng hát Micro, 1-Click đồng bộ Auto-Tune Pro tức thì, truyền âm thanh độ trễ <strong>0ms</strong> trực tiếp sang OBS Studio không cần bất kỳ dây cáp ảo nào.
    </div>

    <div class="hosi-btn-grid">
      <a href="#hosi-dl-box" class="hosi-btn-studio">
        📥 Tải Phần Mềm Miễn Phí
      </a>
      <a href="https://byvn.net/hosiguide" target="_blank" class="hosi-btn-ai">
        🤖 Trợ Lý AI Hướng Dẫn
      </a>
      <a href="{github_repo_url}" target="_blank" class="hosi-btn-github">
        ⭐ GitHub Repository
      </a>
      <a href="#hosi-donate-box" class="hosi-btn-portfolio">
        💖 Ủng Hộ Tác Giả (Donate)
      </a>
    </div>
  </div>

  <!-- 6 TÍNH NĂNG ĐỘT PHÁ -->
  <div style="margin: 35px 0 15px 0;">
    <h2 class="hosi-heading-about">Tính Năng Vượt Trội</h2>
  </div>

  <div class="hosi-features-grid">
    
    <div class="hosi-feature-card">
      <div class="hosi-feature-icon">🎵</div>
      <h3 class="hosi-feature-title">Tự Động Dò Tone Beat & Micro</h3>
      <p class="hosi-feature-desc">Thuật toán Chromagram nhận diện chính xác nốt Root & Scale (Trưởng/Thứ) của beat karaoke. 1-Click gán chuẩn xác vào Auto-Tune Pro trong Rack.</p>
    </div>

    <div class="hosi-feature-card">
      <div class="hosi-feature-icon">📖</div>
      <h3 class="hosi-feature-title">Sổ Tone 1.033+ Bài Hát Việt Nam</h3>
      <p class="hosi-feature-desc">Tích hợp sẵn kho dữ liệu bài hát khổng lồ (Bolero, Nhạc Trẻ, Nhạc Trịnh, Ballad...) kèm đầy đủ Tone Nam, Tone Nữ, Tone Gốc và bộ lọc yêu thích.</p>
    </div>

    <div class="hosi-feature-card">
      <div class="hosi-feature-icon">🎙️</div>
      <h3 class="hosi-feature-title">Smart Auto-Ducking Pro (Talk-Over)</h3>
      <p class="hosi-feature-desc">Tự động hạ nhỏ âm lượng beat khi streamer nói chuyện và đẩy mượt trở lại khi ngừng nói. Menu chuột phải tùy chỉnh mức giảm dB và độ nhạy Micro.</p>
    </div>

    <div class="hosi-feature-card">
      <div class="hosi-feature-icon">⚡</div>
      <h3 class="hosi-feature-title">Zero-Latency Sang OBS Studio</h3>
      <p class="hosi-feature-desc">Truyền âm thanh trực tiếp sang OBS qua Windows Named Shared Memory (0ms delay). Không cần cài đặt Virtual Audio Cable hay Voicemeeter!</p>
    </div>

    <div class="hosi-feature-card">
      <div class="hosi-feature-icon">🎛️</div>
      <h3 class="hosi-feature-title">Rack 8 Slot VST3 & Vocal DSP</h3>
      <p class="hosi-feature-desc">Hỗ trợ mọi plugin VST3 chuyên nghiệp (Auto-Tune, FabFilter, Waves...) kèm bộ 6 hiệu ứng giọng hát Studio tích hợp sẵn (Gate, EQ, Comp, Reverb, Echo, Limiter).</p>
    </div>

    <div class="hosi-feature-card">
      <div class="hosi-feature-icon">🎹</div>
      <h3 class="hosi-feature-title">Soundboard 8 Pad & Phím Tắt F1..F4</h3>
      <p class="hosi-feature-desc">Bàn phím hiệu ứng âm thanh vỗ tay 👏, cười haha 😂 cùng phím nóng đổi Scene tức thì: F1 (Hát Live) ↔ F2 (Giao Lưu) ↔ F3 (AutoTune) ↔ F4/Tab.</p>
    </div>

  </div>

  <!-- SỔ TONE SHOWCASE -->
  <div class="hosi-songbook-box">
    <div style="display: flex; justify-content: space-between; align-items: center; flex-wrap: wrap; gap: 12px; margin-bottom: 18px;">
      <div>
        <span class="hosi-badge-tag">TÍCH HỢP SẴN TRONG DAW</span>
        <h3 style="font-family: 'Oswald', sans-serif; font-size: 22px; color: #111; margin: 4px 0 0 0; text-transform: uppercase;">Sổ Tone Bài Hát & Tone Nam / Nữ Thông Minh</h3>
      </div>
      <span style="font-family: 'JetBrains Mono', monospace; font-size: 13px; background: rgba(255, 25, 25, 0.1); border: 1px solid rgba(255, 25, 25, 0.3); padding: 5px 12px; border-radius: 6px; color: rgb(255, 25, 25); font-weight: 700;">
        ⭐ 1.033+ Bài Hát Tuyển Chọn
      </span>
    </div>

    <!-- Demo Items -->
    <div class="hosi-song-row">
      <div style="display: flex; align-items: center; gap: 10px;">
        <span style="color: rgb(255, 25, 25); font-size: 18px;">❤️</span>
        <div>
          <strong style="color: #111; font-size: 15px;">Ai Chung Tình Được Mãi</strong>
          <span style="color: #777; font-size: 13px; margin-left: 8px;">Đinh Tùng Huy, Hoài Lâm • Pop Ballad</span>
        </div>
      </div>
      <div style="display: flex; gap: 6px;">
        <span class="hosi-tone-tag" style="background: #e0f2fe; color: #0369a1; border: 1px solid #bae6fd;">👨 Nam: Em</span>
        <span class="hosi-tone-tag" style="background: #fce7f3; color: #be185d; border: 1px solid #fbcfe8;">👩 Nữ: Am</span>
        <span class="hosi-tone-tag" style="background: #f1f5f9; color: #334155; border: 1px solid #e2e8f0;">Gốc: Em</span>
      </div>
    </div>

    <div class="hosi-song-row">
      <div style="display: flex; align-items: center; gap: 10px;">
        <span style="color: rgb(255, 25, 25); font-size: 18px;">❤️</span>
        <div>
          <strong style="color: #111; font-size: 15px;">Chúng Ta Của Hiện Tại</strong>
          <span style="color: #777; font-size: 13px; margin-left: 8px;">Sơn Tùng M-TP • Pop / 80s</span>
        </div>
      </div>
      <div style="display: flex; gap: 6px;">
        <span class="hosi-tone-tag" style="background: #e0f2fe; color: #0369a1; border: 1px solid #bae6fd;">👨 Nam: Eb</span>
        <span class="hosi-tone-tag" style="background: #fce7f3; color: #be185d; border: 1px solid #fbcfe8;">👩 Nữ: Ab</span>
        <span class="hosi-tone-tag" style="background: #f1f5f9; color: #334155; border: 1px solid #e2e8f0;">Gốc: Eb</span>
      </div>
    </div>

    <div style="font-size: 13.5px; color: #666; margin-top: 15px;">
      💡 <em>Hỗ trợ: Lưu tone riêng cá nhân, tăng/giảm nửa cung Transpose (-1, +1), xuất/nhập file JSON để sao lưu.</em>
    </div>
  </div>

  <!-- AI ASSISTANT GUIDE SECTION -->
  <div class="hosi-ai-section">
    <div style="display: flex; align-items: center; justify-content: space-between; flex-wrap: wrap; gap: 24px;">
      <div style="flex: 1 1 500px;">
        <span class="hosi-badge-tag" style="background: rgba(2, 132, 199, 0.12); color: #0284c7;">
          🤖 TRỢ LÝ AI TRỰC TUYẾN 24/7
        </span>
        <h2 style="font-family: 'Oswald', sans-serif; font-size: 26px; color: #0f172a; margin: 4px 0 10px 0; text-transform: uppercase;">
          Trợ Lý AI Hướng Dẫn Sử Dụng Trực Tuyến
        </h2>
        <p style="color: #475569; font-size: 14.5px; margin: 0 0 20px 0; line-height: 1.7;">
          Bạn cần giải đáp thắc mắc về cách kết nối OBS Studio, cài đặt Rack VST3, tinh chỉnh Auto-Tune hay cấu hình Vocal DSP? Hãy trò chuyện ngay với <strong>Trợ Lý AI Hosi</strong> – được huấn luyện chuyên sâu để hỗ trợ bạn từng bước hoàn toàn miễn phí!
        </p>
        <div style="display: flex; gap: 12px; flex-wrap: wrap;">
          <a href="https://byvn.net/hosiguide" target="_blank" class="hosi-btn-ai">
            💬 Mở Trợ Lý AI Hướng Dẫn (byvn.net/hosiguide) ↗
          </a>
          <a href="{github_repo_url}" target="_blank" class="hosi-btn-github">
            ⭐ Xem Dự Án Trên GitHub ↗
          </a>
        </div>
      </div>
      <div style="flex: 0 0 auto; text-align: center; margin: 0 auto;">
        <div style="background: #ffffff; border: 2px dashed #0284c7; border-radius: 16px; padding: 22px 28px; box-shadow: 0 4px 15px rgba(2, 132, 199, 0.1);">
          <div style="font-size: 45px; margin-bottom: 6px;">🤖</div>
          <div style="font-family: 'Oswald', sans-serif; font-size: 16px; font-weight: 600; color: #0369a1; text-transform: uppercase;">AI Guide 24/7</div>
          <div style="font-size: 12px; color: #64748b; margin-top: 4px;">Hỗ trợ cài đặt & sử dụng</div>
        </div>
      </div>
    </div>
  </div>

  <!-- DOWNLOAD SECTION -->
  <div class="hosi-download-section" id="hosi-dl-box">
    <span class="hosi-badge-tag">TẢI VỀ MIỄN PHÍ 100% • GITHUB TỐC ĐỘ CAO</span>
    <h2 style="font-family: 'Oswald', sans-serif; font-size: 26px; color: #111; margin: 6px 0 10px 0; text-transform: uppercase;">
      Download LiveStream Micro-DAW
    </h2>
    <p style="color: #666; font-size: 14.5px; max-width: 680px; margin: 0 auto 20px auto;">
      Hỗ trợ đầy đủ Windows 10/11 & macOS. Đã tích hợp sẵn file hướng dẫn sử dụng (README) chi tiết bên trong:
    </p>

    <div class="hosi-dl-grid" style="grid-template-columns: repeat(auto-fit, minmax(280px, 1fr));">
      <!-- Setup (Windows) -->
      <div class="hosi-dl-card" style="border-top: 4px solid rgb(255, 25, 25);">
        <div style="font-size: 38px; margin-bottom: 8px;">📦</div>
        <h3 style="font-family: 'Oswald', sans-serif; font-size: 20px; color: #111; margin: 0 0 6px 0; text-transform: uppercase;">
          Windows Setup ⭐
        </h3>
        <p style="font-size: 13.5px; color: #666; margin: 0 0 16px 0; min-height: 42px;">
          Tự động cài Plugin OBS Receiver vào hệ thống, tạo icon Desktop và đính kèm đầy đủ tài liệu hướng dẫn.
        </p>
        <a href="{github_setup_zip_url}" target="_blank" class="hosi-btn-studio" style="width: 100%; justify-content: center; margin-bottom: 10px;">
          📥 Tải Trọn Gói Setup (.zip)
        </a>
        <div style="font-size: 12.5px; color: #64748b; margin-top: 6px;">
          Hoặc tải trực tiếp: <a href="{github_setup_exe_url}" target="_blank" style="color: rgb(255, 25, 25); font-weight: 700; text-decoration: none;">LiveStream_Micro_DAW_Setup.exe</a>
        </div>
        <div style="font-size: 12px; color: #888; margin-top: 8px;">Windows 10/11 (64-bit) • ~6.14 MB</div>
      </div>

      <!-- Portable (Windows) -->
      <div class="hosi-dl-card" style="border-top: 4px solid #444;">
        <div style="font-size: 38px; margin-bottom: 8px;">🚀</div>
        <h3 style="font-family: 'Oswald', sans-serif; font-size: 20px; color: #111; margin: 0 0 6px 0; text-transform: uppercase;">
          Windows Portable
        </h3>
        <p style="font-size: 13.5px; color: #666; margin: 0 0 16px 0; min-height: 42px;">
          Chạy ngay không cần cài đặt, tiện lợi sao chép vào USB mang đi phòng thu khác, đính kèm đầy đủ tài liệu.
        </p>
        <a href="{github_portable_zip_url}" target="_blank" class="hosi-btn-portfolio" style="width: 100%; justify-content: center; margin-bottom: 10px; background: #24292f; color: #fff !important; border-color: #24292f;">
          🚀 Tải Bản Portable (.zip)
        </a>
        <div style="font-size: 12.5px; color: #64748b; margin-top: 6px;">
          Giải nén và chạy trực tiếp file <strong>.exe</strong>
        </div>
        <div style="font-size: 12px; color: #888; margin-top: 8px;">Windows 10/11 (64-bit) • ~6.52 MB</div>
      </div>

      <!-- macOS Universal -->
      <div class="hosi-dl-card" style="border-top: 4px solid #0284c7;">
        <div style="font-size: 38px; margin-bottom: 8px;">🍏</div>
        <h3 style="font-family: 'Oswald', sans-serif; font-size: 20px; color: #111; margin: 0 0 6px 0; text-transform: uppercase;">
          macOS Universal
        </h3>
        <p style="font-size: 13.5px; color: #666; margin: 0 0 16px 0; min-height: 42px;">
          Hỗ trợ trọn vẹn cả chip <strong>Apple Silicon (M1/M2/M3/M4)</strong> và <strong>Intel</strong>. Tích hợp sẵn CoreAudio & OBS Plugin (AU/VST3).
        </p>
        <a href="{github_macos_url}" target="_blank" class="hosi-btn-ai" style="width: 100%; justify-content: center; margin-bottom: 10px;">
          🍏 Tải Bản macOS (.app / .zip)
        </a>
        <div style="font-size: 12.5px; color: #64748b; margin-top: 6px;">
          Tương thích macOS 10.15+ (Catalina -> Sequoia)
        </div>
        <div style="font-size: 12px; color: #888; margin-top: 8px;">Universal Binary (arm64 + x86_64)</div>
      </div>
    </div>

    <!-- GitHub Direct Link Banner -->
    <div style="margin-top: 22px; padding: 14px 20px; background: #ffffff; border: 1px solid #e2e8f0; border-radius: 10px; display: flex; justify-content: space-between; align-items: center; flex-wrap: wrap; gap: 12px;">
      <div style="text-align: left;">
        <strong style="color: #111; font-size: 14px;">⭐ Dự án chính thức trên GitHub:</strong>
        <div style="font-size: 12.5px; color: #64748b;">Theo dõi dự án, tải bản phát hành & cập nhật phiên bản mới nhất</div>
      </div>
      <a href="{github_repo_url}" target="_blank" class="hosi-btn-github" style="padding: 8px 18px; font-size: 13px;">
        Xem GitHub Repo ↗
      </a>
    </div>
  </div>

  <!-- DONATE SECTION -->
  <div class="hosi-donate-section" id="hosi-donate-box">
    <div style="text-align: center;">
      <span class="hosi-badge-tag">💖 ỦNG HỘ PHÁT TRIỂN / DONATE</span>
      <h2 style="font-family: 'Oswald', sans-serif; font-size: 26px; color: #111; margin: 6px 0 10px 0; text-transform: uppercase;">
        Đồng Hành Cùng Tác Giả
      </h2>
      <p style="color: #666; font-size: 14.5px; max-width: 700px; margin: 0 auto;">
        Sự ủng hộ tự nguyện của bạn là nguồn động lực to lớn giúp tác giả duy trì, nâng cấp tính năng mới và chia sẻ phần mềm miễn phí cho cộng đồng!
      </p>
    </div>

    <!-- Tabs -->
    <div class="hosi-donate-tabs">
      <button class="hosi-tab-btn active" onclick="bloggerSwitchQr('vietqr', this)">🏦 VietQR MB Bank</button>
      <button class="hosi-tab-btn" onclick="bloggerSwitchQr('momo', this)">📱 Ví MoMo</button>
      <button class="hosi-tab-btn" onclick="bloggerSwitchQr('paypal', this)">🌐 PayPal</button>
      <button class="hosi-tab-btn" onclick="bloggerSwitchQr('mbcard', this)">💳 Thẻ MB Bank</button>
    </div>

    <!-- Content -->
    <div class="hosi-donate-content">
      <div style="text-align: center;">
        <img id="blogger-qr-img" src="{vietqr_b64}" alt="QR Donate" class="hosi-qr-img" />
      </div>

      <div>
        <div class="hosi-info-row">
          <div>
            <div style="font-size: 11px; color: #888; font-weight: 700; text-transform: uppercase;">CHỦ TÀI KHOẢN</div>
            <div style="font-family: 'Oswald', sans-serif; font-weight: 600; color: #111; font-size: 18px;" id="blogger-qr-name">LA CHÍ NHÂN</div>
          </div>
        </div>

        <div class="hosi-info-row">
          <div>
            <div style="font-size: 11px; color: #888; font-weight: 700; text-transform: uppercase;" id="blogger-qr-acc-lbl">SỐ TÀI KHOẢN</div>
            <div style="font-family: 'JetBrains Mono', monospace; font-weight: 700; color: rgb(255, 25, 25); font-size: 19px;" id="blogger-qr-acc-val">0908107000</div>
          </div>
          <button class="hosi-copy-btn" onclick="bloggerCopyText('0908107000')">Sao Chép</button>
        </div>

        <div class="hosi-info-row">
          <div>
            <div style="font-size: 11px; color: #888; font-weight: 700; text-transform: uppercase;">NGÂN HÀNG / VÍ</div>
            <div style="font-size: 14.5px; color: #333; font-weight: 600;" id="blogger-qr-bank">MB Bank (Ngân hàng Quân Đội)</div>
          </div>
        </div>

        <div class="hosi-info-row">
          <div>
            <div style="font-size: 11px; color: #888; font-weight: 700; text-transform: uppercase;">NỘI DUNG CHUYỂN KHOẢN</div>
            <div style="font-size: 14px; color: #444;">Ung ho LiveStream Micro-DAW</div>
          </div>
          <button class="hosi-copy-btn" onclick="bloggerCopyText('Ung ho LiveStream Micro-DAW')">Sao Chép</button>
        </div>

        <div id="blogger-toast-copy" style="display: none; color: #16a34a; font-weight: 700; font-size: 13.5px; margin-top: 8px;">
          ✓ Đã sao chép vào bộ nhớ tạm!
        </div>
      </div>
    </div>

    <!-- Dịch vụ Setup chuyên nghiệp -->
    <div class="hosi-quote-box" style="margin-top: 25px;">
      <strong style="color: rgb(255, 25, 25); text-transform: uppercase;">🛠️ Dịch vụ Setup Âm Thanh Livestream & Cài Vocal Chain Chuyên Nghiệp:</strong><br>
      • Cài đặt trọn bộ Plugin VST3 hay nhất (Auto-Tune, FabFilter, Reverb, Delay) qua UltraViewer / TeamViewer.<br>
      • Tinh chỉnh âm thanh chuẩn phòng thu theo từng chất giọng và micro của bạn.<br>
      👉 <strong>Hotline / Zalo: 0908.107.000 (La Chí Nhân)</strong> • Website: <strong><a href="https://lachinhan.xyz" target="_blank" style="color: rgb(255, 25, 25); text-decoration: none;">lachinhan.xyz</a></strong>
    </div>
  </div>

  <div style="text-align: center; padding: 20px 0; color: #888; font-size: 13px;">
    © 2026 LiveStream Micro-DAW • Phát triển bởi <strong>La Chí Nhân</strong> (<a href="https://lachinhan.xyz" target="_blank" style="color: #444; text-decoration: none;">lachinhan.xyz</a>) • GitHub: <strong><a href="{github_repo_url}" target="_blank" style="color: rgb(255, 25, 25); text-decoration: none;">github.com/lachinhan/Hosi-Micro-Daw</a></strong>
  </div>

</div>

<script>
  const bloggerQrData = {{
    vietqr: {{
      src: "{vietqr_b64}",
      name: "LA CHÍ NHÂN",
      accLabel: "SỐ TÀI KHOẢN MB",
      accVal: "0908107000",
      bank: "MB Bank (Ngân hàng Quân Đội)"
    }},
    momo: {{
      src: "{momo_b64}",
      name: "LA CHÍ NHÂN",
      accLabel: "SỐ ĐIỆN THOẠI MOMO",
      accVal: "0908107000",
      bank: "Ví Điện Tử MoMo"
    }},
    paypal: {{
      src: "{paypal_b64}",
      name: "Nhan La Chi",
      accLabel: "TÀI KHOẢN PAYPAL",
      accVal: "Nhan La Chi",
      bank: "Cổng Thanh Toán Quốc Tế PayPal"
    }},
    mbcard: {{
      src: "{mb_b64}",
      name: "LA CHÍ NHÂN",
      accLabel: "SỐ TÀI KHOẢN MB",
      accVal: "0908107000",
      bank: "MB Bank (Thẻ Đa Năng)"
    }}
  }};

  function bloggerSwitchQr(type, btnElement) {{
    const data = bloggerQrData[type];
    if (!data) return;

    document.getElementById('blogger-qr-img').src = data.src;
    document.getElementById('blogger-qr-name').innerText = data.name;
    document.getElementById('blogger-qr-acc-lbl').innerText = data.accLabel;
    document.getElementById('blogger-qr-acc-val').innerText = data.accVal;
    document.getElementById('blogger-qr-bank').innerText = data.bank;

    const buttons = document.querySelectorAll('#micro-daw-root .hosi-tab-btn');
    buttons.forEach(btn => btn.classList.remove('active'));
    if (btnElement) btnElement.classList.add('active');
  }}

  function bloggerCopyText(text) {{
    navigator.clipboard.writeText(text).then(() => {{
      const toast = document.getElementById('blogger-toast-copy');
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
"""

blogger_output_path = os.path.join(workspace, "blogger_embed.html")
with open(blogger_output_path, "w", encoding="utf-8") as f:
    f.write(blogger_html)

print(f"Generated 100% Full Width (About Me Style) Blogger embed HTML with GitHub Links: {blogger_output_path} ({len(blogger_html)} bytes)")
