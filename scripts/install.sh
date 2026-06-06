#!/bin/bash
# install.sh — download and install the latest nomacs-decklink plugin
#
# Usage:
#   curl -fsSL https://raw.githubusercontent.com/OWNER/nomacs-decklink/main/scripts/install.sh | bash
#
# Or after cloning:
#   bash scripts/install.sh

set -euo pipefail

REPO="OWNER/nomacs-decklink"          # ← replace with your GitHub user/repo
PLUGIN_NAME="libnomacs-decklink.dylib"
PLUGIN_DIR="/Applications/nomacs.app/Contents/PlugIns/nomacs"

# -----------------------------------------------------------------------
# Preflight checks
# -----------------------------------------------------------------------
if [[ "$(uname -s)" != "Darwin" ]]; then
    echo "Error: This plugin is macOS only." >&2
    exit 1
fi

if [[ "$(uname -m)" != "arm64" ]]; then
    echo "Error: This plugin requires an Apple Silicon Mac (arm64)." >&2
    exit 1
fi

if [[ ! -d "/Applications/nomacs.app" ]]; then
    echo "Error: nomacs.app not found at /Applications/nomacs.app" >&2
    echo "Install nomacs 3.22.1 from https://nomacs.org first." >&2
    exit 1
fi

if [[ ! -d "$PLUGIN_DIR" ]]; then
    echo "Creating plugin directory: $PLUGIN_DIR"
    mkdir -p "$PLUGIN_DIR"
fi

# Check Desktop Video is installed (DeckLink framework lives here)
DECKLINK_FRAMEWORK="/Library/Frameworks/DeckLinkAPI.framework"
if [[ ! -d "$DECKLINK_FRAMEWORK" ]]; then
    echo "Warning: DeckLinkAPI.framework not found at $DECKLINK_FRAMEWORK" >&2
    echo "Install Blackmagic Desktop Video from https://www.blackmagicdesign.com/support" >&2
    echo "The plugin will be installed but will not work until Desktop Video is installed." >&2
fi

# -----------------------------------------------------------------------
# Download
# -----------------------------------------------------------------------
DOWNLOAD_URL="https://github.com/${REPO}/releases/latest/download/${PLUGIN_NAME}"
DEST="${PLUGIN_DIR}/${PLUGIN_NAME}"

echo "Downloading ${PLUGIN_NAME} from ${DOWNLOAD_URL} ..."
curl -fsSL --progress-bar "$DOWNLOAD_URL" -o "$DEST"

# Remove macOS quarantine attribute so nomacs can load the plugin
xattr -d com.apple.quarantine "$DEST" 2>/dev/null || true

echo ""
echo "Installed: $DEST"
echo "Restart nomacs — the DeckLink Output plugin will appear under Plugins."
