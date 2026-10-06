@echo off
setlocal EnableDelayedExpansion
title LiveStream Micro-DAW - OBS Plugin Installer

:: ----------------------------------------------------------------------------
:: 1. Auto Request Administrator Privileges (UAC)
:: ----------------------------------------------------------------------------
net session >nul 2>&1
if %errorlevel% neq 0 (
    echo Dang yeu cau quyen Administrator de copy plugin vao thu muc he thong...
    powershell -Command "Start-Process cmd -ArgumentList '/c cd /d \"\"%~dp0\"\" && call \"%~f0\" --elevated' -Verb RunAs"
    exit /b
)

:: Set UTF-8 encoding
chcp 65001 >nul
cls

echo ==============================================================================
echo       LIVESTREAM MICRO-DAW - TRINH CAI DAT OBS RECEIVER PLUGIN (1-CLICK)
echo ==============================================================================
echo.
echo  Dang cai dat plugin VST2 va VST3 vao he thong cho OBS Studio...
echo.

:: ----------------------------------------------------------------------------
:: 2. Locate Plugin Sources (Priority: plugins\ folder right next to .bat)
:: ----------------------------------------------------------------------------
set "SRC_VST3="
set "SRC_VST2="

if exist "%~dp0plugins\LiveStream OBS Receiver.vst3" set "SRC_VST3=%~dp0plugins\LiveStream OBS Receiver.vst3"
if exist "%~dp0plugins\LiveStream OBS Receiver.dll" set "SRC_VST2=%~dp0plugins\LiveStream OBS Receiver.dll"

if not defined SRC_VST3 (
    if exist "%~dp0plugins\vst\LiveStream OBS Receiver.vst3" set "SRC_VST3=%~dp0plugins\vst\LiveStream OBS Receiver.vst3"
    if exist "%~dp0LiveStream OBS Receiver.vst3" set "SRC_VST3=%~dp0LiveStream OBS Receiver.vst3"
    if exist "%~dp0build\LiveStreamOBSReceiver_artefacts\Release\VST3\LiveStream OBS Receiver.vst3" set "SRC_VST3=%~dp0build\LiveStreamOBSReceiver_artefacts\Release\VST3\LiveStream OBS Receiver.vst3"
)

if not defined SRC_VST2 (
    if exist "%~dp0plugins\vst\LiveStream OBS Receiver.dll" set "SRC_VST2=%~dp0plugins\vst\LiveStream OBS Receiver.dll"
    if exist "%~dp0LiveStream OBS Receiver.dll" set "SRC_VST2=%~dp0LiveStream OBS Receiver.dll"
    if exist "%~dp0build\LiveStreamOBSReceiver_artefacts\Release\VST\LiveStream OBS Receiver.dll" set "SRC_VST2=%~dp0build\LiveStreamOBSReceiver_artefacts\Release\VST\LiveStream OBS Receiver.dll"
)

if not defined SRC_VST3 if not defined SRC_VST2 (
    echo [LOI] Khong tim thay tep plugin trong thu muc 'plugins\'!
    echo Vui long kiem tra lai thu muc 'plugins/' ngay canh file .bat nay.
    echo.
    pause
    exit /b 1
)

:: ----------------------------------------------------------------------------
:: 3. Copy VST3 Plugin
:: ----------------------------------------------------------------------------
set "DEST_VST3=%CommonProgramFiles%\VST3\LiveStream OBS Receiver.vst3"
if not exist "%CommonProgramFiles%\VST3" mkdir "%CommonProgramFiles%\VST3" >nul 2>&1

echo [*] Dang cai dat VST3 Plugin...
if defined SRC_VST3 (
    if exist "%SRC_VST3%" (
        if not exist "%DEST_VST3%" mkdir "%DEST_VST3%" >nul 2>&1
        xcopy "%SRC_VST3%" "%DEST_VST3%" /E /I /Y /Q >nul 2>&1
        if %errorlevel% equ 0 (
            echo  [OK] VST3: "%DEST_VST3%"
        ) else (
            echo  [!] Khong the copy VST3 (Vui long tat OBS neu dang mo!)
        )
    )
) else (
    echo  [-] Bo qua VST3 (Khong tim thay nguon)
)

echo.

:: ----------------------------------------------------------------------------
:: 4. Copy VST2 Plugin (.dll) to standard OBS VST folders
:: ----------------------------------------------------------------------------
echo [*] Dang cai dat VST2 Plugin (.dll)...
if defined SRC_VST2 (
    if exist "%SRC_VST2%" (
        :: 1. C:\Program Files\Common Files\VST2
        if not exist "%CommonProgramFiles%\VST2" mkdir "%CommonProgramFiles%\VST2" >nul 2>&1
        copy /Y "%SRC_VST2%" "%CommonProgramFiles%\VST2\LiveStream OBS Receiver.dll" >nul 2>&1
        if %errorlevel% equ 0 echo  [OK] VST2: "%CommonProgramFiles%\VST2\LiveStream OBS Receiver.dll"

        :: 2. C:\Program Files\VstPlugins
        if not exist "%ProgramFiles%\VstPlugins" mkdir "%ProgramFiles%\VstPlugins" >nul 2>&1
        copy /Y "%SRC_VST2%" "%ProgramFiles%\VstPlugins\LiveStream OBS Receiver.dll" >nul 2>&1
        if %errorlevel% equ 0 echo  [OK] VST2: "%ProgramFiles%\VstPlugins\LiveStream OBS Receiver.dll"

        :: 3. C:\Program Files\Steinberg\VstPlugins
        if not exist "%ProgramFiles%\Steinberg\VstPlugins" mkdir "%ProgramFiles%\Steinberg\VstPlugins" >nul 2>&1
        copy /Y "%SRC_VST2%" "%ProgramFiles%\Steinberg\VstPlugins\LiveStream OBS Receiver.dll" >nul 2>&1
        if %errorlevel% equ 0 echo  [OK] VST2: "%ProgramFiles%\Steinberg\VstPlugins\LiveStream OBS Receiver.dll"
    )
) else (
    echo  [-] Bo qua VST2 (Khong tim thay nguon)
)

echo.
echo ==============================================================================
echo                          HOAN TAT CAI DAT PLUGIN!
echo ==============================================================================
echo.
echo  Huong dan su dung tren OBS Studio:
echo   1. Mo OBS Studio (khoi dong lai neu OBS dang mo).
echo   2. Chuot phai vao Audio Source (Mic/Aux) ^> Chon "Filters" (Bo loc).
echo   3. Bam dau [+] ^> Chon "VST 2.x Plug-in".
echo   4. Trong danh sach tha xuong, chon: "LiveStream OBS Receiver".
echo   5. Tieng tu Micro-DAW se duoc truyen truc tiep vao OBS khong do tre!
echo.
echo ==============================================================================
echo.
pause
