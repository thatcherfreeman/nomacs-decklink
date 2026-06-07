# install.ps1 — download and install the latest nomacs-decklink plugin on Windows
#
# Usage (run from PowerShell as a normal user):
#   irm https://raw.githubusercontent.com/thatcherfreeman/nomacs-decklink/main/scripts/install.ps1 | iex
#
# Or after cloning:
#   powershell -ExecutionPolicy Bypass -File scripts\install.ps1

$REPO        = "thatcherfreeman/nomacs-decklink"
$PLUGIN_NAME = "nomacs-decklink.dll"

# -----------------------------------------------------------------------
# Preflight checks
# -----------------------------------------------------------------------

# Find nomacs installation
$nomacsExe = Get-Command "nomacs.exe" -ErrorAction SilentlyContinue
if ($nomacsExe) {
    $nomacsDir = Split-Path $nomacsExe.Source
} else {
    $candidates = @(
        "$env:ProgramFiles\nomacs",
        "$env:LOCALAPPDATA\nomacs"
    )
    $nomacsDir = $candidates | Where-Object { Test-Path "$_\nomacs.exe" } | Select-Object -First 1
}

if (-not $nomacsDir) {
    Write-Error "nomacs.exe not found. Install nomacs 3.22.1 from https://nomacs.org first."
    exit 1
}
Write-Host "Found nomacs at: $nomacsDir"

# Check Desktop Video is installed (DeckLink COM server)
$deckLinkKey = "HKLM:\SOFTWARE\Blackmagic Design\Desktop Video"
if (-not (Test-Path $deckLinkKey)) {
    Write-Warning "Blackmagic Desktop Video does not appear to be installed."
    Write-Warning "The plugin will be installed but will not work until Desktop Video is installed."
    Write-Warning "Download from: https://www.blackmagicdesign.com/support"
}

# Create plugin directory if needed
$pluginDir = Join-Path $nomacsDir "plugins\nomacs"
if (-not (Test-Path $pluginDir)) {
    Write-Host "Creating plugin directory: $pluginDir"
    New-Item -ItemType Directory -Path $pluginDir | Out-Null
}

# -----------------------------------------------------------------------
# Download
# -----------------------------------------------------------------------
$downloadUrl = "https://github.com/$REPO/releases/latest/download/$PLUGIN_NAME"
$dest        = Join-Path $pluginDir $PLUGIN_NAME

Write-Host "Downloading $PLUGIN_NAME ..."
try {
    Invoke-WebRequest -Uri $downloadUrl -OutFile $dest -UseBasicParsing
} catch {
    Write-Error "Download failed: $_"
    exit 1
}

# Unblock the downloaded file (removes Mark-of-the-Web)
Unblock-File -Path $dest

Write-Host ""
Write-Host "Installed: $dest"
Write-Host "Restart nomacs — the DeckLink Output plugin will appear under Plugins."
