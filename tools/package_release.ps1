# Package Release Script
$ErrorActionPreference = "Stop"

$workspaceDir = Split-Path -Parent $PSScriptRoot
Set-Location $workspaceDir

Write-Host ">>> [1/4] Copying new binary & docs to folders..." -ForegroundColor Cyan
$sourceExe = "build\LiveStreamMicroDAW_artefacts\Release\LiveStream Micro-DAW.exe"
Copy-Item $sourceExe "LiveStream Micro-DAW.exe" -Force
Copy-Item $sourceExe "Hosi Micro Daw\LiveStream Micro-DAW.exe" -Force

# Clean up standalone exe from Setup folder to prevent user confusion
if (Test-Path "Hosi Micro Daw Setup\LiveStream Micro-DAW.exe") {
    Remove-Item "Hosi Micro Daw Setup\LiveStream Micro-DAW.exe" -Force
}

# Copy documentation and assets
Copy-Item "README.md" "Hosi Micro Daw\README.md" -Force
Copy-Item "README_EN.md" "Hosi Micro Daw\README_EN.md" -Force
Copy-Item "HUONG_DAN_SU_DUNG.txt" "Hosi Micro Daw\HUONG_DAN_SU_DUNG.txt" -Force
Copy-Item "README.md" "Hosi Micro Daw Setup\README.md" -Force
Copy-Item "README_EN.md" "Hosi Micro Daw Setup\README_EN.md" -Force
Copy-Item "HUONG_DAN_SU_DUNG.txt" "Hosi Micro Daw Setup\HUONG_DAN_SU_DUNG.txt" -Force

if (Test-Path "sounds") {
    Copy-Item "sounds" "Hosi Micro Daw\sounds" -Recurse -Force
}

Write-Host ">>> [2/4] Building Inno Setup installer..." -ForegroundColor Cyan
& ".\installer\build_installer.ps1"

Write-Host ">>> [3/4] Copying setup installer to Hosi Micro Daw Setup..." -ForegroundColor Cyan
Copy-Item "LiveStream_Micro_DAW_Setup.exe" "Hosi Micro Daw Setup\LiveStream_Micro_DAW_Setup.exe" -Force

Write-Host ">>> [4/4] Creating ZIP and RAR release packages..." -ForegroundColor Cyan
$winrar = "C:\Program Files\WinRAR\WinRAR.exe"
if (Test-Path $winrar) {
    Write-Host "Using WinRAR at: $winrar" -ForegroundColor Green
    if (Test-Path "Hosi Micro Daw Portable.rar") { Remove-Item "Hosi Micro Daw Portable.rar" -Force }
    if (Test-Path "Hosi Micro Daw Setup.rar") { Remove-Item "Hosi Micro Daw Setup.rar" -Force }
    Start-Process -FilePath $winrar -ArgumentList "a -r -ep1 -ibck ""$workspaceDir\Hosi Micro Daw Portable.rar"" ""$workspaceDir\Hosi Micro Daw\*""" -Wait
    Start-Process -FilePath $winrar -ArgumentList "a -r -ep1 -ibck ""$workspaceDir\Hosi Micro Daw Setup.rar"" ""$workspaceDir\Hosi Micro Daw Setup\*""" -Wait
}

Write-Host "Generating ZIP archives..." -ForegroundColor Green
if (Test-Path "Hosi Micro Daw Portable.zip") { Remove-Item "Hosi Micro Daw Portable.zip" -Force }
if (Test-Path "Hosi Micro Daw Setup.zip") { Remove-Item "Hosi Micro Daw Setup.zip" -Force }
Compress-Archive -Path ".\Hosi Micro Daw\*" -DestinationPath ".\Hosi Micro Daw Portable.zip" -Force
Compress-Archive -Path ".\Hosi Micro Daw Setup\*" -DestinationPath ".\Hosi Micro Daw Setup.zip" -Force

Write-Host "`n>>> RELEASE PACKAGE SUMMARY:" -ForegroundColor Cyan
Get-Item "LiveStream Micro-DAW.exe", "LiveStream_Micro_DAW_Setup.exe", "Hosi Micro Daw Portable.zip", "Hosi Micro Daw Setup.zip", "Hosi Micro Daw Portable.rar", "Hosi Micro Daw Setup.rar" -ErrorAction SilentlyContinue | Select-Object Name, @{Name="Size(MB)";Expression={[math]::Round($_.Length/1MB,2)}}, LastWriteTime | Format-Table -AutoSize
