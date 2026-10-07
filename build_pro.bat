@echo off
echo ========================================================
echo  BUILDING LIVESTREAM MICRO-DAW PRO (COMMERCIAL EDITION)
echo ========================================================
call "C:\Program Files\Microsoft Visual Studio\18\Community\VC\Auxiliary\Build\vcvars64.bat"
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
ninja -C build LiveStreamMicroDAW_PRO LiveStreamOBSReceiver_VST LiveStreamOBSReceiver_VST3
if %ERRORLEVEL% EQU 0 (
    echo.
    echo [SUCCESS] LiveStream Micro-DAW PRO compiled successfully!
    echo Packaging PRO Edition artifacts...
    powershell -ExecutionPolicy Bypass -File tools\package_pro.ps1
)
