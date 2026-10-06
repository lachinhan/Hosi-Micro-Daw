# 🎛️ LiveStream Micro-DAW & OBS Audio Receiver (VST2 / VST3)

[🇻🇳 Tiếng Việt](README.md) | [🇬🇧 English](README_EN.md)

Một hệ thống **Micro-DAW chuyên dụng cho Livestream & Hát Live** siêu nhẹ, độ trễ cực thấp (< 5ms), được xây dựng trên nền tảng **C++20** và **JUCE 8**. 

Ứng dụng cho phép bạn cắm trực tiếp Microphone và các Plugin VST3 chuyên nghiệp (Auto-Tune, Pro-Q3, CLA-76, Pro-R...) để xử lý giọng hát trong thời gian thực, đồng thời truyền thẳng luồng âm thanh đã qua xử lý sang **OBS Studio** thông qua bộ nhớ chia sẻ không khóa (Lock-Free Shared Memory IPC) với độ trễ bằng 0 mà **không cần cài đặt phần mềm Virtual Cable (VB-Cable, Voicemeeter)**.

---

## 📑 Mục Lục (Table of Contents)
1. [🌟 Tính Năng Nổi Bật (Key Features)](#-tính-năng-nổi-bật-key-features)
2. [📦 Cài Đặt & Triển Khai (Installation)](#-cài-đặt--triển-khai-installation)
3. [🎙️ Hướng Dẫn Cấu Hình OBS Studio (Bắt Buộc)](#️-hướng-dẫn-cấu-hình-obs-studio-bắt-buộc)
4. [🎧 Hướng Dẫn Sử Dụng LiveStream Micro-DAW](#-hướng-dẫn-sử-dụng-livestream-micro-daw)
5. [❓ Xử Lý Sự Cố Thường Gặp (Troubleshooting / FAQ)](#-xử-lý-sự-cố-thường-gặp-troubleshooting--faq)
6. [🛠️ Hướng Dẫn Build Dự Án (Developer Guide)](#️-hướng-dẫn-build-dự-án-developer-guide)

---

## 🌟 Tính Năng Nổi Bật (Key Features)

- **🎵 Tự Động Dò Tone Nhạc Beat & Micro (Auto Key & Scale Detection)**: Tích hợp thuật toán phân tích phổ âm Harmonic Chromagram kết hợp tương quan Krumhansl-Schmuckler để tự động nhận diện chính xác Tone (C Major, F# Minor...) của bài hát/beat karaoke hoặc giọng hát mộc từ Micro theo thời gian thực.
- **⚡ Tự Động Nạp Tone Vào Auto-Tune (Auto-Push on Lock - Nút `⚡ AUTO`)**:
  - **Chế độ Tự Động (`⚡ AUTO`)**: Khi nhạc beat chạy và thuật toán chốt Tone chính xác ($\ge 75\%$ confidence & `isLocked`), hệ thống **tự động truyền thẳng Key & Scale vào Auto-Tune Pro trong Rack** mà streamer không cần rời tay bấm chuột!
  - **Chế độ Thủ Công (`AUTO: TẮT`)**: Dành cho người dùng muốn chủ động kiểm tra tone trước khi nạp qua nút **`ĐỒNG BỘ AUTO-TUNE`**.
- **🤖 Trợ Lý AI Hướng Dẫn Sử Dụng Trực Tuyến 24/7**: Tích hợp trợ lý AI chuyên sâu tại [byvn.net/hosiguide](https://byvn.net/hosiguide) sẵn sàng giải đáp và hướng dẫn từng bước cài đặt VST3, OBS Studio và căn chỉnh Vocal DSP.
- **🎛️ Nút Chọn Tone Thủ Công (24 Cung Giọng Major/Minor)**: Menu trực quan cho phép chọn nhanh 24 cung giọng (Trưởng / Thứ) bất kỳ và truyền lập tức vào Auto-Tune chỉ với 1 cú click.
- **⚡ 1-Click Đồng Bộ Tone Vào Auto-Tune**: Tự động dò tìm và gán chuẩn xác nốt Root & Scale vừa phát hiện vào plugin Auto-Tune / Pitch Correction trong Rack, giúp Streamer không cần am hiểu nhạc lý vẫn hát đúng tone 100%.
- **🎶 Trình Phát Beat Chuẩn DAW Với Vector Icon Chuyên Nghiệp**:
  - Hỗ trợ các định dạng MP3, WAV, FLAC, OGG, AIFF.
  - Bộ nút điều khiển Vector Icon sắc nét: **`▶` Play / `❚❚` Pause**, **`■` Stop**, **`⟳` Loop**, thanh trượt Seek bar, thanh Volume và đồng hồ đếm thời gian.
  - Phổ âm 12 nốt Chroma LED sinh động trực quan.
- **🎙️ Tự Động Hạ Nhỏ Nhạc Nền Beat Khi Nói (Smart Auto-Ducking / Radio Talk-over Pro)**:
  - **Click trái 1 chạm**: Bật / Tắt chế độ Ducking thông minh tức thì (`DUCK: OFF` $\leftrightarrow$ `DUCK -12dB`).
  - **Menu ngữ cảnh chuột phải**: Cho phép tùy biến sâu các thông số chuyên nghiệp:
    - **Mức giảm âm lượng (Depth)**: `-6 dB` (Hát đệm), `-10 dB` (Cân bằng), `-12 dB` (Tiêu chuẩn MC / Talkshow ⭐), `-15 dB` (Giao lưu), `-20 dB` (Chuyên Radio / Truyện), `-30 dB` (Hạ gần tắt).
    - **Độ nhạy Micro (Sensitivity)**: `-42 dB` (Nói nhỏ, thì thầm), `-36 dB` (Tiêu chuẩn Studio ⭐), `-28 dB` (Phòng ồn / Nói to).
    - **Thời gian giữ tiếng (Hold Time)**: `300 ms`, `500 ms` ⭐, `800 ms` (giữ nhạc êm ái khi streamer ngừng nói nghỉ câu).
  - Thuật toán Sidechain DSP thời gian thực mượt mà (Smooth Ramp 40ms) loại bỏ hoàn toàn hiện tượng méo tiếng hoặc giật âm.
  - Tự động lưu và khôi phục toàn bộ thiết lập Ducking vào Session/Preset.
- **🎵 Sổ Tone Bài Hát Thông Minh (Songbook & Auto-Key Manager)**:
  - **Kho dữ liệu bài hát Việt Nam khổng lồ**: Tích hợp sẵn hơn **1.033+ bài hát** đầy đủ thể loại (Nhạc Trẻ, Ballad, Bolero, Nhạc Trịnh, Quê Hương, Tiền Chiến, Làn Sóng Xanh) kèm đầy đủ Tone gốc, Tone Nam, Tone Nữ, Tác giả và Điệu nhạc.
  - **❤️ Danh Sách Bài Hát Yêu Thích**: Biểu tượng trái tim `❤️` / `🤍` trực quan trên từng dòng danh sách, bấm 1 chạm để thêm/bỏ bài hát yêu thích và lọc nhanh tức thì trong menu thể loại.
  - **Tìm kiếm tức thì (Instant Search)**: Hỗ trợ tìm tiếng Việt có dấu hoặc không dấu (vd: gõ `hoa no`, `cat doi`, `hoai lam`... là ra ngay).
  - **Bộ lọc đa dạng**: Lọc theo Thể loại (`🌟 Tất Cả Bài Hát`, `❤️ Bài Hát Yêu Thích`, `🔥 Nhạc Trẻ / Pop`, `🎸 Bolero / Nhạc Vàng`, `☕ Nhạc Trịnh`, `🎼 Trữ Tình / Ballad`, `⭐ Bài Tự Thêm`).
  - **1-Click Đồng Bộ Auto-Tune**:
    - Chọn nhanh `[ 👨 TONE NAM ]`, `[ 👩 TONE NỮ ]` hoặc `[ 🎵 TONE GỐC ]`.
    - Tăng/giảm nửa cung Transpose (`-1`, `+1`) mượt mà.
    - Nút **`⚡ ĐỒNG BỘ VÀO AUTO-TUNE`** tự động nạp Key & Scale thẳng vào plugin Auto-Tune trong Rack!
  - **Cá nhân hóa & Sao lưu**: Nút **`💾 Lưu Tone Của Tôi`**, **`❤️ Yêu Thích`**, **`➕ Thêm Bài`**, **`📥 Nhập JSON`**, **`📤 Xuất JSON`** để chia sẻ danh sách bài hát giữa các máy.
- **🎛️ Bàn Phím Hiệu Ứng Âm Thanh Soundboard (Tùy Biến Đổi Tên & Nạp Âm Thanh)**:
  - Tích hợp 8 nút Pad hiệu ứng âm thanh (Vỗ tay 👏, Cười ồ 😂, Hồi hộp 🥁, Chuông ting 🔔, Kèn Airhorn 🎺, Tiếng nổ 💥, Buzz lỗi ❌, Reo hò 🎉).
  - **Đổi tên nút dễ dàng**: **Click đúp chuột** hoặc **Click chuột phải** $\to$ `✏ Đổi tên nút (Rename)...`.
  - **Menu ngữ cảnh chuột phải**: Hỗ trợ đổi tên, nạp file âm thanh riêng (`.mp3`, `.wav`) và khôi phục âm thanh gốc (`↺ Reset`).
  - Hỗ trợ phím tắt Numpad 1..8 / 1..8, nút Stop All (Esc) và tự động lưu cấu hình Pad vào Session/Preset.
- **📊 Master Output Channel Strip Cân Đối Chuẩn Studio**:
  - Gộp Dual Stereo VU Meter (L & R), Thước đo dB Logarithmic (`+6`, `0`, `-6`, `-12`, `-24`, `-inf`) và Master Fader vào khối trung tâm cân đối.
  - Đèn LED cảnh báo Clip riêng biệt cho 2 kênh và nút **`0 dB RESET`** tiện lợi.
- **Độ trễ siêu thấp (Zero-Latency IPC)**: Sử dụng Windows Named Shared Memory (`Local\LiveStreamMicroDAW_AudioIPC_v1`) và cấu trúc Lock-Free Ring Buffer 16.384 sample, SPSC `acquire`/`release` memory barrier.
- **Tự Động Đồng Bộ Tần Số Mẫu (PLL Resampling)**: Tự động chuyển đổi tần số mẫu theo thời gian thực (ví dụ DAW chạy 44.1 kHz, OBS chạy 48.0 kHz) bằng thuật toán Fractional Linear Resampler kết hợp bộ bám xung Pha (PLL), loại bỏ 100% tiếng nổ rẹt và hiện tượng tràn/cạn bộ đệm (Buffer Underflow/Overflow).
- **Tương Thích Mọi Soundcard ASIO**: Tự động nhận diện cặp kênh Input đang hoạt động (kênh 1-2, 3-4...) trên các soundcard chuyên nghiệp như Roland QUAD-CAPTURE, Focusrite Scarlett, Behringer, Yamaha Steinberg...
- **VST3 Host Chuyên Nghiệp**: Rack 8 slot linh hoạt, hỗ trợ mở cửa sổ Native GUI gốc của các hãng nổi tiếng (Antares Auto-Tune, FabFilter, Waves, Soundtoys...).
- **Chuyển Scene 1 Chạm & Phím Tắt Toàn Cục (F1, F2, F3)**: 
  - 🎤 **`LIVE (F1)` - HÁT LIVE**: Bật trọn bộ Vocal Chain chất lượng cao (Auto-Tune + EQ + Comp + Reverb/Delay mịn).
  - 💬 **`TALK (F2)` - GIAO LƯU**: Tự động bypass Auto-Tune & Reverb trong 1/1000s, giữ lại Noise Gate + Compressor giúp giọng nói mộc ấm áp, rõ ràng, không bị vang vọng khó chịu khi tâm sự với khán giả.
  - 🔥 **`TUNE (F3)` - AUTOTUNE**: Bật chế độ ép Tune sôi động cho nhạc Trap, Remix, Vinahouse.
- **💖 Tích Hợp Mã QR Donate & Hỗ Trợ Tác Giả**: Tích hợp nút **`☕ DONATE`** trên thanh tiêu đề mở hộp thoại mã QR MoMo, VietQR MBBank sắc nét kèm tính năng 1-click sao chép Số tài khoản và Nội dung chuyển khoản nhanh chóng.
- **Tùy Chỉnh Tỷ Lệ Giao Diện (UI Zoom Scaling)**: Hỗ trợ 3 kích thước hiển thị: **85% Nhỏ gọn (Compact)**, **100% Tiêu chuẩn (Standard)**, và **125% Rộng rãi (Large)**, tự động căn giữa màn hình.

---

## 📦 Cài Đặt 1-Click & Triển Khai (Installation)

### 🌟 Cách 1: Sử Dụng Bộ Cài Đặt Tự Động 1-Click (Khuyên Dùng)
Chạy file cài đặt duy nhất:
```text
LiveStream_Micro_DAW_Setup.exe
```
- **Tự động hóa hoàn toàn**:
  - Cài đặt phần mềm **LiveStream Micro-DAW** vào máy tính và tạo icon tiện lợi trên **Desktop** & **Start Menu**.
  - **Tự động nhận diện và sao chép Plugin OBS Receiver (cả VST2 `.dll` và VST3 `.vst3`)** vào đúng thư mục hệ thống của OBS Studio (`C:\Program Files\Common Files\VST3` và `C:\Program Files\VstPlugins`).
  - Người dùng **không cần phải tìm thư mục VST hay copy thủ công bằng tay!**

### 📁 Cách 2: Chạy Bản Portable Trực Tiếp (Không Cần Cài Đặt)
1. **Chạy phần mềm**: Nhấp đúp chạy trực tiếp `LiveStream Micro-DAW.exe`.
2. **Cài nhanh Plugin vào OBS (1-Click)**:
   - Nhấp đúp chuột vào file **`Install_OBS_Plugin.bat`** (hoặc chuột phải chọn *Run as administrator*).
   - Script sẽ **tự động sao chép trọn bộ VST2 (.dll) và VST3 (.vst3)** vào đúng các thư mục VST chuẩn của hệ thống (`C:\Program Files\Common Files\VST3\` và `C:\Program Files\VstPlugins\`).
   - *(Tùy chọn)* Nếu muốn gỡ plugin, chạy file `Uninstall_OBS_Plugin.bat`.

---

## 🎙️ Hướng Dẫn Cấu Hình OBS Studio (Bắt Buộc)

Để livestream hoặc thu âm chất lượng cao không bị trễ tiếng và không bị tiếng vọng lặp lại, hãy làm theo các bước sau:

```
[Microphone] ──► [LiveStream Micro-DAW]
                       │
                       ├───► [Tai nghe qua Soundcard ASIO] (Bạn nghe 0-latency trực tiếp)
                       │
                       └───► [Shared Memory IPC] ──► [OBS: LiveStream Receiver] ──► [Khán giả Livestream]
```

### Bước 1: Thêm Plugin vào OBS Studio
1. Mở **OBS Studio**.
2. Tại khung **Bộ trộn âm thanh (Audio Mixer)**, tìm nguồn mic (ví dụ **Mic/Aux** hoặc tạo mới một **Âm thanh đầu vào (Audio Input Capture)**).
3. Nhấp chuột phải vào nguồn đó $\to$ chọn **Bộ lọc (Filters)**.
4. Nhấn dấu **`+`** ở góc dưới $\to$ chọn **Bộ lọc VST 2.x/3.x (VST 2.x/3.x Plug-in)**.
5. Tại menu danh sách thả xuống, chọn **LiveStream OBS Receiver**.
6. Nhấp vào nút **Mở giao diện phần bổ trợ (Open Plug-in Interface)** $\to$ Bạn sẽ thấy đèn báo **`🟢 CONNECTED TO MICRO-DAW (Active)`** và thanh VU Meter nhảy đều theo giọng nói.

### Bước 2: CẤU HÌNH BẮT BUỘC TRÁNH TIẾNG VỌNG (Echo) ⚠️
1. Trong OBS Studio, nhấp chuột phải vào bất kỳ thanh âm lượng nào trong Audio Mixer $\to$ chọn **Thuộc tính âm thanh nâng cao (Advanced Audio Properties)**.
2. Tìm đến dòng nguồn mic vừa thêm plugin (ví dụ `Mic/Aux`).
3. Tại cột **Giám sát âm thanh (Audio Monitoring)**:
   👉 **BẮT BUỘC CHỌN: `Tắt giám sát` (Monitor Off)**.

> [!IMPORTANT]
> **Tại sao phải "Tắt giám sát"?**
> - Bạn đã nghe tiếng hát thời gian thực (0-delay) trực tiếp từ **LiveStream Micro-DAW** qua tai nghe cắm ở soundcard.
> - Nếu bật Monitor trên OBS, OBS sẽ phát thêm 1 lần âm thanh trễ của Windows vào tai bạn, gây ra hiện tượng **tiếng vọng (Echo 2 lần)**. Để "Tắt giám sát" đảm bảo chỉ có khán giả trên Live stream nghe thấy âm thanh từ OBS!

---

## 🎧 Hướng Dẫn Sử Dụng LiveStream Micro-DAW

### 1. Cấu hình Thiết bị Âm thanh (Audio Settings)
- Nhấn vào nút **Cài đặt âm thanh (Audio Settings)** ở góc trên bên phải giao diện DAW.
- **Loại thiết bị (Audio Device Type)**: Khuyên dùng **ASIO** (chọn driver soundcard của bạn, ví dụ *QUAD-CAPTURE, Focusrite USB ASIO, ASIO4ALL v2...*).
- **Kích thước bộ đệm (Buffer Size)**: Chọn **128 samples** hoặc **256 samples** để đạt độ trễ nhỏ nhất.
- **Kênh vào (Active Inputs)**: Bật kênh micro của bạn (kênh 1 hoặc 2).

### 2. Quét và Thêm Plugin VST3
- Nhấn nút **Quét Plugin VST3 (Scan VST3)** để DAW tự động phát hiện tất cả các plugin VST3 cài trên máy tính (`C:\Program Files\Common Files\VST3`).
- Tại 8 slot rack của DAW:
  - Nhấp vào menu slot để chọn Plugin (ví dụ Slot 1: Auto-Tune, Slot 2: Noise Gate, Slot 3: Pro-Q 3, Slot 4: CLA-76, Slot 5: Pro-R...).
  - **Nhấp đúp chuột (Double-Click)** vào slot để mở bảng điều khiển giao diện gốc (GUI) của Plugin.
  - Bật/tắt nút **Bypass** trên từng slot khi cần thiết.

### 3. Dò Tone Nhạc Beat & Tự Động Đồng Bộ Auto-Tune 🎵
- **Nạp beat**: Nhấn nút **NẠP BEAT** $\to$ Chọn file nhạc karaoke (`.mp3`, `.wav`, `.flac`, `.ogg`). DAW sẽ tự động quét tone trong 1 giây và hiển thị kết quả (ví dụ: `F# Minor`).
- **Phát nhạc**: Dùng các nút **`▶` Play / `❚❚` Pause**, **`■` Stop**, **`⟳` Loop** và thanh trượt Seek bar để điều khiển bài hát. Âm lượng Beat có thể tinh chỉnh bằng thanh **VOL**.
- **Chuyển nguồn dò Tone**: Bấm nút **`NGUỒN: BEAT`** / **`NGUỒN: MIC LIVE`** để linh hoạt dò tone từ file nhạc beat hoặc từ giọng hát thực tế qua micro.
- **Chọn Tone thủ công**: Bấm nút **`CHỌN TONE`** $\to$ Chọn nhanh bất kỳ giọng nào trong danh sách 24 giọng Major/Minor để gán trực tiếp vào Auto-Tune.
- **Đồng bộ vào Auto-Tune**: Nhấn nút **`ĐỒNG BỘ AUTO-TUNE`** $\to$ DAW sẽ tự động tìm plugin Auto-Tune trong Rack và gán chính xác Tone + Scale vừa dò được.

### 4. Tự Động Hạ Nhạc Nền Khi Nói (Smart Voice Ducking) 📉
- **Ý tưởng**: Chuẩn Radio / MC chuyên nghiệp.
- **Cách sử dụng**: Bấm nút **`DUCKING: OFF`** trên thanh Beat Player để chuyển sang **`DUCKING: ON`** (nút sáng xanh).
- **Cơ chế hoạt động**:
  - Khi Streamer bắt đầu cất giọng nói chuyện với khán giả $\to$ Nhạc nền tự động giảm âm lượng 6 dB cực kỳ mượt mà.
  - Khi ngừng nói hoặc bắt đầu hát theo beat $\to$ Nhạc nền tự động nâng âm lượng trở lại mức ban đầu (không bị giật cục/pumping).

### 5. Bộ Hiệu Ứng Studio Tích Hợp Sẵn (Built-in DSP Vocal Suite) 💎
- **Ý tưởng**: Không bắt buộc phải cài đặt hay crack các bộ plugin VST ngoài nặng nề (Waves, FabFilter, iZotope...). Micro-DAW đã tích hợp sẵn một bộ xử lý âm thanh Studio nội bộ siêu nhẹ, độ trễ bằng 0 (0ms latency):
  1. **Noise Gate**: Tự động khử tiếng xì nền, tiếng quạt máy tính, tiếng gõ phím khi không nói.
  2. **Studio EQ 3-Band**: Tinh chỉnh dải trầm (Low Shelf 120Hz), tăng độ dày/sáng giọng (Mid 2.6kHz), và tăng dải khí trong trẻo (High Air 9.5kHz).
  3. **Warm Compressor**: Nén dải động, bù âm lượng (Makeup Gain), tạo độ ấm áp mượt mà cho giọng hát.
  4. **Lush Reverb**: Không gian vang mềm mại chuẩn phòng thu (Plate/Hall Reverb).
  5. **Stereo Delay / Echo**: Hiệu ứng vọng Stereo Ping-Pong analog ấm áp cho karaoke và hát live.
  6. **Brickwall Limiter**: Chống vỡ tiếng (Anti-Clipping) bảo vệ tuyệt đối khi ca sĩ hát nốt cao hoặc hét to.
- **Cách sử dụng**: Ở cột bên phải, bấm vào tab **`VOCAL DSP`** $\to$ Bật/tắt các nút `PWR` hoặc chọn nhanh Preset có sẵn (*Hát Live, Streamer Talk, Karaoke Hall, Podcast Clean, Tắt DSP, hoặc Khôi Phục Mặc Định Factory Reset*). Bạn cũng có thể bấm nút **`🔄 KHÔI PHỤC GỐC`** để lập tức đưa mọi nút vặn về thông số chuẩn Studio ban đầu.

### 6. Thu Âm 1 Chạm Chất Lượng Cao (Quick Record / Multi-Track) 🔴
- **Ý tưởng**: Nút REC đặt ngay trên cùng một hàng với tính năng Ducking trên thanh Beat Player.
- **Tính năng**:
  - **Thu âm 1 chạm**: Bấm nút **`● REC`** $\to$ DAW chuyển sang trạng thái nhấp nháy đỏ và đếm giờ thu âm theo thời gian thực (ví dụ `REC 01:25`).
  - **Đa luồng (Multi-Track WAV 24-bit)**: DAW tự động lưu đồng thời 2 file riêng biệt vào thư mục `recordings/`:
    1. `Master_Mix_YYYY-MM-DD_HH-MM-SS.wav`: Toàn bộ buổi live đầy đủ nhạc beat, giọng hát Auto-Tune, FX và soundboard.
    2. `Mic_Dry_YYYY-MM-DD_HH-MM-SS.wav`: Giọng mộc sạch của micro (chưa qua effect) để streamer dễ dàng đưa vào CapCut, Premiere, DaVinci Resolve dựng clip ngắn TikTok, YouTube Shorts, Reels.
  - **Mở nhanh thư mục**: Bấm nút **`DIR`** ngay cạnh nút REC để mở nhanh thư mục chứa các file thu âm.

### 7. Bàn Phím Hiệu Ứng Âm Thanh (Live Soundboard Pads) 🎛️
- **Chuyển sang Soundboard**: Ở cột bên phải, nhấn vào tab **`SOUND FX`** $\to$ Giao diện 8 nút Pad rực rỡ sẽ xuất hiện.
- **Phím tắt nhanh (Hotkeys)**:
  - Bấm các phím **`1` đến `8`** (hoặc bàn phím số **`Numpad 1` đến `Numpad 8`**) để phát âm thanh tức thì khi đang livestream.
  - Bấm phím **`ESC`** (hoặc nút **`STOP ALL`**) để ngắt toàn bộ âm thanh ngay lập tức.
- **Đổi tên nút & Nạp file âm thanh tùy chỉnh**:
  - **Đổi tên nút (Rename)**: **Click đúp chuột** vào nút Pad hoặc **Click chuột phải** $\to$ Chọn **`✏ Đổi tên nút (Rename)...`**.
  - **Nạp file âm thanh riêng**: **Click chuột phải** $\to$ Chọn **`📁 Nạp file âm thanh (.wav, .mp3)...`**.
  - **Khôi phục âm thanh gốc**: **Click chuột phải** $\to$ Chọn **`↺ Khôi phục mặc định (Reset)`**.
  - Hoặc chép trực tiếp các file âm thanh vào thư mục **`sounds/`** cạnh file chạy DAW.

### 8. Lưu và Tải Cấu Hình Dự Án (Presets)
- Nhấn **Save Preset** $\to$ Lưu toàn bộ chuỗi plugin, thông số DSP và effect ra file định dạng `.hmdaw`.
- Nhấn **Load Preset** $\to$ Mở lại cấu hình yêu thích cho từng ca sĩ hoặc từng buổi live chỉ trong 1 giây.

### 9. Phóng To / Thu Nhỏ Giao Diện
- Ở góc trên cùng, bạn có thể bấm chuyển đổi giữa **85%**, **100%**, và **125%** để giao diện hiển thị vừa vặn nhất với độ phân giải màn hình của bạn (Laptop 1080p hay Màn hình rời 2K/4K).

---

## ❓ Xử Lý Sự Cố Thường Gặp (Troubleshooting / FAQ)

### 1. OBS không nhận được tín hiệu âm thanh từ DAW?
- **Kiểm tra DAW**: Đảm bảo thanh Master Output Meter trong Micro-DAW đang nhảy tín hiệu.
- **Kiểm tra OBS Receiver**: Mở giao diện plugin trong OBS, kiểm tra xem trạng thái có phải là `🟢 CONNECTED TO MICRO-DAW (Active)` hay không.
- **Kiểm tra Sample Rate**: Nếu vừa đổi Sample Rate trên soundcard, hãy khởi động lại Micro-DAW để bộ nhớ IPC thiết lập lại chuẩn đồng bộ.

### 2. Âm thanh trong OBS bị nổ lụp bụp hoặc méo tiếng?
- Hãy đảm bảo bạn đang sử dụng phiên bản mới nhất của **LiveStream OBS Receiver** (đã tích hợp bộ Resampler PLL chống trôi xung).
- Đặt Buffer Size của Soundcard trong DAW ở mức hợp lý (khuyên dùng **128** hoặc **256 samples**).

### 3. Tôi nghe thấy giọng mình bị vang 2 lần và bị trễ?
- Bạn chưa tắt Monitor trên OBS! Hãy vào **Advanced Audio Properties** của OBS và chỉnh nguồn mic về **`Monitor Off` (Tắt giám sát)**.

---

## 🛠️ Hướng Dẫn Build Dự Án (Developer Guide)

### Yêu Cầu Môi Trường
- **Windows 10 / 11 64-bit**
- **Visual Studio 2022** (với C++ Desktop Development workload)
- **CMake 3.22+**
- **Steinberg ASIO SDK** (đặt tại `C:/SDKs/asiosdk` hoặc thư mục tùy chỉnh)

### Lệnh Build (PowerShell)

```powershell
# 1. Đi tới thư mục dự án
cd "e:\Hosi Micro Daw"

# 2. Tạo project CMake (Visual Studio 2022 x64)
cmake -B build -G "Visual Studio 17 2022" -A x64 -DASIO_SDK_DIR="C:/SDKs/asiosdk"

# 3. Biên dịch bản Release tối ưu hóa cao
cmake --build build --config Release --parallel
```

### Kết Quả Biên Dịch:
1. `build/LiveStreamMicroDAW_artefacts/Release/LiveStream Micro-DAW.exe` (Ứng dụng DAW độc lập)
2. `build/plugins/receiver/LiveStreamOBSReceiver_artefacts/Release/VST/LiveStream OBS Receiver.dll` (VST2 Plugin)
3. `build/plugins/receiver/LiveStreamOBSReceiver_artefacts/Release/VST3/LiveStream OBS Receiver.vst3` (VST3 Plugin)

---

## 📄 Bản Quyền & Tác Giả
- **Phát triển bởi**: Hosi Studio Engineering Team
- **Công nghệ cốt lõi**: JUCE Framework 8, C++20, Win32 Memory-Mapped Files IPC.

---

## 💖 Ủng Hộ & Quyên Góp (Donate)

Nếu bạn thấy **LiveStream Micro-DAW** hữu ích cho công việc livestream hoặc sản xuất âm thanh, bạn có thể ủng hộ tác giả qua:

- **MB Bank**: `0908107000` (LA CHI NHAN)
- **MoMo**: `0908107000` (LA CHI NHAN)
- **PayPal**: `Nhan La Chi` (Quét mã QR PayPal trực tiếp trên popup của ứng dụng)
- **MB Card**: Quét mã thẻ thanh toán quốc tế / nội địa trực tiếp trong ứng dụng

