# install.ps1 — download and install the latest nomacs-decklink plugin on Windows
#
# Usage (run from PowerShell as a normal user):
#   irm https://raw.githubusercontent.com/thatcherfreeman/nomacs-decklink/main/scripts/install.ps1 | iex
#
# Or after cloning:
#   powershell -ExecutionPolicy Bypass -File scripts\install.ps1
#
# Note: this script uses a function + `return` (never `exit`) so that a failed
# preflight check does NOT close the PowerShell window when run via `irm | iex`.

function Install-NomacsDeckLink {
    $REPO        = "thatcherfreeman/nomacs-decklink"
    $PLUGIN_NAME = "nomacs-decklink.dll"

    # -------------------------------------------------------------------
    # Locate nomacs
    # -------------------------------------------------------------------
    $nomacsDir = $null

    # 1) On PATH?
    $nomacsExe = Get-Command "nomacs.exe" -ErrorAction SilentlyContinue
    if ($nomacsExe) {
        $nomacsDir = Split-Path $nomacsExe.Source
    }

    # 2) Known install locations (the Windows installer puts nomacs.exe in bin\)
    if (-not $nomacsDir) {
        $candidates = @(
            "$env:ProgramFiles\nomacs\bin",
            "$env:ProgramFiles\nomacs",
            "${env:ProgramFiles(x86)}\nomacs\bin",
            "${env:ProgramFiles(x86)}\nomacs",
            "$env:LOCALAPPDATA\nomacs\bin",
            "$env:LOCALAPPDATA\nomacs"
        )
        $nomacsDir = $candidates | Where-Object { $_ -and (Test-Path "$_\nomacs.exe") } | Select-Object -First 1
    }

    # 3) Recursive fallback search under Program Files
    if (-not $nomacsDir) {
        foreach ($root in @($env:ProgramFiles, ${env:ProgramFiles(x86)}, $env:LOCALAPPDATA)) {
            if (-not $root) { continue }
            $hit = Get-ChildItem -Path $root -Filter "nomacs.exe" -Recurse -ErrorAction SilentlyContinue |
                   Select-Object -First 1
            if ($hit) { $nomacsDir = Split-Path $hit.FullName; break }
        }
    }

    if (-not $nomacsDir) {
        Write-Warning "nomacs.exe not found. Install nomacs 3.22.1 from https://nomacs.org first."
        return
    }
    Write-Host "Found nomacs at: $nomacsDir"

    # Plugins live alongside the other plugin DLLs in <nomacsDir>\plugins
    $pluginDir = Join-Path $nomacsDir "plugins"
    $dest      = Join-Path $pluginDir $PLUGIN_NAME

    # -------------------------------------------------------------------
    # Elevation: writing to Program Files needs admin
    # -------------------------------------------------------------------
    $isAdmin = ([Security.Principal.WindowsPrincipal]`
                [Security.Principal.WindowsIdentity]::GetCurrent()`
               ).IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)

    $needsAdmin = $false
    try {
        if (-not (Test-Path $pluginDir)) {
            New-Item -ItemType Directory -Path $pluginDir -ErrorAction Stop | Out-Null
        }
        # probe write access
        $probe = Join-Path $pluginDir ".write_test_$PID"
        New-Item -ItemType File -Path $probe -ErrorAction Stop | Out-Null
        Remove-Item $probe -Force -ErrorAction SilentlyContinue
    } catch {
        $needsAdmin = $true
    }

    if ($needsAdmin -and -not $isAdmin) {
        Write-Warning "Installing to '$pluginDir' requires Administrator rights."
        Write-Host "Relaunching elevated..."
        # Re-download self to a temp file and run it elevated (works for `irm | iex` too)
        $tmp = Join-Path $env:TEMP "install-nomacs-decklink.ps1"
        try {
            Invoke-WebRequest -UseBasicParsing `
                -Uri "https://raw.githubusercontent.com/$REPO/main/scripts/install.ps1" `
                -OutFile $tmp -ErrorAction Stop
            Start-Process powershell.exe -Verb RunAs -Wait -ArgumentList `
                "-NoProfile","-ExecutionPolicy","Bypass","-File","`"$tmp`""
        } catch {
            Write-Warning "Could not auto-elevate: $_"
            Write-Host "Re-run PowerShell as Administrator and try again."
        }
        return
    }

    # -------------------------------------------------------------------
    # Check Desktop Video (DeckLink COM server)
    # -------------------------------------------------------------------
    $deckLinkKey = "HKLM:\SOFTWARE\Blackmagic Design\Desktop Video"
    if (-not (Test-Path $deckLinkKey)) {
        Write-Warning "Blackmagic Desktop Video does not appear to be installed."
        Write-Warning "The plugin will be installed but will not work until Desktop Video is installed."
        Write-Warning "Download from: https://www.blackmagicdesign.com/support"
    }

    # -------------------------------------------------------------------
    # Download
    # -------------------------------------------------------------------
    $downloadUrl = "https://github.com/$REPO/releases/latest/download/$PLUGIN_NAME"
    Write-Host "Downloading $PLUGIN_NAME ..."
    try {
        Invoke-WebRequest -Uri $downloadUrl -OutFile $dest -UseBasicParsing -ErrorAction Stop
    } catch {
        Write-Warning "Download failed: $_"
        return
    }

    # Unblock the downloaded file (removes Mark-of-the-Web)
    Unblock-File -Path $dest

    Write-Host ""
    Write-Host "Installed: $dest"
    Write-Host "Restart nomacs - the DeckLink Output plugin will appear under Plugins."
}

Install-NomacsDeckLink
