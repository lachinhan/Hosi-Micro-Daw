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

blogger_html = f"""<!-- =======================================================
     HOSI PROD - LIVESTREAM MICRO-DAW v2.0.5 BLOGGER EMBED
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
    font-size: 24px;
    font-weight: 500;
    letter-spacing: 0.5px;
    line-height: 1.1;
    margin: 30px 0 16px 0;
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

  /* CTA Buttons (Balanced Responsive Grid) */
  #micro-daw-root .hosi-btn-grid {{
    display: grid;
    grid-template-columns: repeat(auto-fit, minmax(210px, 1fr));
    gap: 12px;
    max-width: 960px;
    margin: 24px auto 0 auto;
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

  /* 8 Features Grid */
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
    font-size: 18.5px;
    font-weight: 600;
    color: #111;
    text-transform: uppercase;
    margin: 0 0 8px 0;
    letter-spacing: 0.3px;
  }}

  #micro-daw-root .hosi-feature-desc {{
    font-size: 13.5px;
    color: #666;
    line-height: 1.6;
    margin: 0;
  }}

  /* Pricing Cards Grid (4 Tiers) */
  #micro-daw-root .hosi-pricing-grid {{
    display: grid;
    grid-template-columns: repeat(auto-fit, minmax(230px, 1fr));
    gap: 16px;
    margin: 25px 0 35px 0;
  }}

  #micro-daw-root .hosi-price-card {{
    background: #ffffff;
    border: 1px solid #e2e8f0;
    border-radius: 14px;
    padding: 24px 20px;
    text-align: center;
    position: relative;
    box-shadow: 0 4px 15px rgba(0,0,0,0.03);
    transition: all 0.25s ease;
    display: flex;
    flex-direction: column;
    justify-content: space-between;
  }}
  #micro-daw-root .hosi-price-card.featured {{
    border: 2px solid #f59e0b;
    background: linear-gradient(180deg, #fffbeb 0%, #ffffff 100%);
    box-shadow: 0 8px 30px rgba(245, 158, 11, 0.15);
    transform: scale(1.02);
  }}
  #micro-daw-root .hosi-price-badge {{
    position: absolute;
    top: -12px;
    left: 50%;
    transform: translateX(-50%);
    background: #f59e0b;
    color: #ffffff;
    font-size: 11px;
    font-weight: 800;
    padding: 3px 12px;
    border-radius: 12px;
    text-transform: uppercase;
    letter-spacing: 0.5px;
  }}
  #micro-daw-root .hosi-price-name {{
    font-family: 'Oswald', sans-serif;
    font-size: 20px;
    color: #0f172a;
    margin-bottom: 8px;
    text-transform: uppercase;
  }}
  #micro-daw-root .hosi-price-val {{
    font-family: 'Oswald', sans-serif;
    font-size: 32px;
    font-weight: 700;
    color: rgb(255, 25, 25);
    line-height: 1;
    margin: 10px 0 4px 0;
  }}
  #micro-daw-root .hosi-price-duration {{
    font-size: 12.5px;
    color: #64748b;
    margin-bottom: 16px;
  }}

  /* Bulletproof List Styling to eliminate Blogger square bullet & entity overlap */
  #micro-daw-root .hosi-price-features,
  #micro-daw-root ul.hosi-price-features {{
    list-style: none !important;
    list-style-type: none !important;
    padding: 0 !important;
    margin: 0 0 20px 0 !important;
    text-align: left !important;
    font-size: 13px !important;
    color: #475569 !important;
    line-height: 1.6 !important;
  }}
  #micro-daw-root .hosi-price-features li,
  #micro-daw-root ul.hosi-price-features li {{
    list-style: none !important;
    list-style-type: none !important;
    padding: 3px 0 !important;
    margin: 0 0 4px 0 !important;
    display: flex !important;
    align-items: flex-start !important;
    gap: 8px !important;
  }}
  #micro-daw-root .hosi-chk {{
    color: #16a34a !important;
    font-weight: 800 !important;
    flex-shrink: 0 !important;
    font-size: 14px !important;
    line-height: 1.4 !important;
  }}
  #micro-daw-root .hosi-txt {{
    flex: 1 !important;
    font-size: 13px !important;
    line-height: 1.4 !important;
    color: #334155 !important;
  }}

  /* CTV Affiliate Section */
  #micro-daw-root .hosi-ctv-box {{
    background: #0f172a;
    color: #f8fafc;
    border-radius: 16px;
    padding: 35px 30px;
    margin: 35px 0;
    box-shadow: 0 10px 35px rgba(15, 23, 42, 0.25);
    border: 1px solid #1e293b;
  }}
  #micro-daw-root .hosi-ctv-box h3 {{
    font-family: 'Oswald', sans-serif;
    color: #facc15;
    font-size: 24px;
    text-transform: uppercase;
    margin-bottom: 12px;
  }}
  #micro-daw-root .hosi-ctv-table {{
    width: 100%;
    border-collapse: collapse;
    margin: 20px 0;
    font-size: 14px;
  }}
  #micro-daw-root .hosi-ctv-table th, #micro-daw-root .hosi-ctv-table td {{
    padding: 12px 14px;
    border: 1px solid #334155;
    text-align: left;
  }}
  #micro-daw-root .hosi-ctv-table th {{
    background: #1e293b;
    color: #38bdf8;
    font-family: 'Oswald', sans-serif;
    font-size: 15px;
  }}
  #micro-daw-root .hosi-ctv-table tr:nth-child(even) {{
    background: rgba(255, 255, 255, 0.02);
  }}

  /* Step Flow */
  #micro-daw-root .hosi-steps-grid {{
    display: grid;
    grid-template-columns: repeat(auto-fit, minmax(220px, 1fr));
    gap: 14px;
    margin: 20px 0;
  }}
  #micro-daw-root .hosi-step-item {{
    background: #1e293b;
    border-radius: 10px;
    padding: 16px;
    border-left: 4px solid #38bdf8;
  }}
  #micro-daw-root .hosi-step-num {{
    font-family: 'JetBrains Mono', monospace;
    font-size: 13px;
    color: #38bdf8;
    font-weight: 700;
    margin-bottom: 4px;
  }}
  #micro-daw-root .hosi-step-title {{
    font-size: 14.5px;
    font-weight: 700;
    color: #ffffff;
    margin-bottom: 6px;
  }}
  #micro-daw-root .hosi-step-desc {{
    font-size: 12.5px;
    color: #94a3b8;
    line-height: 1.5;
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
    grid-template-columns: repeat(auto-fit, minmax(280px, 1fr));
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
    <span class="hosi-badge-tag">⚡ LiveStream Micro-DAW v2.0.5 • Zero-Latency IPC</span>
    <h1 class="hosi-heading-main">
      PHẦN MỀM HÁT LIVE &amp; THU ÂM CHUYÊN NGHIỆP<br />
      <span style="color: #ff1919;">TỰ ĐỘNG DÒ TONE SỐ 1 CHO STREAMER</span>
    </h1>

    <div class="hosi-lead-desc">
      <strong>LiveStream Micro-DAW v2.0.5</strong> là giải pháp trạm âm thanh số (DAW) siêu nhẹ do <strong>Hosi Prod (La Chí Nhân)</strong> nghiên cứu và phát triển độc quyền. Tích hợp sẵn <strong>Mini YouTube Karaoke Player chặn quảng cáo</strong>, <strong>Lá chắn AI Noise &amp; De-Reverb</strong>, <strong>Tự động dò Tone Beat &amp; Micro</strong> truyền thẳng vào Auto-Tune, và truyền âm thanh sang OBS Studio độ trễ <strong>0ms</strong> trực tiếp không cần cài dây cáp ảo.
    </div>

    <div class="hosi-btn-grid">
      <a class="hosi-btn-studio" href="#hosi-dl-box">
        📥 Tải Phần Mềm Miễn Phí
      </a>
      <a class="hosi-btn-ai" href="https://byvn.net/hosiguide" target="_blank">
        🤖 Trợ Lý AI Hướng Dẫn
      </a>
      <a class="hosi-btn-github" href="{github_repo_url}" target="_blank">
        ⭐ GitHub Repository
      </a>
      <a class="hosi-btn-portfolio" href="{zalo_personal_url}" target="_blank">
        💬 Liên Hệ Mua Key PRO
      </a>
    </div>
  </div>

  <!-- 9 TÍNH NĂNG ĐỘT PHÁ -->
  <div>
    <h2 class="hosi-heading-about">9 Tính Năng Đột Phá Trên LiveStream Micro-DAW</h2>
  </div>

  <div class="hosi-features-grid">
    <div class="hosi-feature-card">
      <div class="hosi-feature-icon">📺</div>
      <h3 class="hosi-feature-title">Mini YouTube Karaoke Player</h3>
      <p class="hosi-feature-desc">Trình phát Beat YouTube siêu nhẹ (~40MB RAM), tự động bỏ qua 100% quảng cáo trong 0.1s, tăng/giảm Tone Beat trực tiếp và ghim nổi Always-On-Top khi livestream.</p>
    </div>

    <div class="hosi-feature-card">
      <div class="hosi-feature-icon">🛡️</div>
      <h3 class="hosi-feature-title">AI Real-Time Noise &amp; De-Reverb</h3>
      <p class="hosi-feature-desc">Mạng nơ-ron AI (DeepFilter GRU) thời gian thực, khử sạch tiếng quạt gió, ve kêu, tiếng còi xe và triệt tiêu dội âm phòng ngủ/hội trường tức thì mà không làm méo giọng.</p>
    </div>

    <div class="hosi-feature-card">
      <div class="hosi-feature-icon">🎙️</div>
      <h3 class="hosi-feature-title">AI Vocal Profiler &amp; Auto EQ</h3>
      <p class="hosi-feature-desc">Đo mẫu giọng 5 giây, tự động phân tích Formant và sinh đường cong 7-Band EQ lý tưởng (Studio Master, Bolero, Pop Remix, Talkshow) giúp giọng sáng và ấm.</p>
    </div>

    <div class="hosi-feature-card">
      <div class="hosi-feature-icon">🎵</div>
      <h3 class="hosi-feature-title">Tự Động Dò Tone Beat &amp; Micro</h3>
      <p class="hosi-feature-desc">Thuật toán Chromagram nhận diện chính xác nốt Root &amp; Scale (Trưởng/Thứ) của beat karaoke. 1-Click gán chuẩn xác vào Auto-Tune Pro trong Rack.</p>
    </div>

    <div class="hosi-feature-card">
      <div class="hosi-feature-icon">📖</div>
      <h3 class="hosi-feature-title">Sổ Tone 1.033+ Bài Hát &amp; AI Match</h3>
      <p class="hosi-feature-desc">Kho dữ liệu 1.033+ bài hát Việt Nam kèm đầy đủ Tone Nam, Tone Nữ, Tone Gốc. Tự động quét âm vực giọng (C2-C6) để gợi ý bài hát vừa vặn nhất (Fit Score 100%).</p>
    </div>

    <div class="hosi-feature-card">
      <div class="hosi-feature-icon">⏱️</div>
      <h3 class="hosi-feature-title">Smart BPM-Synced Reverb &amp; Delay</h3>
      <p class="hosi-feature-desc">Tự động nhận diện Tempo (BPM) bài hát, đồng bộ bước nhại Delay nảy tanh tách theo phách và khép đuôi Reverb mượt mà cuối ô nhịp.</p>
    </div>

    <div class="hosi-feature-card">
      <div class="hosi-feature-icon">⚡</div>
      <h3 class="hosi-feature-title">Zero-Latency Sang OBS Studio</h3>
      <p class="hosi-feature-desc">Truyền âm thanh trực tiếp sang OBS qua Windows Named Shared Memory (0ms delay). Không cần cài đặt Virtual Audio Cable hay Voicemeeter!</p>
    </div>

    <div class="hosi-feature-card">
      <div class="hosi-feature-icon">🎛️</div>
      <h3 class="hosi-feature-title">Rack 8 Slot VST3 &amp; Vocal DSP</h3>
      <p class="hosi-feature-desc">Hỗ trợ mọi plugin VST3 chuyên nghiệp (Auto-Tune, FabFilter, Waves...) kèm bộ 6 hiệu ứng giọng hát Studio tích hợp sẵn (Gate, EQ, Comp, Reverb, Echo, Limiter).</p>
    </div>

    <div class="hosi-feature-card">
      <div class="hosi-feature-icon">☁️</div>
      <h3 class="hosi-feature-title">Cloud Songbook &amp; Artist Preset Shop</h3>
      <p class="hosi-feature-desc">Tự động đồng bộ các bài hát Hot Trend TikTok/YouTube mới nhất vào Sổ Tone mỗi tuần qua Cloud. Kèm kho Preset phong cách ca sĩ/streamer nổi tiếng nạp tức thì chỉ bằng 1-Click.</p>
    </div>
  </div>

  <!-- SỔ TONE SHOWCASE -->
  <div class="hosi-songbook-box">
    <div style="align-items: center; display: flex; flex-wrap: wrap; gap: 12px; justify-content: space-between; margin-bottom: 18px;">
      <div>
        <span class="hosi-badge-tag">TÍCH HỢP SẴN TRONG DAW</span>
        <h3 style="color: #111111; font-family: 'Oswald', sans-serif; font-size: 22px; margin: 4px 0 0 0; text-transform: uppercase;">Sổ Tone Bài Hát &amp; Tone Nam / Nữ Thông Minh</h3>
      </div>
      <span style="background: rgba(255, 25, 25, 0.1); border-radius: 6px; border: 1px solid rgba(255, 25, 25, 0.3); color: #ff1919; font-family: 'JetBrains Mono', monospace; font-size: 13px; font-weight: 700; padding: 5px 12px;">
        ⭐ 1.033+ Bài Hát Tuyển Chọn
      </span>
    </div>

    <!-- Demo Items -->
    <div class="hosi-song-row">
      <div style="align-items: center; display: flex; gap: 10px;">
        <span style="color: #ff1919; font-size: 18px;">❤️</span>
        <div>
          <strong style="color: #111111; font-size: 15px;">Ai Chung Tình Được Mãi</strong>
          <span style="color: #777777; font-size: 13px; margin-left: 8px;">Đinh Tùng Huy, Hoài Lâm • Pop Ballad</span>
        </div>
      </div>
      <div style="display: flex; gap: 6px;">
        <span class="hosi-tone-tag" style="background: #e0f2fe; border: 1px solid #bae6fd; color: #0369a1;">👨 Nam: Em</span>
        <span class="hosi-tone-tag" style="background: #fce7f3; border: 1px solid #fbcfe8; color: #be185d;">👩 Nữ: Am</span>
        <span class="hosi-tone-tag" style="background: #f1f5f9; border: 1px solid #e2e8f0; color: #334155;">Gốc: Em</span>
      </div>
    </div>

    <div class="hosi-song-row">
      <div style="align-items: center; display: flex; gap: 10px;">
        <span style="color: #ff1919; font-size: 18px;">❤️</span>
        <div>
          <strong style="color: #111111; font-size: 15px;">Chúng Ta Của Hiện Tại</strong>
          <span style="color: #777777; font-size: 13px; margin-left: 8px;">Sơn Tùng M-TP • Pop / 80s</span>
        </div>
      </div>
      <div style="display: flex; gap: 6px;">
        <span class="hosi-tone-tag" style="background: #e0f2fe; border: 1px solid #bae6fd; color: #0369a1;">👨 Nam: Eb</span>
        <span class="hosi-tone-tag" style="background: #fce7f3; border: 1px solid #fbcfe8; color: #be185d;">👩 Nữ: Ab</span>
        <span class="hosi-tone-tag" style="background: #f1f5f9; border: 1px solid #e2e8f0; color: #334155;">Gốc: Eb</span>
      </div>
    </div>

    <div style="color: #666666; font-size: 13.5px; margin-top: 15px;">
      💡 <em>Hỗ trợ: Lưu tone riêng cá nhân, tăng/giảm nửa cung Transpose (-1, +1), xuất/nhập file JSON để sao lưu.</em>
    </div>
  </div>

  <!-- BẢNG GIÁ & GÓI BẢN QUYỀN PRO -->
  <div id="hosi-pricing-box">
    <h2 class="hosi-heading-about">Bảng Giá &amp; Các Gói Bản Quyền PRO</h2>
    <p class="hosi-lead-desc">Phần mềm cung cấp <strong>Bản Miễn Phí trọn đời</strong> cho cộng đồng và các gói <strong>Nâng cấp PRO VIP</strong> mở khóa trọn bộ tính năng AI đỉnh cao:</p>
  </div>

  <div class="hosi-pricing-grid">
    <!-- Tier 1: 7 Days Trial -->
    <div class="hosi-price-card">
      <div>
        <div class="hosi-price-name">🎁 Dùng Thử 7 Ngày</div>
        <div class="hosi-price-val">0đ</div>
        <div class="hosi-price-duration">Trải nghiệm Full tính năng PRO</div>
        <ul class="hosi-price-features">
          <li><span class="hosi-chk">✓</span><span class="hosi-txt">Mở khóa 100% tính năng PRO</span></li>
          <li><span class="hosi-chk">✓</span><span class="hosi-txt">YouTube Player chặn quảng cáo</span></li>
          <li><span class="hosi-chk">✓</span><span class="hosi-txt">Lá chắn AI Denoise &amp; De-Reverb</span></li>
          <li><span class="hosi-chk">✓</span><span class="hosi-txt">Đồng bộ Smart BPM Reverb/Delay</span></li>
          <li><span class="hosi-chk">✓</span><span class="hosi-txt">Hỗ trợ qua Zalo cộng đồng</span></li>
        </ul>
      </div>
      <a href="{zalo_trial_group_url}" target="_blank" class="hosi-btn-studio" style="background:#475569;">NHẬN KEY 7 NGÀY</a>
    </div>

    <!-- Tier 2: 1 Month -->
    <div class="hosi-price-card">
      <div>
        <div class="hosi-price-name">📅 Gói 1 Tháng</div>
        <div class="hosi-price-val">99.000đ</div>
        <div class="hosi-price-duration">Thời hạn 30 ngày sử dụng</div>
        <ul class="hosi-price-features">
          <li><span class="hosi-chk">✓</span><span class="hosi-txt">Full tính năng PRO VIP v2.0.5</span></li>
          <li><span class="hosi-chk">✓</span><span class="hosi-txt">YouTube Karaoke chặn quảng cáo</span></li>
          <li><span class="hosi-chk">✓</span><span class="hosi-txt">AI Denoise &amp; AI Vocal Profiler</span></li>
          <li><span class="hosi-chk">✓</span><span class="hosi-txt">Tự động dò tone &amp; đồng bộ Auto-Tune</span></li>
          <li><span class="hosi-chk">✓</span><span class="hosi-txt">Lý tưởng cho nhu cầu ngắn hạn</span></li>
        </ul>
      </div>
      <a href="{zalo_personal_url}" target="_blank" class="hosi-btn-ai">MUA GÓI 1 THÁNG</a>
    </div>

    <!-- Tier 3: 1 Year -->
    <div class="hosi-price-card">
      <div>
        <div class="hosi-price-name">⏱️ Gói 1 Năm</div>
        <div class="hosi-price-val">590.000đ</div>
        <div class="hosi-price-duration">Thời hạn 365 ngày (Tiết kiệm ~50%)</div>
        <ul class="hosi-price-features">
          <li><span class="hosi-chk">✓</span><span class="hosi-txt">Full tính năng PRO VIP</span></li>
          <li><span class="hosi-chk">✓</span><span class="hosi-txt">Cập nhật tính năng mới liên tục</span></li>
          <li><span class="hosi-chk">✓</span><span class="hosi-txt">Hỗ trợ kỹ thuật 1-1 qua UltraView</span></li>
          <li><span class="hosi-chk">✓</span><span class="hosi-txt">Đồng bộ Sổ Tone Cloud VIP</span></li>
          <li><span class="hosi-chk">✓</span><span class="hosi-txt">Chuyển đổi máy miễn phí</span></li>
        </ul>
      </div>
      <a href="{zalo_personal_url}" target="_blank" class="hosi-btn-studio" style="background:#059669;">MUA GÓI 1 NĂM</a>
    </div>

    <!-- Tier 4: Lifetime -->
    <div class="hosi-price-card featured">
      <div class="hosi-price-badge">🔥 PHỔ BIẾN NHẤT</div>
      <div>
        <div class="hosi-price-name">👑 PRO Trọn Đời</div>
        <div class="hosi-price-val">990.000đ</div>
        <div class="hosi-price-duration">Sở hữu Vĩnh Viễn • Nâng cấp trọn đời</div>
        <ul class="hosi-price-features">
          <li><span class="hosi-chk">✓</span><span class="hosi-txt">Mở khóa VĨNH VIỄN toàn bộ tính năng</span></li>
          <li><span class="hosi-chk">✓</span><span class="hosi-txt">Tất cả bản nâng cấp tương lai</span></li>
          <li><span class="hosi-chk">✓</span><span class="hosi-txt">Hỗ trợ setup phòng thu live chuyên nghiệp</span></li>
          <li><span class="hosi-chk">✓</span><span class="hosi-txt">Cấp lại Key trọn đời khi nâng cấp PC</span></li>
          <li><span class="hosi-chk">✓</span><span class="hosi-txt">Ưu tiên hỗ trợ kỹ thuật VIP 24/7</span></li>
        </ul>
      </div>
      <a href="{zalo_personal_url}" target="_blank" class="hosi-btn-studio">SỞ HỮU TRỌN ĐỜI</a>
    </div>
  </div>

  <!-- CHÍNH SÁCH DÀNH CHO CỘNG TÁC VIÊN / REVIEWER / ĐẠI LÝ -->
  <div class="hosi-ctv-box" id="hosi-ctv-box">
    <h3>🤝 CHƯƠNG TRÌNH ĐỐI TÁC &amp; CỘNG TÁC VIÊN (AFFILIATE PARTNER)</h3>
    <p style="color: #cbd5e1; font-size: 14.5px; margin-bottom: 20px;">
      Bạn là <strong>Reviewer Âm Thanh</strong>, <strong>Kỹ Thuật Viên Setup Phòng Thu</strong>, hoặc <strong>Admin Cộng Đồng Livestream</strong>? Hãy tham gia mạng lưới phân phối LiveStream Micro-DAW với mức chiết khấu hoa hồng hấp dẫn nhất thị trường:
    </p>

    <h4 style="color: #38bdf8; font-family: 'Oswald', sans-serif; font-size: 18px; margin-top: 20px;">1. BẢNG HOA HỒNG CHIẾT KHẤU THEO MỐC DOANH SỐ (KEY TRỌN ĐỜI 990K)</h4>
    <div style="overflow-x: auto;">
      <table class="hosi-ctv-table">
        <thead>
          <tr>
            <th>Doanh Số Tháng</th>
            <th>Hạng Đối Tác</th>
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
            <td>Thưởng thêm <strong>+5% (50.000đ/Key)</strong> vào cuối tháng</td>
            <td><strong>350.000 VNĐ / Key</strong></td>
          </tr>
          <tr>
            <td><strong>Từ 30 Key trở lên</strong></td>
            <td>Đại Lý VIP</td>
            <td><strong style="color:#f87171;">40%</strong></td>
            <td>Thưởng thêm <strong>+10% (100.000đ/Key)</strong> vào cuối tháng</td>
            <td><strong>400.000 VNĐ / Key</strong></td>
          </tr>
        </tbody>
      </table>
    </div>

    <h4 style="color: #38bdf8; font-family: 'Oswald', sans-serif; font-size: 18px; margin-top: 25px;">2. QUY TRÌNH PHỄU "TẶNG KEY 7 NGÀY" ➔ CHỐT ĐƠN TỰ ĐỘNG (HOW TO WORK)</h4>
    <div class="hosi-steps-grid">
      <div class="hosi-step-item">
        <div class="hosi-step-num">BƯỚC 1</div>
        <div class="hosi-step-title">Tặng Key Dùng Thử 7 Ngày</div>
        <p class="hosi-step-desc">Khách tải app Free, gửi Machine ID ➔ Bạn yêu cầu Admin cấp Key Trial 7 Ngày miễn phí cho khách trải nghiệm.</p>
      </div>
      <div class="hosi-step-item">
        <div class="hosi-step-num">BƯỚC 2</div>
        <div class="hosi-step-title">Khách Trải Nghiệm Chất Âm</div>
        <p class="hosi-step-desc">Khách hàng tận hưởng chất giọng mượt mà qua Auto EQ và YouTube chặn quảng cáo trong 7 ngày.</p>
      </div>
      <div class="hosi-step-item">
        <div class="hosi-step-num">BƯỚC 3</div>
        <div class="hosi-step-title">Hết Hạn &amp; Chốt Đơn</div>
        <p class="hosi-step-desc">Sau 7 ngày, app tự quay về Free. Khách đã quen chất âm VIP sẽ chủ động nhắn tin nâng cấp gói PRO.</p>
      </div>
      <div class="hosi-step-item">
        <div class="hosi-step-num">BƯỚC 4</div>
        <div class="hosi-step-title">Nhận Hoa Hồng Ngay</div>
        <p class="hosi-step-desc">Khách chuyển 990k ➔ Bạn giữ lại 300k - 400k và gửi phần còn lại để Admin xuất Key kích hoạt vĩnh viễn.</p>
      </div>
    </div>

    <h4 style="color: #38bdf8; font-family: 'Oswald', sans-serif; font-size: 18px; margin-top: 25px;">3. BA MÔ HÌNH VẬN HÀNH CTV LINH HOẠT</h4>
    <div class="hosi-steps-grid">
      <div class="hosi-step-item">
        <div class="hosi-step-num">MÔ HÌNH 1</div>
        <div class="hosi-step-title">Reviewer Thu Tiền Trực Tiếp</div>
        <p class="hosi-step-desc">Gắn link Zalo/Fanpage của bạn dưới video. Khách chuyển 990k ➔ Giữ lại 300k hoa hồng ngay, chuyển phần còn lại cho Admin cấp Key.</p>
      </div>
      <div class="hosi-step-item">
        <div class="hosi-step-num">MÔ HÌNH 2</div>
        <div class="hosi-step-title">Bán Sỉ Key Cho Thợ Cài Đặt</div>
        <p class="hosi-step-desc">Dành cho thợ setup âm thanh có sẵn khách hàng. Mua sỉ gói 5 - 20 Key với giá ưu đãi (450k - 600k/Key) để tự cài cho khách.</p>
      </div>
      <div class="hosi-step-item">
        <div class="hosi-step-num">MÔ HÌNH 3</div>
        <div class="hosi-step-title">Sub-Keygen Tool Cho Đại Lý</div>
        <p class="hosi-step-desc">Cấp tool sinh key riêng với số lượng tín dụng nạp trước, tự động xuất key cho khách hàng 24/7 không cần chờ đợi.</p>
      </div>
    </div>

    <div style="text-align: center; margin-top: 25px;">
      <a href="{zalo_personal_url}" target="_blank" class="hosi-btn-studio" style="background:#f59e0b; color:#0f172a !important; font-weight:800; font-size:15px; padding:15px 30px;">🚀 ĐĂNG KÝ CỘNG TÁC VIÊN / ĐẠI LÝ TRỰC TIẾP</a>
    </div>
  </div>

  <!-- AI ASSISTANT GUIDE SECTION -->
  <div class="hosi-ai-section">
    <div style="align-items: center; display: flex; flex-wrap: wrap; gap: 24px; justify-content: space-between;">
      <div style="flex: 1 1 500px;">
        <span class="hosi-badge-tag" style="background: rgba(2, 132, 199, 0.12); color: #0284c7;">
          🤖 TRỢ LÝ AI TRỰC TUYẾN 24/7
        </span>
        <h2 style="color: #0f172a; font-family: 'Oswald', sans-serif; font-size: 26px; margin: 4px 0 10px 0; text-transform: uppercase;">
          Trợ Lý AI Hướng Dẫn Sử Dụng Trực Tuyến
        </h2>
        <p style="color: #475569; font-size: 14.5px; line-height: 1.7; margin: 0 0 20px 0;">
          Bạn cần giải đáp thắc mắc về cách kết nối OBS Studio, cài đặt Rack VST3, tinh chỉnh Auto-Tune hay cấu hình Vocal DSP? Hãy trò chuyện ngay với <strong>Trợ Lý AI Hosi</strong> – được huấn luyện chuyên sâu để hỗ trợ bạn từng bước hoàn toàn miễn phí!
        </p>
        <div style="display: flex; flex-wrap: wrap; gap: 12px;">
          <a class="hosi-btn-ai" href="https://byvn.net/hosiguide" target="_blank">
            💬 Mở Trợ Lý AI Hướng Dẫn (byvn.net/hosiguide) ↗
          </a>
          <a class="hosi-btn-github" href="{github_repo_url}" target="_blank">
            ⭐ Xem Dự Án Trên GitHub ↗
          </a>
        </div>
      </div>
      <div style="flex: 0 0 auto; margin: 0 auto; text-align: center;">
        <div style="background: #ffffff; border-radius: 16px; border: 2px dashed #0284c7; box-shadow: 0 4px 15px rgba(2, 132, 199, 0.1); padding: 22px 28px;">
          <div style="font-size: 45px; margin-bottom: 6px;">🤖</div>
          <div style="color: #0369a1; font-family: 'Oswald', sans-serif; font-size: 16px; font-weight: 600; text-transform: uppercase;">AI Guide 24/7</div>
          <div style="color: #64748b; font-size: 12px; margin-top: 4px;">Hỗ trợ cài đặt &amp; sử dụng</div>
        </div>
      </div>
    </div>
  </div>

  <!-- DOWNLOAD SECTION -->
  <div class="hosi-download-section" id="hosi-dl-box">
    <span class="hosi-badge-tag">TẢI VỀ MIỄN PHÍ 100% • GITHUB TỐC ĐỘ CAO</span>
    <h2 style="color: #111111; font-family: 'Oswald', sans-serif; font-size: 26px; margin: 6px 0 10px 0; text-transform: uppercase;">
      Download LiveStream Micro-DAW (v2.0.5)
    </h2>
    <p style="color: #666666; font-size: 14.5px; margin: 0 auto 20px auto; max-width: 680px;">
      Hỗ trợ đầy đủ Windows 10/11 &amp; macOS. Đã tích hợp sẵn file hướng dẫn sử dụng (README) chi tiết bên trong:
    </p>

    <div class="hosi-dl-grid">
      <!-- Setup (Windows) -->
      <div class="hosi-dl-card" style="border-top: 4px solid rgb(255, 25, 25);">
        <div style="font-size: 38px; margin-bottom: 8px;">📦</div>
        <h3 style="color: #111111; font-family: 'Oswald', sans-serif; font-size: 20px; margin: 0 0 6px 0; text-transform: uppercase;">
          Windows Setup ⭐
        </h3>
        <p style="color: #666666; font-size: 13.5px; margin: 0 0 16px 0; min-height: 42px;">
          Tự động cài Plugin OBS Receiver vào hệ thống, tạo icon Desktop và đính kèm đầy đủ tài liệu hướng dẫn.
        </p>
        <a class="hosi-btn-studio" href="{github_setup_zip_url}" style="justify-content: center; margin-bottom: 10px; width: 100%;" target="_blank">
          📥 Tải Trọn Gói Setup (.zip)
        </a>
        <div style="color: #64748b; font-size: 12.5px; margin-top: 6px;">
          Hoặc tải trực tiếp: <a href="{github_setup_exe_url}" style="color: #ff1919; font-weight: 700; text-decoration: none;" target="_blank">LiveStream_Micro_DAW_Setup.exe</a>
        </div>
        <div style="color: #888888; font-size: 12px; margin-top: 8px;">Windows 10/11 (64-bit) • ~6.26 MB</div>
      </div>

      <!-- Portable (Windows) -->
      <div class="hosi-dl-card" style="border-top: 4px solid #444;">
        <div style="font-size: 38px; margin-bottom: 8px;">🚀</div>
        <h3 style="color: #111111; font-family: 'Oswald', sans-serif; font-size: 20px; margin: 0 0 6px 0; text-transform: uppercase;">
          Windows Portable
        </h3>
        <p style="color: #666666; font-size: 13.5px; margin: 0 0 16px 0; min-height: 42px;">
          Chạy ngay không cần cài đặt, tiện lợi sao chép vào USB mang đi phòng thu khác, đính kèm đầy đủ tài liệu.
        </p>
        <a class="hosi-btn-portfolio" href="{github_portable_zip_url}" style="background: #24292f; border-color: #24292f; color: #fff !important; justify-content: center; margin-bottom: 10px; width: 100%;" target="_blank">
          🚀 Tải Bản Portable (.zip)
        </a>
        <div style="color: #64748b; font-size: 12.5px; margin-top: 6px;">
          Hoặc tải bản nén: <a href="{github_portable_rar_url}" style="color: #ff1919; font-weight: 700; text-decoration: none;" target="_blank">Portable (.rar)</a>
        </div>
        <div style="color: #888888; font-size: 12px; margin-top: 8px;">Windows 10/11 (64-bit) • ~6.68 MB</div>
      </div>

      <!-- macOS Universal -->
      <div class="hosi-dl-card" style="border-top: 4px solid #0284c7;">
        <div style="font-size: 38px; margin-bottom: 8px;">🍏</div>
        <h3 style="color: #111111; font-family: 'Oswald', sans-serif; font-size: 20px; margin: 0 0 6px 0; text-transform: uppercase;">
          macOS Universal
        </h3>
        <p style="color: #666666; font-size: 13.5px; margin: 0 0 16px 0; min-height: 42px;">
          Hỗ trợ trọn vẹn cả chip <strong>Apple Silicon (M1/M2/M3/M4)</strong> và <strong>Intel</strong>. Tích hợp sẵn CoreAudio &amp; OBS Plugin (AU/VST3).
        </p>
        <a class="hosi-btn-ai" href="{github_macos_url}" style="justify-content: center; margin-bottom: 10px; width: 100%;" target="_blank">
          🍏 Tải Bản macOS (.app / .zip)
        </a>
        <div style="color: #64748b; font-size: 12.5px; margin-top: 6px;">
          Tương thích macOS 10.15+ (Catalina ➔ Sequoia)
        </div>
        <div style="color: #888888; font-size: 12px; margin-top: 8px;">v2.0.5 • ~26 MB • Universal Binary</div>
      </div>
    </div>

    <!-- GitHub Direct Link Banner -->
    <div style="align-items: center; background: #ffffff; border-radius: 10px; border: 1px solid #e2e8f0; display: flex; flex-wrap: wrap; gap: 12px; justify-content: space-between; margin-top: 22px; padding: 14px 20px;">
      <div style="text-align: left;">
        <strong style="color: #111111; font-size: 14px;">⭐ Dự án chính thức trên GitHub:</strong>
        <div style="color: #64748b; font-size: 12.5px;">Theo dõi dự án, tải bản phát hành &amp; cập nhật phiên bản mới nhất</div>
      </div>
      <a class="hosi-btn-github" href="{github_repo_url}" style="font-size: 13px; padding: 8px 18px;" target="_blank">
        Xem GitHub Repo ↗
      </a>
    </div>
  </div>

  <!-- DONATE SECTION -->
  <div class="hosi-donate-section" id="hosi-donate-box">
    <div style="text-align: center;">
      <span class="hosi-badge-tag">💖 ỦNG HỘ PHÁT TRIỂN / DONATE</span>
      <h2 style="color: #111111; font-family: 'Oswald', sans-serif; font-size: 26px; margin: 6px 0 10px 0; text-transform: uppercase;">
        Đồng Hành Cùng Tác Giả
      </h2>
      <p style="color: #666666; font-size: 14.5px; margin: 0 auto; max-width: 700px;">
        Sự ủng hộ tự nguyện hoặc mua key bản quyền của bạn là nguồn động lực to lớn giúp tác giả duy trì, nâng cấp tính năng mới và chia sẻ phần mềm miễn phí cho cộng đồng!
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
        <img alt="QR Donate" class="hosi-qr-img" id="blogger-qr-img" src="{vietqr_b64}" />
      </div>

      <div>
        <div class="hosi-info-row">
          <div>
            <div style="color: #888888; font-size: 11px; font-weight: 700; text-transform: uppercase;">CHỦ TÀI KHOẢN</div>
            <div id="blogger-qr-name" style="color: #111111; font-family: 'Oswald', sans-serif; font-size: 18px; font-weight: 600;">LA CHÍ NHÂN</div>
          </div>
        </div>

        <div class="hosi-info-row">
          <div>
            <div id="blogger-qr-acc-lbl" style="color: #888888; font-size: 11px; font-weight: 700; text-transform: uppercase;">SỐ TÀI KHOẢN</div>
            <div id="blogger-qr-acc-val" style="color: #ff1919; font-family: 'JetBrains Mono', monospace; font-size: 19px; font-weight: 700;">0908107000</div>
          </div>
          <button class="hosi-copy-btn" onclick="bloggerCopyText('0908107000')">Sao Chép</button>
        </div>

        <div class="hosi-info-row">
          <div>
            <div style="color: #888888; font-size: 11px; font-weight: 700; text-transform: uppercase;">NGÂN HÀNG / VÍ</div>
            <div id="blogger-qr-bank" style="color: #333333; font-size: 14.5px; font-weight: 600;">MB Bank (Ngân hàng Quân Đội)</div>
          </div>
        </div>

        <div class="hosi-info-row">
          <div>
            <div style="color: #888888; font-size: 11px; font-weight: 700; text-transform: uppercase;">NỘI DUNG CHUYỂN KHOẢN</div>
            <div style="color: #444444; font-size: 14px;">Ung ho LiveStream Micro-DAW</div>
          </div>
          <button class="hosi-copy-btn" onclick="bloggerCopyText('Ung ho LiveStream Micro-DAW')">Sao Chép</button>
        </div>

        <div id="blogger-toast-copy" style="color: #16a34a; display: none; font-size: 13.5px; font-weight: 700; margin-top: 8px;">
          ✓ Đã sao chép vào bộ nhớ tạm!
        </div>
      </div>
    </div>

    <!-- Dịch vụ Setup chuyên nghiệp -->
    <div class="hosi-quote-box" style="margin-top: 25px;">
      <strong style="color: #ff1919; text-transform: uppercase;">🛠️ Dịch vụ Setup Âm Thanh Livestream &amp; Cài Vocal Chain Chuyên Nghiệp:</strong><br />
      • Cài đặt trọn bộ Plugin VST3 hay nhất (Auto-Tune, FabFilter, Reverb, Delay) qua UltraViewer / TeamViewer.<br />
      • Tinh chỉnh âm thanh chuẩn phòng thu theo từng chất giọng và micro của bạn.<br />
      👉 <strong>Hotline / Zalo: 0908.107.000 (La Chí Nhân)</strong> • Tham gia nhóm Zalo Dùng Thử 7 Ngày: <strong><a href="{zalo_trial_group_url}" style="color: #ff1919; text-decoration: none;" target="_blank">Nhóm Zalo Nhận Key 7 Ngày</a></strong> • Website: <strong><a href="https://lachinhan.xyz" style="color: #ff1919; text-decoration: none;" target="_blank">lachinhan.xyz</a></strong>
    </div>
  </div>

  <div style="color: #888888; font-size: 13px; padding: 20px 0; text-align: center;">
    © 2026 LiveStream Micro-DAW v2.0.5 • Phát triển bởi <strong>La Chí Nhân</strong> (<a href="https://lachinhan.xyz" style="color: #444444; text-decoration: none;" target="_blank">lachinhan.xyz</a>) • GitHub: <strong><a href="{github_repo_url}" style="color: #ff1919; text-decoration: none;" target="_blank">github.com/lachinhan/Hosi-Micro-Daw</a></strong>
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
      name: "LA CHÍ NHÂN",
      accLabel: "TÀI KHOẢN PAYPAL",
      accVal: "lachinhan@gmail.com",
      bank: "PayPal Quốc Tế (lachinhan@gmail.com)"
    }},
    mbcard: {{
      src: "{mb_b64}",
      name: "LA CHÍ NHÂN",
      accLabel: "SỐ THẺ MB BANK",
      accVal: "0908107000",
      bank: "MB Bank (Ngân hàng Quân Đội)"
    }}
  }};

  function bloggerSwitchQr(type, btn) {{
    const data = bloggerQrData[type];
    if (!data) return;

    const img = document.getElementById('blogger-qr-img');
    const name = document.getElementById('blogger-qr-name');
    const accLbl = document.getElementById('blogger-qr-acc-lbl');
    const accVal = document.getElementById('blogger-qr-acc-val');
    const bank = document.getElementById('blogger-qr-bank');

    if (img && data.src) img.src = data.src;
    if (name) name.innerText = data.name;
    if (accLbl) accLbl.innerText = data.accLabel;
    if (accVal) accVal.innerText = data.accVal;
    if (bank) bank.innerText = data.bank;

    const allBtns = document.querySelectorAll('#micro-daw-root .hosi-tab-btn');
    allBtns.forEach(b => b.classList.remove('active'));
    if (btn) btn.classList.add('active');
  }}

  function bloggerCopyText(text) {{
    if (navigator.clipboard) {{
      navigator.clipboard.writeText(text).then(() => {{
        const toast = document.getElementById('blogger-toast-copy');
        if (toast) {{
          toast.style.display = 'block';
          setTimeout(() => {{ toast.style.display = 'none'; }}, 2500);
        }}
      }}).catch(err => {{
        fallbackCopy(text);
      }});
    }} else {{
      fallbackCopy(text);
    }}
  }}

  function fallbackCopy(text) {{
    const ta = document.createElement('textarea');
    ta.value = text;
    document.body.appendChild(ta);
    ta.select();
    document.execCommand('copy');
    document.body.removeChild(ta);
    const toast = document.getElementById('blogger-toast-copy');
    if (toast) {{
      toast.style.display = 'block';
      setTimeout(() => {{ toast.style.display = 'none'; }}, 2500);
    }}
  }}
</script>
"""

output_path = os.path.join(workspace, "blogger_embed.html")
with open(output_path, "w", encoding="utf-8") as f:
    f.write(blogger_html)

print(f"[SUCCESS] Generated blogger_embed.html successfully at: {output_path}")
