#!/bin/bash
# ==============================================================================
# LiveStream Micro-DAW - macOS Universal Binary (Apple Silicon + Intel) Build & Package Script
# ==============================================================================
set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
WORKSPACE_DIR="$(cd "${SCRIPT_DIR}/.." && pwd)"
cd "${WORKSPACE_DIR}"

echo ">>> [1/4] Configuring CMake for macOS (Universal Binary: arm64 + x86_64)..."
cmake -B build_mac \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_OSX_ARCHITECTURES="arm64;x86_64" \
    -DCMAKE_OSX_DEPLOYMENT_TARGET="10.15"

echo ">>> [2/4] Building LiveStream Micro-DAW & OBS Receiver Plugin..."
cmake --build build_mac --config Release --target LiveStreamMicroDAW LiveStreamOBSReceiver -j$(sysctl -n hw.ncpu)

echo ">>> [3/4] Preparing macOS Release Bundle..."
RELEASE_DIR="dist_macos/LiveStream Micro-DAW (macOS)"
rm -rf "dist_macos"
mkdir -p "${RELEASE_DIR}/Plugins"

# Copy Standalone App Bundle
cp -R "build_mac/LiveStreamMicroDAW_artefacts/Release/LiveStream Micro-DAW.app" "${RELEASE_DIR}/"

# Copy OBS Receiver VST3 & AU (Component)
if [ -d "build_mac/LiveStreamOBSReceiver_artefacts/Release/VST3/LiveStream OBS Receiver.vst3" ]; then
    cp -R "build_mac/LiveStreamOBSReceiver_artefacts/Release/VST3/LiveStream OBS Receiver.vst3" "${RELEASE_DIR}/Plugins/"
fi
if [ -d "build_mac/LiveStreamOBSReceiver_artefacts/Release/AU/LiveStream OBS Receiver.component" ]; then
    cp -R "build_mac/LiveStreamOBSReceiver_artefacts/Release/AU/LiveStream OBS Receiver.component" "${RELEASE_DIR}/Plugins/"
fi

# Copy Documentation
cp "README.md" "${RELEASE_DIR}/"
cp "README_EN.md" "${RELEASE_DIR}/"
cp "HUONG_DAN_SU_DUNG.txt" "${RELEASE_DIR}/"

# Quick Install Script for macOS
cat << 'EOF' > "${RELEASE_DIR}/Cai_Dat_Plugin_OBS_Mac.command"
#!/bin/bash
DIR="$(cd "$(dirname "$0")" && pwd)"
echo ">>> Đang cài đặt Plugin LiveStream OBS Receiver vào hệ thống macOS..."
mkdir -p ~/Library/Audio/Plug-Ins/VST3
mkdir -p ~/Library/Audio/Plug-Ins/Components

if [ -d "$DIR/Plugins/LiveStream OBS Receiver.vst3" ]; then
    cp -R "$DIR/Plugins/LiveStream OBS Receiver.vst3" ~/Library/Audio/Plug-Ins/VST3/
    echo "✓ Đã cài VST3 vào ~/Library/Audio/Plug-Ins/VST3"
fi

if [ -d "$DIR/Plugins/LiveStream OBS Receiver.component" ]; then
    cp -R "$DIR/Plugins/LiveStream OBS Receiver.component" ~/Library/Audio/Plug-Ins/Components/
    echo "✓ Đã cài AU vào ~/Library/Audio/Plug-Ins/Components"
fi

echo ""
echo "=== HOÀN TẤT CÀI ĐẶT PLUGIN CHO OBS MAC ==="
echo "Bây giờ bạn có thể mở OBS Studio và thêm bộ lọc VST3/Audio Unit!"
EOF
chmod +x "${RELEASE_DIR}/Cai_Dat_Plugin_OBS_Mac.command"

echo ">>> [4/4] Creating ZIP package for macOS..."
cd "dist_macos"
zip -r -y "LiveStream_Micro_DAW_macOS.zip" "LiveStream Micro-DAW (macOS)"
cd ..

echo ">>> SUCCESS: Generated dist_macos/LiveStream_Micro_DAW_macOS.zip"
