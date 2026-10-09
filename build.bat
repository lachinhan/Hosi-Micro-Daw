@echo off
echo ========================================================
echo  BUILDING LIVESTREAM MICRO-DAW v2.0.5 (STANDALONE ^& VST)
echo ========================================================
call "C:\Program Files\Microsoft Visual Studio\18\Community\VC\Auxiliary\Build\vcvars64.bat"
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
ninja -C build LiveStreamMicroDAW LiveStreamOBSReceiver_VST LiveStreamOBSReceiver_VST3
if %ERRORLEVEL% EQU 0 (
    echo.
    echo [SUCCESS] LiveStream Micro-DAW v2.0.5 compiled successfully!
    echo Packaging release artifacts...
    powershell -ExecutionPolicy Bypass -File tools\package_release.ps1
)
