# 🚀 CHIẾN LƯỢC CHUYỂN ĐỔI THU PHÍ (MONETIZATION) & TÍNH NĂNG AI THÔNG MINH
**Dự án:** LiveStream Micro-DAW (Windows & macOS)  
**Tác giả:** La Chí Nhân  
**Phiên bản tài liệu:** 2.1 (Bổ sung Smart BPM-Synced Delay/Reverb - Lưu nội bộ - Không commit Git)

---

## 📌 PHẦN 1: BÀI TOÁN CHUYỂN ĐỔI KHI ĐÃ CÓ HÀNG NGÀN USER DÙNG BẢN FREE FULL

### 1. Thực trạng & Sai lầm cần tránh
- **Thực trạng**: Hàng ngàn người dùng đã tải và cài đặt bản **v2.0.0 (Free Full tính năng offline)**. Họ đã có bộ effect, Auto-Tune, Sổ Tone, OBS receiver.
- ❌ **Sai lầm chết người**: Dùng cập nhật từ xa để khóa hoặc cắt bớt tính năng của bản cũ. Điều này sẽ lập tức gây làn sóng phẫn nộ, đánh giá xấu và người dùng sẽ tìm cách giữ lại file cài cũ hoặc bẻ khóa.
-  **Chiến lược thông minh (Freemium Value-Add)**:
  - Giữ nguyên bản **v2.0 (Community Edition)** hoàn toàn **MIỄN PHÍ VĨNH VIỄN** làm "mồi câu" (Lead Magnet) và tạo dựng niềm tin thương hiệu.
  - Khiến người dùng **tự nguyện nâng cấp lên bản PRO (v3.0)** vì bản mới có những **tính năng AI & Tiện ích Studio đột phá** mà bản cũ hoàn toàn không thể làm được.

---

## 💎 PHẦN 2: CÁC TÍNH NĂNG AI & TIỆN ÍCH ĐỘT PHÁ TRÊN BẢN PRO (KILLER FEATURES)

```
+-------------------------------------------------------------------------------+
|                      BỘ NÃO AI & TIỆN ÍCH PRO (v3.0)                          |
+---------------------------------------+---------------------------------------+
|  1. AI VOCAL PROFILER & AUTO EQ       |  2. SMART VOCAL RANGE & SONG MATCH    |
|  • Thu 5s mẫu giọng người dùng        |  • Nhận diện dải nốt (A2 -> G4)       |
|  • Phân tích Formant, Resonance       |  • Quét Sổ Tone tìm bài vừa giọng     |
|  • Tự nạp đường cong 7-Band EQ        |  • Tự gợi ý Transpose nốt cao/thấp    |
+---------------------------------------+---------------------------------------+
|  3. AI NOISE SUPPRESSION & DE-REVERB  |  4. SMART BPM-SYNCED REVERB & DELAY   |
|  • Khử quạt, chó sủa, xe chạy bằng AI |  • Tự động dò Tempo (BPM) của Beat    |
|  • Khử dội âm phòng không tiêu âm     |  • Delay nảy đúng phách, Reverb sạch  |
+---------------------------------------+---------------------------------------+
|  5. MINI YOUTUBE KARAOKE PLAYER       |  6. CLOUD AUTO-SYNC & PRESET SHOP     |
|  • 1-Click phát Beat từ Sổ Tone       |  • Sổ Tone cập nhật tự động online    |
|  • Auto Ad-Skip (Bỏ qua quảng cáo)   |  • Tải preset ca sĩ chỉ bằng 1-Click  |
+---------------------------------------+---------------------------------------+
```

### 1. 🎙️ Tính Năng 1: AI Vocal Profiler & Auto EQ Thông Minh (1-Click AI Sounding) - [x] **ĐÃ HOÀN THÀNH & TÍCH HỢP 100% VÀO BẢN PRO v3.x**
* **Vấn đề của người dùng thông thường**: 95% người hát livestream không biết chỉnh EQ (không biết cắt tần số 250Hz cho bớt đục, nâng 3kHz cho sáng tiếng, hay cắt 6kHz để khử chói).
* **Giải pháp AI Đã Triển Khai Hoàn Hảo**:
  1. Người dùng bấm nút **`[ ✨ AI AUTO-EQ ]`** trong Module Studio EQ và hát/nói thử một câu mẫu 5 giây vào Micro.
  2. Thuật toán **Spectral Profile & Formant Analysis** (`AiVocalProfiler`) tự động đo đạc:
     - Âm vực giọng: *Nam Trầm (Baritone), Nam Cao (Tenor), Nữ Trung (Alto), Nữ Cao (Soprano)*.
     - Tần số đục phòng (Room resonance mud: 200 - 450Hz).
     - Độ chói sibilance của âm gió (5kHz - 8.5kHz).
     - Độ thoát âm & độ sáng tự nhiên (Air frequencies: 9k - 18kHz).
  3. **Auto-EQ**: Tự động sinh ra 4 preset phong cách phối âm lý tưởng riêng biệt (*Studio Master, Bolero & Ballad, Remix & Pop, Podcast Streamer*), 1-Click nạp thẳng thông số vào Studio EQ giúp giọng hát **ấm, sáng, trong trẻo như thu âm studio**.

---

### 2. 🎵 Tính Năng 2: Nhận Diện Âm Vực & Đề Xuất Bài Hát Phù Hợp (Smart Song Recommendation)
* **Vấn đề của người dùng**: Người hát thường chọn bài theo sở thích nhưng hay bị "đuối hơi", "tịt nốt cao" hoặc "quá trầm không hát được", dẫn đến vỡ giọng trên livestream.
* **Giải pháp AI**:
  1. **Vocal Range Detector**: Khi người dùng hát khởi động, thuật toán Pitch Tracking (YIN/PYIN) tự động ghi nhận **Quãng giọng thực tế** (Ví dụ: Từ `C3` đến `E4`).
  2. **Smart Database Matching**: Hệ thống tự đối chiếu với Database Sổ Tone (1.000+ bài hát):
     - Lọc danh sách: *"Các bài hát vừa vặn nhất với chất giọng của bạn (Độ khó: 100% Phù hợp)"*.
     - Gợi ý bài Tone Nam / Nữ tương thích mà không cần gằn giọng.
  3. **Auto Transpose Suggestion**: Đối với những bài hát user rất thích nhưng nốt cao nhất vượt quá âm vực 2 bán âm, AI tự động gợi ý: *"Hạ Tone Beat xuống -2 semitones để vừa vặn hoàn hảo"*.

---

### 3. 🛡️ Tính Năng 3: AI Real-Time Denoise & Room De-reverb (DeepFilter AI Shield) - [x] **ĐÃ HOÀN THÀNH & TÍCH HỢP 100% VÀO BẢN PRO v3.x**
* **Giải pháp kỹ thuật siêu nhẹ (C++ RNNoise / DeepFilter GRU)**: Trọng số AI được biên dịch trực tiếp vào mã nguồn C++, dung lượng tăng thêm chỉ **~0.8 MB**, chiếm CPU < 1.5%.
* **Hiệu quả**: Khử triệt để tiếng quạt gió, tiếng ve kêu, tiếng còi xe ngoài đường, tiếng bàn phím cơ và tiếng dội âm của phòng chưa dán mút tiêu âm mà **hoàn toàn không làm méo tiếng giọng hát**.
* **Trải nghiệm sử dụng**:
  - **Module 1 trong Built-In DSP Rack**: Nút Power 🛡️, Thanh trượt `KHỬ ỒN AI (DENOISE)` 0-100%, Thanh trượt `TRIỆT TIÊU DỘI PHÒNG (DE-REVERB)` 0-100%, và đồng hồ đo `dB Khử` & `% Giọng Hát` thời gian thực.
  - **Phím tắt nhanh 1 chạm trên Top Bar**: Nút `[ 🛡️ AI SHIELD ]` hiển thị mức dB khử ồn trực tiếp, click trái bật/tắt nhanh, click phải mở menu cường độ (Nhẹ 40%, Studio Tiêu Chuẩn 75%, Mạnh 90%, Tối đa 100%).
  - **Song hành độc lập với Classic Noise Gate**: Không can thiệp hoặc xoá Noise Gate truyền thống; người dùng có thể kết hợp cả 2 để đạt độ tĩnh tuyệt đối.

---

### 4. ⏱️ Tính Năng 4: Smart BPM-Synced Reverb & Delay (Bí Quyết Studio Cho Tiếng Hát Hòa Quyện) - [x] **ĐÃ HOÀN THÀNH & TÍCH HỢP 100% VÀO BẢN PRO v3.x**
* **Nỗi đau lớn**:
  - Hát bài nhanh (Remix 128 BPM): Delay nhại chậm làm chồng chéo chữ, dính giọng, đục ngầu.
  - Hát bài chậm (Bolero 70 BPM): Delay dứt sớm làm khô giọng, hụt hơi.
* **Giải pháp Tự động Đã Triển Khai**:
  - **Tự động nhận Tempo (BPM)**: Lõi `TempoSyncEngine` nhận diện BPM từ dữ liệu Sổ Tone (1.033+ bài hát), nạp từ YouTube Player, nút `[ TAP ]` Tempo thông minh hoặc dò tự động qua thuật toán Spectral Flux Transient Onset Beat Detector.
  - **Auto-Delay theo phách**: Tự tính chính xác thời gian delay: $\text{Delay (ms)} = \frac{60.000}{\text{BPM}} \times \text{Subdivision}$ (1/4 Pop, 1/8 Dotted Ballad, 1/8 Energy, 1/8 Triplet Bounce, 1/16 Fast, 1/2 Long). Bộ đệm nội suy mượt mà (Fractional Smoothed Buffer) không gây nổ click khi chuyển tempo khi đang hát live.
  - **Auto-Reverb Tail**: Đuôi vang (Decay) tự động khép lại đúng cuối ô nhịp (1/2 Bar, 1 Bar, 2 Bars, 4 Bars) theo công thức $\text{Decay (sec)} = \frac{240}{\text{BPM}} \times \text{Bars}$, giọng vừa bay bổng mênh mang mà **không bao giờ bị đè mờ câu hát tiếp theo**.
  - **Đồng bộ toàn diện với VST3 bên thứ ba**: Tích hợp trực tiếp vào Host PlayHead (`MicroDawPlayHead`), giúp các Plugin VST3 như Valhalla, Soundtoys EchoBoy, FabFilter tự động bám nhịp BPM của bài hát đang phát.
  - **Dung lượng thêm vào**: **~0 MB (~20 KB code toán C++)**.

---

### 5. 📺 Tính Năng 5: Mini YouTube Karaoke Player (Siêu Nhẹ ~500 KB - Cực Phẩm Cho Dân Live)

#### A. Nỗi đau lớn của người hát Livestream hiện tại:
1. **Ngốn RAM & Gây đơ giật âm thanh:** Mở trình duyệt Chrome/Cốc Cốc ngốn **1GB - 2GB RAM**, khiến máy tính bị quá tải, gây hiện tượng nổ lách tách (Audio Crackles) và delay trên DAW.
2. **Quảng cáo bất thình lình:** Đang hát live cao trào thì YouTube chèn quảng cáo 15s gây tụt cảm xúc.
3. **Thao tác rườm rà:** Phải liên tục Alt-Tab giữa Chrome và DAW để chỉnh Tone / đổi bài.

#### B. Kiến trúc Kỹ thuật "Micro Footprint" (Chỉ ~500 KB):
- **Tuyệt đối KHÔNG dùng Electron hay đóng gói Chromium** (tránh làm phình App lên 150MB - 300MB).
- **Windows**: Tận dụng lõi **Microsoft Edge WebView2** (có sẵn 100% trên Windows 10/11) -> Dung lượng code nhúng chỉ **~300 KB**.
- **macOS**: Tận dụng lõi **WebKit / WKWebView** (có sẵn trong hệ điều hành macOS).
- **Mức ăn RAM**: Chỉ **~40 MB - 60 MB RAM** (Tiết kiệm **95% RAM** so với mở Chrome).

#### C. 4 Điểm Đột Phá Độc Quyền:
1. **1-Click Beat Sync từ Sổ Tone:** Bấm đúp vào bất kỳ bài nào trong Sổ Tone -> Mini Player tự động tìm và phát đúng Beat chuẩn trên YouTube, đồng thời nạp luôn Tone & BPM vào DAW.
2. **Clean Live View & Auto Ad-Skip:** Ẩn sạch bình luận và đề xuất video rác, tự động bấm Skip Ad trong 0.1 giây khi có quảng cáo.
3. **Always-on-Top Floating Mode:** Thu nhỏ cửa sổ video Karaoke ghim nổi trên góc màn hình OBS / DAW để vừa nhìn lời vừa quan sát sóng âm.
4. **Live Pitch Shifter:** Tích hợp nút `[ -1 ]` `[ 0 ]` `[ +1 ]` tăng/giảm Tone Beat trực tiếp trên trình phát mà không làm méo tiếng.

---

### 6. ☁️ Tính Năng 6: Cloud Songbook & Artist Preset Cloud
* **Dung lượng thêm vào App**: **0 MB** (Sử dụng REST API HTTPS nạp dữ liệu on-demand).
* Tự động đồng bộ các bài hát Hot Trend TikTok/YouTube vào Sổ Tone mỗi tuần qua Internet.
* Cung cấp thư viện Preset của các ca sĩ/streamer nổi tiếng (Preset Chu Bin, Preset Lệ Quyên, Preset Bolero ấm áp, Preset Acoustic mộc mạc...) chỉ cần bấm nạp.

---

## 📊 BẢNG TỔNG HỢP DUNG LƯỢNG & TỐC ĐỘ APP (GIỮ CHUẨN MICRO-DAW)

| Thành phần | Dung lượng bản v2.0 | Dự kiến bản v3.0 PRO | Mức chiếm RAM | Ghi chú kỹ thuật |
| :--- | :---: | :---: | :---: | :--- |
| **Engine DAW + UI Dark Mode** | ~5.8 MB | ~6.2 MB | ~35 MB | Pure C++ Native JUCE Engine |
| **AI Vocal Profiler & Auto EQ** | ❌ Chưa có | **+0.1 MB** | ~2 MB | FFT Spectral Formant Tracker |
| **Smart Vocal Range & Match** | ❌ Chưa có | **+0.2 MB** | ~1 MB | YIN Pitch Range & JSON Query |
| **AI Denoise & De-reverb** | ❌ Chưa có | **+0.8 MB** | ~10 MB | C++ Embedded RNNoise Weights |
| **Smart BPM-Synced FX** | ❌ Chưa có | **+0.02 MB** | ~0.5 MB | Tempo Math & Onset Beat Detector |
| **Mini YouTube Karaoke Player** | ❌ Chưa có | **+0.5 MB** | **~40 MB** | Native WebView2 / WKWebView API |
| **Cloud Sync & Preset Shop** | ❌ Chưa có | **0 MB** | ~1 MB | HTTP REST API On-Demand |
| **TỔNG BỘ CÀI SETUP (.EXE)** | **~6.15 MB** | **~7.8 MB - 8.5 MB** | **< 90 MB** | 🚀 **Vẫn siêu nhẹ (Dưới 10 MB)!** |

---

## 💰 PHẦN 3: MÔ HÌNH KINH DOANH & ĐỊNH GIÁ (PRICING STRATEGY)

```
                            +--------------------------+
                            |   PHỄU NGƯỜI DÙNG DAW    |
                            +--------------------------+
                                         |
                                         v
               [ BẢN FREE v2.0 - COMMUNITY ] (~80% User)
               • Đầy đủ Effect cơ bản + Sổ Tone Offline
               • Tự do hát hò livestream cơ bản
               • Đóng vai trò Marketing lan tỏa thương hiệu
                                         |
                       +-----------------+-----------------+
                       | (Muốn chuyên nghiệp, tự động hóa) |
                       v                                   v
             [ BẢN PRO v3.0 ]                     [ GÓI DỊCH VỤ VIP ]
             • AI Auto EQ theo chất giọng         • Setup 1-1 qua Anydesk
             • AI Đề xuất bài hát theo âm vực     • Cân chỉnh Mic & Soundcard
             • AI Denoise khử ồn sạch sẽ          • Tặng kèm Key PRO trọn đời
             • Smart BPM-Synced Delay/Reverb      • Giá: 299k - 499k / lượt
             • Mini YouTube Player Ad-Free
             • Giá: 49k/tháng hoặc 299k trọn đời
```

### 1. Bảng so sánh tính năng Free vs PRO

| Tính Năng | Bản Free (v2.0) | Bản PRO (v3.0) |
| :--- | :---: | :---: |
| Bộ Effect (Reverb, Delay, Compressor, Auto-Tune) | ✅ Có | ✅ Bản nâng cao (High Definition) |
| Tự động dò Tone Beat (Auto-Pitch Detection) | ✅ Có | ✅ Có |
| Sổ Tone tích hợp | ✅ 1.033 bài (Offline) | ☁️ Cập nhật Online liên tục |
| Kết nối OBS Studio 0ms | ✅ Có | ✅ Có |
| **Smart BPM-Synced Delay & Reverb (Theo nhịp Beat)** | ❌ Chỉnh tay ms | 🔥 **Tự động theo phách PRO** |
| **AI Nhận diện giọng & Auto EQ thông minh** | ❌ Không | 🔥 **Độc quyền PRO** |
| **AI Quét âm vực & Đề xuất bài hát hợp giọng** | ❌ Không | 🔥 **Độc quyền PRO** |
| **AI Real-time Denoise (Khử ồn thông minh)** | ❌ Không | 🔥 **Độc quyền PRO** |
| **Mini YouTube Karaoke Player (Chặn QC, Siêu nhẹ)** | ❌ Không | 🔥 **Độc quyền PRO** |
| **Cloud Preset Shop (Kho Preset ca sĩ)** | ❌ Không | 🔥 **Độc quyền PRO** |
| Hỗ trợ kỹ thuật ưu tiên | Qua group cộng đồng | Kỹ thuật viên hỗ trợ riêng |

---

## 🛠️ PHẦN 4: LỘ TRÌNH KỸ THUẬT TRIỂN KHAI (TECHNICAL ROADMAP)

### Giai đoạn 1: Tận dụng cơ chế Check Update (`version.json`)
- Phần mềm v2.0 đang chạy có sẵn hàm kiểm tra phiên bản từ file `version.json` trên GitHub.
- Khi bản v3.0 PRO hoàn thành, nội dung popup thông báo cập nhật sẽ giới thiệu các tính năng AI mới cùng nút **"Trải nghiệm bản PRO"**.

### Giai đoạn 2: Phát triển Module C++ & DSP Engine
1. **Module `TempoSyncEngine`**:
   - Tính toán `delayTimeMs = (60000.0f / currentBpm) * subdivisionFactor`.
   - Điều khiển tự động `StereoDelayProcessor` và `PlateReverbProcessor`.
2. **Module `VocalAnalysisEngine`**:
   - Sử dụng thuật toán FFT và YIN Pitch Tracker đo dải tần Formant (F1, F2, F3) và Pitch Contour.
   - Trả về cấu hình `EQSettings` (Gain, Q, Freq của từng Band) nạp trực tiếp vào `ParametricEQProcessor`.
3. **Module `MiniYouTubePlayerComponent`**:
   - Tích hợp `juce::WebBrowserComponent` với backend WebView2 (Windows) và WKWebView (macOS).
   - Inject JavaScript Controller điều khiển Play/Pause, Seek, Pitch và Ad-Skip.
4. **Module `VocalRangeMatcher`**:
   - Ghi nhận `MinPitch` và `MaxPitch` khi người dùng hát thử.
   - Lọc dữ liệu JSON Sổ Tone theo trường `vocal_range_min` và `vocal_range_max`.

### Giai đoạn 3: Hệ thống Bản quyền (License Manager)
- **Machine ID**: Tạo chuỗi Hardware UUID duy nhất từ CPU ID + Motherboard UUID.
- **Kích hoạt Online**: Xác thực License Key qua API Cloudflare Worker / Firebase cực nhẹ, không làm chậm DAW.
- **Thanh toán tự động**: Tích hợp quét mã QR VietQR (SeAPay / Casso) để cấp Key tự động qua Email/Zalo trong 30 giây.

---

## 🎯 TÓM LẠI
1. **Không cần lo lắng về lượng người dùng bản Free hiện tại**: Họ chính là đại sứ truyền thông tốt nhất giúp quảng bá phần mềm rộng rãi.
2. **Bản PRO ra mắt sẽ nhắm vào nhóm muốn "Hát hay hơn mà không cần biết kỹ thuật"**: Tính năng **Auto EQ theo chất giọng**, **Smart BPM-Synced FX**, **Đề xuất bài theo âm vực**, và **Mini YouTube Player siêu nhẹ** tạo thành một hệ sinh thái hoàn hảo, không có đối thủ cạnh tranh trên thị trường hát live/karaoke máy tính!
