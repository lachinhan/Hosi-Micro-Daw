$pkgDir = 'e:\Hosi Micro Daw\third_party\webview2'
if (!(Test-Path $pkgDir)) { New-Item -ItemType Directory -Path $pkgDir -Force | Out-Null }
$zipPath = Join-Path $pkgDir 'webview2.zip'
Invoke-WebRequest -Uri 'https://www.nuget.org/api/v2/package/Microsoft.Web.WebView2/1.0.2792.45' -OutFile $zipPath -UseBasicParsing
Expand-Archive -Path $zipPath -DestinationPath $pkgDir -Force
if (Test-Path $zipPath) { Remove-Item $zipPath -Force }
Write-Host 'Done WebView2 Setup'