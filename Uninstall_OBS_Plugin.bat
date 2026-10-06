@echo off
setlocal EnableDelayedExpansion
title LiveStream Micro-DAW - OBS Plugin Uninstaller

:: ----------------------------------------------------------------------------
:: 1. Auto Request Administrator Privileges
:: ----------------------------------------------------------------------------
net session >nul 2>&1
if %errorlevel% neq 0 (
    echo Dang yeu cau quyen Administrator de go bo plugin...
    powershell -Command "Start-Process cmd -ArgumentList '/c cd /d \"\"%~dp0\"\" && call \"%~f0\" --elevated' -Verb RunAs"
    exit /b
)

:: Set UTF-8 encoding
chcp 65001 >nul
cls

echo ==============================================================================
echo       LIVESTREAM MICRO-DAW - GO BO OBS RECEIVER PLUGIN
echo ==============================================================================
echo.
echo  Dang xoa plugin VST2 va VST3 khoi he thong...
echo.

:: 1. Remove VST3
if exist "%CommonProgramFiles%\VST3\LiveStream OBS Receiver.vst3" (
    rmdir /S /Q "%CommonProgramFiles%\VST3\LiveStream OBS Receiver.vst3" >nul 2>&1
    echo  [OK] Da xoa: "%CommonProgramFiles%\VST3\LiveStream OBS Receiver.vst3"
)

:: 2. Remove VST2 (.dll)
if exist "%CommonProgramFiles%\VST2\LiveStream OBS Receiver.dll" (
    del /F /Q "%CommonProgramFiles%\VST2\LiveStream OBS Receiver.dll" >nul 2>&1
    echo  [OK] Da xoa: "%CommonProgramFiles%\VST2\LiveStream OBS Receiver.dll"
)
if exist "%ProgramFiles%\VstPlugins\LiveStream OBS Receiver.dll" (
    del /F /Q "%ProgramFiles%\VstPlugins\LiveStream OBS Receiver.dll" >nul 2>&1
    echo  [OK] Da xoa: "%ProgramFiles%\VstPlugins\LiveStream OBS Receiver.dll"
)
if exist "%ProgramFiles%\Steinberg\VstPlugins\LiveStream OBS Receiver.dll" (
    del /F /Q "%ProgramFiles%\Steinberg\VstPlugins\LiveStream OBS Receiver.dll" >nul 2>&1
    echo  [OK] Da xoa: "%ProgramFiles%\Steinberg\VstPlugins\LiveStream OBS Receiver.dll"
)

echo.
echo ==============================================================================
echo                          HOAN TAT GO BO PLUGIN!
echo ==============================================================================
echo.
pause
