@echo off
setlocal enabledelayedexpansion
title HOSI STUDIO - PUSH CLOUD DATA TO GITHUB PUBLIC

echo ==============================================================================
echo       HOSI MICRO-DAW - DONG BO DU LIEU CLOUD LEN GITHUB PUBLIC
echo ==============================================================================
echo.

cd /d "%~dp0"

:: 1. Kiem tra thu muc cloud
if not exist "cloud\songbook_cloud.json" (
    echo [LOI] Khong tim thay file cloud\songbook_cloud.json!
    echo Vui long kiem tra lai du lieu trong thu muc cloud/
    echo.
    pause
    exit /b 1
)

echo [1/5] Dang luu tru toan bo thay doi vao repo Private...
git add .
git commit -m "feat(cloud): update cloud songbook and presets" >nul 2>&1
git push private source-main:main >nul 2>&1

echo [2/5] Dang chuyen sang nhanh Public Release (release-public)...
git checkout release-public
if %ERRORLEVEL% NEQ 0 (
    echo [LOI] Khong the chuyen sang nhanh release-public!
    pause
    exit /b 1
)

echo [3/5] Dang nap du lieu cloud moi nhat tu source-main...
git checkout source-main -- cloud
git add cloud
git commit -m "feat(cloud): update hot trend songbook & artist presets" >nul 2>&1

echo [4/5] Dang day du lieu len GitHub Public (origin/main)...
git push -f origin release-public:main
if %ERRORLEVEL% NEQ 0 (
    echo [CANH BAO] Co loi khi push len origin. Dang thu lai...
    git push origin release-public:main
)

echo [5/5] Dang chuyen ve lai nhanh phat trien (source-main)...
git checkout source-main

echo.
echo ==============================================================================
echo   [OK] HOAN TAT DONG BO DU LIEU CLOUD LEN GITHUB PUBLIC THANH CONG!
echo ==============================================================================
echo.
echo Tat ca nguoi dung ban PRO v3.0 khi bam [Dong Bo Cloud] se nhan du lieu moi!
echo.
pause
