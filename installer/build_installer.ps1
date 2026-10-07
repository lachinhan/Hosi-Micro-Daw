# ==============================================================================
# PowerShell script to build the 1-Click Installer Setup Executable
# ==============================================================================
$ErrorActionPreference = "Stop"

$workspaceDir = Split-Path -Parent $PSScriptRoot
Set-Location $workspaceDir

Write-Host ">>> [1/3] Checking prerequisites..." -ForegroundColor Cyan

# Find Inno Setup Compiler
$isccCandidates = @(
    "C:\Users\PC\AppData\Local\Programs\Inno Setup 6\ISCC.exe",
    "C:\Program Files (x86)\Inno Setup 6\ISCC.exe",
    "C:\Program Files\Inno Setup 6\ISCC.exe",
    (Get-Command ISCC.exe -ErrorAction SilentlyContinue | Select-Object -ExpandProperty Source)
)

$isccPath = $null
foreach ($cand in $isccCandidates) {
    if ($cand -and (Test-Path $cand)) {
        $isccPath = $cand
        break
    }
}

if (-not $isccPath) {
    Write-Error "ISCC.exe (Inno Setup Compiler) was not found! Please install Inno Setup 6."
}

Write-Host "Found Inno Setup Compiler at: $isccPath" -ForegroundColor Green

# Ensure target artifacts exist
$exePath = Join-Path $workspaceDir "LiveStream Micro-DAW.exe"
if (-not (Test-Path $exePath)) {
    Write-Error "Main executable not found: $exePath"
}

Write-Host ">>> [2/3] Compiling LiveStream_Micro_DAW_Setup.exe..." -ForegroundColor Cyan
$issFile = Join-Path $workspaceDir "installer\installer_script.iss"
& "$isccPath" "$issFile"

if ($LASTEXITCODE -ne 0) {
    Write-Error "Inno Setup compilation failed with exit code $LASTEXITCODE"
}

Write-Host ">>> [3/3] Deploying setup executable to project root..." -ForegroundColor Cyan
$setupSrc = Join-Path $workspaceDir "installer_output\LiveStream_Micro_DAW_Setup.exe"
$setupDst = Join-Path $workspaceDir "LiveStream_Micro_DAW_Setup.exe"

if (Test-Path $setupSrc) {
    Copy-Item -Path $setupSrc -Destination $setupDst -Force
    $fileItem = Get-Item $setupDst
    Write-Host "Successfully generated: $($fileItem.FullName) ($([math]::Round($fileItem.Length / 1MB, 2)) MB)" -ForegroundColor Green
} else {
    Write-Error "Setup executable was not generated at $setupSrc"
}
