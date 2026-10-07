# Package PRO Release Script
$ErrorActionPreference = "Stop"

$workspaceDir = Split-Path -Parent $PSScriptRoot
Set-Location $workspaceDir

# Ensure PRO directories exist
if (!(Test-Path "Hosi Micro Daw PRO")) { New-Item -ItemType Directory -Path "Hosi Micro Daw PRO" -Force | Out-Null }
if (!(Test-Path "Hosi Micro Daw PRO Setup")) { New-Item -ItemType Directory -Path "Hosi Micro Daw PRO Setup" -Force | Out-Null }

Write-Host ">>> [1/4] Copying new PRO binary, DLLs & docs to folders..." -ForegroundColor Cyan
$sourceExe = "build\LiveStreamMicroDAW_PRO_artefacts\Release\LiveStream Micro-DAW PRO.exe"
Copy-Item $sourceExe "LiveStream Micro-DAW PRO.exe" -Force
Copy-Item $sourceExe "Hosi Micro Daw PRO\LiveStream Micro-DAW PRO.exe" -Force

$wv2Dll = "third_party\webview2\build\native\x64\WebView2Loader.dll"
if (Test-Path $wv2Dll) {
    Copy-Item $wv2Dll "WebView2Loader.dll" -Force
    Copy-Item $wv2Dll "Hosi Micro Daw PRO\WebView2Loader.dll" -Force
}

# Clean up standalone exe from Setup folder to prevent user confusion
if (Test-Path "Hosi Micro Daw PRO Setup\LiveStream Micro-DAW PRO.exe") {
    Remove-Item "Hosi Micro Daw PRO Setup\LiveStream Micro-DAW PRO.exe" -Force
}

# Copy documentation and assets
Copy-Item "README.md" "Hosi Micro Daw PRO\README.md" -Force
Copy-Item "README_EN.md" "Hosi Micro Daw PRO\README_EN.md" -Force
Copy-Item "HUONG_DAN_SU_DUNG.txt" "Hosi Micro Daw PRO\HUONG_DAN_SU_DUNG.txt" -Force
Copy-Item "README.md" "Hosi Micro Daw PRO Setup\README.md" -Force
Copy-Item "README_EN.md" "Hosi Micro Daw PRO Setup\README_EN.md" -Force
Copy-Item "HUONG_DAN_SU_DUNG.txt" "Hosi Micro Daw PRO Setup\HUONG_DAN_SU_DUNG.txt" -Force

if (Test-Path "sounds") {
    Copy-Item "sounds" "Hosi Micro Daw PRO\sounds" -Recurse -Force
}

Write-Host ">>> [2/4] Building Inno Setup PRO installer..." -ForegroundColor Cyan
& ".\installer\build_installer_pro.ps1"

Write-Host ">>> [3/4] Copying setup installer to Hosi Micro Daw PRO Setup..." -ForegroundColor Cyan
Copy-Item "LiveStream_Micro_DAW_PRO_Setup.exe" "Hosi Micro Daw PRO Setup\LiveStream_Micro_DAW_PRO_Setup.exe" -Force

Write-Host ">>> [4/4] Creating PRO ZIP and RAR release packages..." -ForegroundColor Cyan
$winrar = "C:\Program Files\WinRAR\WinRAR.exe"
if (Test-Path $winrar) {
    Write-Host "Using WinRAR at: $winrar" -ForegroundColor Green
    if (Test-Path "Hosi Micro Daw PRO Portable.rar") { Remove-Item "Hosi Micro Daw PRO Portable.rar" -Force }
    if (Test-Path "Hosi Micro Daw PRO Setup.rar") { Remove-Item "Hosi Micro Daw PRO Setup.rar" -Force }
    Start-Process -FilePath $winrar -ArgumentList "a -r -ep1 -ibck ""$workspaceDir\Hosi Micro Daw PRO Portable.rar"" ""$workspaceDir\Hosi Micro Daw PRO\*""" -Wait
    Start-Process -FilePath $winrar -ArgumentList "a -r -ep1 -ibck ""$workspaceDir\Hosi Micro Daw PRO Setup.rar"" ""$workspaceDir\Hosi Micro Daw PRO Setup\*""" -Wait
}

Write-Host "Generating PRO ZIP archives..." -ForegroundColor Green
if (Test-Path "Hosi Micro Daw PRO Portable.zip") { Remove-Item "Hosi Micro Daw PRO Portable.zip" -Force }
if (Test-Path "Hosi Micro Daw PRO Setup.zip") { Remove-Item "Hosi Micro Daw PRO Setup.zip" -Force }
Compress-Archive -Path ".\Hosi Micro Daw PRO\*" -DestinationPath ".\Hosi Micro Daw PRO Portable.zip" -Force
Compress-Archive -Path ".\Hosi Micro Daw PRO Setup\*" -DestinationPath ".\Hosi Micro Daw PRO Setup.zip" -Force

Write-Host "`n>>> PRO RELEASE PACKAGE SUMMARY:" -ForegroundColor Cyan
Get-Item "LiveStream Micro-DAW PRO.exe", "LiveStream_Micro_DAW_PRO_Setup.exe", "Hosi Micro Daw PRO Portable.zip", "Hosi Micro Daw PRO Setup.zip", "Hosi Micro Daw PRO Portable.rar", "Hosi Micro Daw PRO Setup.rar" -ErrorAction SilentlyContinue | Select-Object Name, @{Name="Size(MB)";Expression={[math]::Round($_.Length/1MB,2)}}, LastWriteTime | Format-Table -AutoSize

