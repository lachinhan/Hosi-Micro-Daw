@echo off
chcp 65001 >nul
title ĐẨY PHIÊN BẢN LÊN GITHUB - HOSI MICRO-DAW
color 0b

echo ==============================================================================
echo        HOSI PROD - 1-CLICK ĐÓNG GÓI VÀ ĐẨY BẢN MỚI LÊN GITHUB
echo ==============================================================================
echo.

:: 1. Chạy đóng gói và đồng bộ các thư mục phân phối
echo [1/3] Đang cập nhật bộ cài đặt Setup và đóng gói file nén...
powershell -ExecutionPolicy Bypass -File "%~dp0tools\package_release.ps1"
if %errorlevel% neq 0 (
    color 0c
    echo [LỖI] Đóng gói thất bại!
    pause
    exit /b %errorlevel%
)

:: 2. Nhập ghi chú phiên bản (Commit Message)
echo.
echo ==============================================================================
set "commit_msg="
set /p commit_msg="Nhập nội dung cập nhật (Bấm Enter để dùng mặc định): "

if "%commit_msg%"=="" (
    set "commit_msg=Cap nhat phien ban LiveStream Micro-DAW moi nhat"
)

:: 3. Thực hiện Git commit và Push lên GitHub
echo.
echo [2/3] Đang thêm file vào Git và tạo commit...
git add .
git commit -m "%commit_msg%"

echo.
echo [3/3] Đang đẩy lên GitHub (https://github.com/lachinhan/Hosi-Micro-Daw)...
git push origin main

if %errorlevel% equ 0 (
    color 0a
    echo.
    echo ==============================================================================
    echo    ✓ CHÚC MỪNG! ĐÃ ĐẨY BẢN CẬP NHẬT LÊN GITHUB THÀNH CÔNG!
    echo    👉 Xem trực tiếp tại: https://github.com/lachinhan/Hosi-Micro-Daw
    echo ==============================================================================
) else (
    color 0c
    echo.
    echo [LỖI] Đẩy lên GitHub không thành công. Vui lòng kiểm tra lại kết nối mạng!
)

echo.
pause
