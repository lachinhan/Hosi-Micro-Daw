# 🎛️ LiveStream Micro-DAW & OBS Audio Receiver (VST2 / VST3)

[🇻🇳 Tiếng Việt](README.md) | [🇬🇧 English](README_EN.md)

An ultra-low latency (< 5ms), specialized **Micro-DAW designed for Live Stream Vocalists and Stage Performers**, built on modern **C++20** and **JUCE 8**.

The application allows you to connect your microphone directly to professional VST3 plugins (Auto-Tune, Pro-Q 3, CLA-76, Pro-R...) for real-time vocal processing. It simultaneously streams the processed audio directly into **OBS Studio** via zero-copy, lock-free Shared Memory (IPC) with zero latency—**without requiring virtual audio cables (VB-Cable, Voicemeeter)**.

---

## 📑 Table of Contents
1. [🌟 Key Features](#-key-features)
2. [📦 Installation & Deployment](#-installation--deployment)
3. [🎙️ OBS Studio Setup Guide (Mandatory)](#️-obs-studio-setup-guide-mandatory)
4. [🎧 How to Use LiveStream Micro-DAW](#-how-to-use-livestream-micro-daw)
5. [❓ Troubleshooting & FAQ](#-troubleshooting--faq)
6. [🛠️ Build & Developer Guide](#️-build--developer-guide)

---

## 🌟 Key Features

### 🤖 NEXT-GEN AI VOCAL STUDIO FEATURES (PRO EDITION v3.x ⭐)

1. **🎙️ AI Vocal Profiler & Smart Auto-EQ (1-Click AI Sounding)**:
   - **Intelligent Spectral & Formant Profiling**: Speak or sing a short vocal snippet into your microphone. The AI analyzer captures your fundamental F0, detects room boom mud (200 - 450Hz), vocal sibilance (5k - 8.5kHz), and high-frequency air (9k - 18kHz).
   - **1-Click Optimal EQ Compensation**: Generates 4 tailored professional mixing styles (*Studio Master, Sweet Bolero, Remix Pop, Podcast Streamer*), automatically configuring the 3-Band Studio EQ for crystal-clear, broadcast-ready vocals.

2. **🎵 AI Vocal Range Detector & Smart Song Recommendation**:
   - **Real-Time Vocal Range Tracking (`VocalRangeDetector`)**: High-precision pitch tracker measuring F0 frequencies from 65 Hz (`C2`) up to 1,050 Hz (`C6`).
   - **15-Second Vocal Siren Scan**: Sing smoothly from your lowest note ("Oh...") to your highest note ("Ee..."). The interactive Piano Roll visualizer expands dynamically in real-time, categorizing your voice (*Baritone/Bass, Tenor, Alto/Mezzo, Soprano*).
   - **Smart Database Matching (1,033+ Songs)**: Filter songs via `[ 🎯 Gợi Ý Vừa Giọng AI ]`, calculating vocal Fit Scores (100% Perfect Match, 95% Great Fit) with smart transpose recommendations: *"🎯 Lower -2 semitones to reach peak notes comfortably"*.
   - **1-Click Sing Now**: Transposes and syncs target Key & Scale into Auto-Tune and triggers YouTube Beat playback simultaneously.

3. **🛡️ AI Real-Time Denoise & Room De-reverb Shield (DeepFilter AI Shield)**:
   - **Zero-Latency Neural Noise & Reverb Suppression**: 16-band zero-latency biquad filterbank eliminates fan noise, road traffic, cicadas, mechanical keyboard clicks, and untreated room reverberations **with zero vocal phase distortion**.
   - **1-Touch Top Bar Quick Access `[ 🛡️ AI SHIELD ]`**: Left-click for instant toggle, right-click to select 4 preset intensities (40% Light, 75% Studio Standard, 90% Heavy, 100% Maximum).

4. **⏱️ Smart BPM-Synced Reverb & Delay (Tempo-Locked Spatial FX)**:
   - **Automatic Tempo Detection (BPM)**: Detects BPM automatically from Songbook database, YouTube Player, smart `[ TAP ]` tempo, or Spectral Flux Onset Beat Tracking.
   - **Rhythmic Auto-Delay**: Calculates exact tempo-synced delay intervals: $\text{Delay (ms)} = \frac{60,000}{\text{BPM}} \times \text{Subdivision}$ (1/4 Pop, 1/8 Dotted Ballad, 1/8 Triplet Bounce...).
   - **Auto-Reverb Tail**: Reverb decay closes rhythmically at the end of the musical bar (1/2 Bar, 1 Bar, 2 Bars, 4 Bars) via $\text{Decay (sec)} = \frac{240}{\text{BPM}} \times \text{Bars}$, keeping vocals spacious without muddying subsequent lyrics.
   - **3rd-Party VST3 Host Sync**: Directly updates the host PlayHead (`MicroDawPlayHead`) for Valhalla, Soundtoys EchoBoy, FabFilter.

5. **📺 Integrated YouTube Karaoke Player (Mini YouTube Karaoke Player)**:
   - **Native YouTube Player**: Search and stream karaoke backing tracks directly inside the DAW with an ad-free clean interface (**`🛡️ CHẶN QC / LIVE`**).
   - **1-Touch YouTube Key Detection (`🎯 DÒ TONE`)**: Matches video titles against the 1,033+ song database to feed accurate root keys and scales directly into Auto-Tune.
   - **Auto-Tune Modulation on Key Changes (`MOD MIC: [-1] [0] [+1] [+2]`)**: Transpose Auto-Tune when songs modulate key at the bridge or chorus, **keeping YouTube backing audio 100% natural and unpitched**.
   - **Instant Male / Female Duet Switching (`[♂ NAM]` / `[♀ NỮ]`)**: Seamlessly switch Auto-Tune target keys with 0ms latency during duets.

6. **☁️ Cloud Songbook & Artist Preset Cloud (Online Songbook & Studio Presets)**:
   - **On-Demand Hot Trend Sync (`☁️ Đồng Bộ Cloud`)**: Background non-blocking HTTPS synchronization fetching the latest viral TikTok/YouTube songs without overwriting custom or favorite songs `❤️`.
   - **Studio Artist Preset Cloud**: 1-Click loading of professional vocal chains crafted for signature genres (*Dance/Trap Chu Bin, Deep Bolero Lệ Quyên, Acoustic Mộc Đạt G, Streamer Talkshow Radio, Vinahouse Party*).
   - **Automated Management Tool**: Bundled with `tools/sync_cloud_data.py` for automated cloud validation and crawling.

---

### 🎛️ CORE STUDIO & LIVESTREAM DAW ENGINE

- **🎵 Auto Key & Scale Detection (Real-Time)**: Built-in Harmonic Chromagram analyzer combined with the Krumhansl-Schmuckler musical correlation engine to automatically detect the exact Root Note and Scale (e.g., C Major, F# Minor) of any loaded backing track or live microphone input stream.
- **⚡ Auto-Push Tone on Lock (`⚡ AUTO` Mode)**:
  - **Auto-Push Mode (`⚡ AUTO`)**: When a backing track plays and locks tone with high confidence ($\ge 75\%$), the engine **automatically transmits the Key & Scale directly into Auto-Tune Pro in the rack** without needing any manual mouse clicks!
  - **Manual Mode (`AUTO: OFF`)**: Allows users to review detected tone and sync manually via the **`SYNC AUTO-TUNE`** button.
- **🤖 24/7 Online AI Assistant Guide**: Built-in specialized AI assistant at [byvn.net/hosiguide](https://byvn.net/hosiguide) ready to answer questions and provide step-by-step setup guides for VST3, OBS Studio, and vocal routing.
- **🎛️ Manual Key Selector Menu (24 Major/Minor Keys)**: Intuitive popup menu to instantly select any of the 24 musical keys and feed it directly into Auto-Tune with a single click.
- **⚡ 1-Click Auto-Tune Key Sync**: Automatically discovers and synchronizes the detected Key and Scale directly into loaded pitch correction plugins (Antares Auto-Tune, Waves Tune, MAutoPitch) in the rack.

- **🎶 Studio DAW Transport Bar with Sleek Vector Icons**:
  - Supports loading and playback for MP3, WAV, FLAC, OGG, and AIFF files.
  - Studio-grade vector icons: **`▶` Play / `❚❚` Pause**, **`■` Stop**, **`⟳` Loop**, precision Seek bar, Volume slider, and time counter.
  - Animated 12-semitone Chroma LED visualizer.
- **🎛️ Live Soundboard Pads (Custom Renaming & Sample Loading)**:
  - Integrated 8-Pad Soundboard (Applause 👏, Laughs 😂, Drumroll 🥁, Ding Bell 🔔, Airhorn 🎺, Boom/Hit 💥, Buzzer ❌, Cheer/Win 🎉).
  - **Easy Pad Renaming**: **Double-click** any pad or **Right-click** $\to$ `✏ Rename Pad...`.
  - **Right-Click Context Menu**: Instant renaming, custom audio sample assignment (`.mp3`, `.wav`), and 1-click factory sample reset (`↺ Reset`).
  - Global hotkeys (1..8, Numpad 1..8), Stop All (Esc), and automatic preset persistence.
- **📊 Symmetrical Master Channel Strip & Studio dB Metering**:
  - Harmoniously centered Dual Stereo VU Meter (L & R), logarithmic dB reference markings (`+6`, `0`, `-6`, `-12`, `-24`, `-inf`), and Master Fader.
  - Dual L/R Clip warning LEDs, digital `+0.0 dB` badge, and **`0 dB RESET`** button.
- **Zero-Latency IPC Engine**: Powered by Windows Named Shared Memory (`Local\LiveStreamMicroDAW_AudioIPC_v1`) and a 16,384-sample lock-free ring buffer with SPSC `acquire`/`release` memory barriers.
- **Dynamic PLL Resampling**: Automatic real-time sample rate conversion (e.g., DAW running at 44.1 kHz, OBS running at 48.0 kHz) utilizing a fractional linear resampler coupled with a Phase-Locked Loop (PLL) drift compensator, eliminating audio pops, crackling, buffer underruns, and overruns.
- **Universal ASIO Compatibility**: Auto-detects active hardware input channel pairs (1-2, 3-4...) on professional audio interfaces such as Roland QUAD-CAPTURE, Focusrite Scarlett, Behringer, Yamaha Steinberg, and more.
- **Full VST3 Hosting Rack**: 8 flexible slots supporting native VST3 GUI popup windows from industry-standard plugins (Antares Auto-Tune, FabFilter, Waves, Soundtoys...).
- **1-Click Live Scene Presets**:
  - 🎤 **HÁT LIVE (Singing)**: Engages the full vocal chain (Auto-Tune + EQ + Compressor + Reverb Aux).
  - 💬 **GIAO LƯU (Talk/Chat)**: Automatically bypasses Auto-Tune & Reverb while maintaining Noise Gate and Compression for crisp, intelligible speech.
  - 🔥 **AUTOTUNE (Hard-Tune)**: Delivers aggressive pitch correction and spatial processing for energetic genres.
- **📺 Professional YouTube Karaoke Player (PRO Edition ⭐)**:
  - **Embedded YouTube Karaoke Portal**: Search and stream karaoke backing tracks directly inside the DAW with an ad-free clean interface (**`🛡️ CHẶN QC / LIVE`**).
  - **1-Touch YouTube Tone Detection (`🎯 DÒ TONE`)**: Automatically detects song titles and matches them against a 1,033+ song database to feed accurate root keys and scales directly into Auto-Tune.
  - **Vocal Auto-Tune Modulation on Key Changes (`MOD MIC: [-1] [0] [+1] [+2]`)**: When the backing music naturally shifts key at the bridge or chorus, singer can press a button or hotkey to transpose Auto-Tune, **keeping the YouTube backing track 100% natural with zero pitch distortion**.
  - **Real-Time YouTube Beat Transposition (`BEAT: [-3] ... [+3]`)**: Shift the backing track pitch in real-time when a suitable singer key is unavailable, with automatic Auto-Tune synchronization.
  - **1-Click Male / Female Duet Switching (`[♂ NAM]` / `[♀ NỮ]`)**: Instantly toggle between male and female vocal keys with 0ms latency.
  - **Live Keyboard Performance Shortcuts**:
    - `]` (or `PageUp`): Transpose Auto-Tune +1 semitone (on song modulation).
    - `[` (or `PageDown`): Transpose Auto-Tune -1 semitone.
    - `+` / `-`: Transpose YouTube Beat pitch.
    - `0`: Reset both Beat and Auto-Tune to original factory key.
    - `M` or `1`: Switch Auto-Tune to Male key.
    - `F` or `2`: Switch Auto-Tune to Female key.
    - `D` or `Tab`: Fast toggle Male $\leftrightarrow$ Female keys.
- **💖 Creator Support & QR Donation Modal**: Top header **`☕ DONATE`** button opens a high-resolution QR modal (VietQR MBBank & MoMo) with 1-click clipboard copy for account numbers and transfer notes.
- **Custom UI Zoom Scaling**: 3 responsive scaling modes: **85% (Compact)**, **100% (Standard)**, and **125% (Large)** with auto-window centering.

---

## 📦 1-Click Installation & Deployment

### 🌟 Option 1: Automated 1-Click Setup Wizard (Recommended)
Run the single setup installer executable:
```text
LiveStream_Micro_DAW_Setup.exe
```
- **Fully Automated**:
  - Installs **LiveStream Micro-DAW** and creates handy shortcuts on **Desktop** & **Start Menu**.
  - **Auto-detects and installs OBS Receiver Plugins (both VST2 `.dll` and VST3 `.vst3`)** into system directories (`C:\Program Files\Common Files\VST3` and `C:\Program Files\VstPlugins`).
  - Users **never have to manually locate system VST folders or copy files by hand!**

### 📁 Option 2: Portable Mode (Direct Launch)
1. **Run Application**: Double-click `LiveStream Micro-DAW.exe`.
2. **1-Click Plugin Setup for OBS**:
   - Double-click **`Install_OBS_Plugin.bat`** (or right-click $\to$ *Run as administrator*).
   - The script will **automatically copy both VST2 (.dll) and VST3 (.vst3)** into standard system directories (`C:\Program Files\Common Files\VST3\` and `C:\Program Files\VstPlugins\`).
   - *(Optional)* To remove plugins cleanly, run `Uninstall_OBS_Plugin.bat`.

---

## 🎙️ OBS Studio Setup Guide (Mandatory)

To ensure crystal-clear stream audio with zero latency and no double-monitoring echo, follow these steps:

```
[Microphone] ──► [LiveStream Micro-DAW]
                       │
                       ├───► [Headphones via ASIO Soundcard] (You hear 0-latency direct audio)
                       │
                       └───► [Shared Memory IPC] ──► [OBS: LiveStream Receiver] ──► [Stream Viewers]
```

### Step 1: Add the Plugin Filter in OBS
1. Open **OBS Studio**.
2. In the **Audio Mixer** dock, locate your microphone source (e.g., **Mic/Aux** or an **Audio Input Capture** source).
3. Right-click the source $\to$ select **Filters**.
4. Click the **`+`** icon at the bottom $\to$ choose **VST 2.x/3.x Plug-in**.
5. In the plugin dropdown menu, select **LiveStream OBS Receiver**.
6. Click **Open Plug-in Interface** $\to$ You will see the status indicator **`🟢 CONNECTED TO MICRO-DAW (Active)`** and live VU metering.

### Step 2: MANDATORY SETTING TO PREVENT ECHO ⚠️
1. In OBS Studio, click the gear icon (or right-click any volume slider) in the Audio Mixer $\to$ select **Advanced Audio Properties**.
2. Find the microphone source where you added the receiver plugin (e.g., `Mic/Aux`).
3. In the **Audio Monitoring** column:
   👉 **SET TO: `Monitor Off` (Tắt giám sát)**.

> [!IMPORTANT]
> **Why must Audio Monitoring be set to `Monitor Off`?**
> - You already monitor your singing in real-time (0-latency) directly from the **LiveStream Micro-DAW** through your soundcard headphones.
> - If you enable OBS Monitoring, OBS will feed a secondary, delayed Windows audio stream back into your headphones, causing an **unwanted double-voice echo**. Setting it to `Monitor Off` ensures only your stream viewers receive the audio from OBS!

---

## 🎧 How to Use LiveStream Micro-DAW

### 1. Audio Device Configuration
- Click the **Audio Settings** button at the top-right corner.
- **Audio Device Type**: Select **ASIO** (choose your soundcard driver, e.g., *QUAD-CAPTURE, Focusrite USB ASIO, ASIO4ALL v2...*).
- **Audio Buffer Size**: Recommended **128 samples** or **256 samples** for lowest latency.
- **Active Inputs**: Enable your physical microphone input channels.

### 2. Scanning and Loading VST3 Plugins
- Click **Scan VST3** to automatically index all installed VST3 plugins on your system (`C:\Program Files\Common Files\VST3`).
- In the 8 rack slots:
  - Click the dropdown menu on any slot to assign a plugin (e.g., Slot 1: Auto-Tune, Slot 2: Noise Gate, Slot 3: Pro-Q 3, Slot 4: CLA-76, Slot 5: Pro-R...).
  - **Double-click** any slot to pop out the plugin's native GUI window.
  - Toggle the **Bypass** button on individual slots as needed.

### 3. Beat Key Detection & Auto-Tune Synchronization 🎵
- **Load Backing Track**: Click **LOAD BEAT** $\to$ Select any karaoke track (`.mp3`, `.wav`, `.flac`, `.ogg`). The DAW will analyze its key within 1 second and display the result (e.g., `F# Minor`).
- **Playback Controls**: Use the vector **`▶` Play / `❚❚` Pause**, **`■` Stop**, and **`⟳` Loop** buttons with the seek slider. Adjust track loudness with the **VOL** slider.
- **Source Selection**: Switch between **`SOURCE: BEAT`** and **`SOURCE: MIC LIVE`** to analyze either the background track or live singing from the microphone.
- **Manual Key Selection**: Click **`CHOOSE KEY`** $\to$ Instantly assign any of the 24 Major/Minor scales directly into Auto-Tune.
- **Sync to Auto-Tune**: Click **SYNC AUTO-TUNE** $\to$ Automatically assigns the exact Root Key and Scale to Auto-Tune in the rack.

### 4. Smart Voice Ducking (Radio Talk-over) 📉
- **Concept**: Broadcast/radio style automatic volume ducking.
- **Usage**: Click the **`DUCKING: OFF`** toggle on the Beat Player toolbar to activate **`DUCKING: ON`** (glowing green).
- **Behavior**:
  - Whenever you speak into the microphone $\to$ Backing track automatically attenuates by 6 dB smoothly.
  - When speech stops or you start singing along to the beat $\to$ Volume seamlessly ramps back up to normal levels without abrupt pumping.

### 5. Built-in Studio DSP Vocal Suite 💎
- **Concept**: Users are not required to install or crack heavy 3rd-party VST bundles (Waves, FabFilter, iZotope...). Micro-DAW includes a native, studio-grade vocal DSP suite with zero added latency (0ms):
  1. **Noise Gate**: Silences background room hum, PC fan noise, and keyboard clicks.
  2. **Studio 3-Band EQ**: Tailor Low shelf (120Hz), vocal presence/clarity (Mid 2.6kHz), and high-end air (High 9.5kHz).
  3. **Warm Compressor**: Dynamic range control with makeup gain and analog-modeled harmonic warmth.
  4. **Lush Reverb**: Silky plate/hall room acoustics for live singing and karaoke.
  5. **Stereo Delay / Echo**: Ping-pong stereo echoes with analog tape high-cut damping.
  6. **Brickwall Limiter**: Peak ceiling protection (-0.3 dBFS) preventing harsh clipping when shouting or singing loud passages.
- **How to Use**: In the right-hand column, click the **`VOCAL DSP`** tab $\to$ Toggle `PWR` switches, select quick presets, or click **`🔄 RESET`** (or select Option 6: *Factory Reset*) to instantly restore all DSP sliders and modules to pristine studio defaults.

### 6. Quick 1-Touch 24-Bit Multi-Track Audio Recorder 🔴
- **Concept**: Dedicated recording controls aligned directly alongside the Smart Ducking toggle on the Beat Player bar.
- **Features**:
  - **1-Touch Quick Record**: Click **`● REC`** $\to$ Button pulses red with live elapsed time (e.g., `REC 01:25`).
  - **Dual-Track 24-bit WAV**: Simultaneously outputs 2 pristine studio audio files into the `recordings/` folder:
    1. `Master_Mix_YYYY-MM-DD_HH-MM-SS.wav`: Complete live stream output with backing beat, tuned vocals, FX, and soundboard.
    2. `Mic_Dry_YYYY-MM-DD_HH-MM-SS.wav`: Raw, unprocessed dry microphone signal, ideal for video editors in CapCut, Adobe Premiere, and DaVinci Resolve (TikTok, YouTube Shorts, Reels).
  - **Open Recordings Directory**: Click **`DIR`** right next to the REC button to open the recordings directory in Windows Explorer.

### 7. Live Soundboard Multi-FX Pads 🎛️
- **Switch to Soundboard**: Click the **`SOUND FX`** tab at the top of the right-hand column $\to$ Displays the 8 illuminated sound pads.
- **Global Hotkeys**:
  - Press keys **`1` to `8`** (or **`Numpad 1` to `Numpad 8`**) to trigger sounds in real-time during live streams.
  - Press **`ESC`** (or the **`STOP ALL`** button) to immediately silence all audio.
- **Pad Renaming & Custom Audio Files**:
  - **Rename Pad**: **Double-click** any pad or **Right-click** $\to$ Select **`✏ Rename Pad...`**.
  - **Load Custom Sample**: **Right-click** $\to$ Select **`📁 Load Audio Sample (.wav, .mp3)...`**.
  - **Reset to Factory Sound**: **Right-click** $\to$ Select **`↺ Reset to Default`**.
  - Or place audio files into the **`sounds/`** directory next to `LiveStream Micro-DAW.exe`.

### 8. Saving & Loading Presets
- Click **Save Preset** $\to$ Exports the full plugin chain, DSP settings, and parameter state to a `.hmdaw` file.
- Click **Load Preset** $\to$ Instantly recalls your custom vocal chains in seconds.

### 9. Adjusting UI Zoom
- Click the **85%**, **100%**, or **125%** buttons on the top bar to scale the interface to match your display resolution.

### 10. YouTube Karaoke Player & Performance Hotkeys (PRO Edition ⭐)
- **Open Player**: Click the **`📺 YOUTUBE KARAOKE`** button on the top header (or launch tracks from the Songbook).
- **Core Features**:
  1. **🎯 1-Touch Tone Detection (`🎯 DÒ TONE (AUTO-KEY)`)**: Automatically detects song titles and syncs correct key and scale into Auto-Tune Pro.
  2. **🎤 Vocal Auto-Tune Modulation on Key Changes (`MOD MIC: [-1] [0] [+1] [+2]`)**: When the song naturally modulates at the bridge or chorus, press the button or hotkey to transpose Auto-Tune, **keeping the YouTube video playback untouched**.
  3. **🎛️ Real-Time YouTube Beat Transposition (`BEAT: [-3] ... [+3]`)**: Transpose the backing track in real-time when a suitable vocal key is not available, with automatic Auto-Tune synchronization.
  4. **👫 1-Click Male / Female Duet Switching (`[♂ NAM]` / `[♀ NỮ]`)**: Instant toggle between male and female keys with 0ms latency.

#### ⌨️ Live Performance Hotkeys Table:
| Shortcut | Action | Description |
| :--- | :--- | :--- |
| **`]`** or **`PageUp`** | **Auto-Tune Transpose (+1)** | Transpose Auto-Tune +1 semitone when song modulates (YouTube beat untouched) |
| **`[`** or **`PageDown`** | **Auto-Tune Transpose (-1)** | Transpose Auto-Tune -1 semitone (YouTube beat untouched) |
| **`+`** / **`-`** (or `↑`/`↓`) | **Transpose YouTube Beat** | Shift YouTube backing track pitch with auto Auto-Tune sync |
| **`0`** | **Reset to Original Key** | Reset both Beat and Auto-Tune to default key |
| **`M`** or **`1`** | **Male Vocal Tone** | Switch Auto-Tune to Male key |
| **`F`** or **`2`** | **Female Vocal Tone** | Switch Auto-Tune to Female key |
| **`D`** or **`Tab`** | **Toggle Male $\leftrightarrow$ Female** | Fast toggle between Male and Female vocal keys |

---

## ❓ Troubleshooting & FAQ

### 1. OBS is not receiving any audio signal from the DAW?
- **Verify Micro-DAW**: Check if the Master Output VU Meter inside Micro-DAW is actively moving.
- **Verify OBS Receiver**: Open the plugin interface in OBS and ensure it displays `🟢 CONNECTED TO MICRO-DAW (Active)`.
- **Sample Rate Mismatch**: If you changed the soundcard sample rate, restart Micro-DAW to reset the IPC memory synchronization.

### 2. Audio crackles, pops, or stutters in OBS?
- Ensure you are running the latest build of **LiveStream OBS Receiver** (equipped with the PLL Resampler).
- Configure your ASIO Soundcard Buffer Size to **128** or **256 samples**.

### 3. I hear my voice twice (delayed echo)?
- You forgot to disable OBS Audio Monitoring! Open **Advanced Audio Properties** in OBS and change your Mic source monitoring to **`Monitor Off`**.

---

## 🛠️ Build & Developer Guide

### Prerequisites
- **Windows 10 / 11 64-bit**
- **Visual Studio 2022** (with Desktop Development with C++ workload)
- **CMake 3.22+**
- **Steinberg ASIO SDK** (placed at `C:/SDKs/asiosdk` or custom directory)

### Build Commands (PowerShell)

```powershell
# 1. Navigate to the project directory
cd "e:\Hosi Micro Daw"

# 2. Configure CMake project (Visual Studio 2022 x64)
cmake -B build -G "Visual Studio 17 2022" -A x64 -DASIO_SDK_DIR="C:/SDKs/asiosdk"

# 3. Build optimized Release binaries
cmake --build build --config Release --parallel
```

### Build Output Binaries:
1. `build/LiveStreamMicroDAW_artefacts/Release/LiveStream Micro-DAW.exe` (Standalone DAW Host)
2. `build/plugins/receiver/LiveStreamOBSReceiver_artefacts/Release/VST/LiveStream OBS Receiver.dll` (VST2 Plugin)
3. `build/plugins/receiver/LiveStreamOBSReceiver_artefacts/Release/VST3/LiveStream OBS Receiver.vst3` (VST3 Plugin)

---

## 📄 License & Credits
- **Developed by**: Hosi Studio Engineering Team
- **Core Technologies**: JUCE Framework 8, C++20, Win32 Memory-Mapped Files IPC.
