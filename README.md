# nomacs-decklink

A [nomacs](https://nomacs.org) plugin that outputs the currently displayed image as a clean-feed still to a [Blackmagic DeckLink](https://www.blackmagicdesign.com/products/decklink) SDI output card.

When active, every image you open or navigate to in nomacs is automatically pushed to the DeckLink output with no additional button presses. A configuration dialog lets you choose the output format before streaming begins.

**Supported options:**
- Output resolution and frame rate (whatever the card supports)
- Pixel format: 8-bit RGBA, 10-bit RGB (r210), 8-bit YUV 4:2:2 (2vuy), 10-bit YUV 4:2:2 (v210)
- Legal (SMPTE 64–940) or full range levels (for YUV and 10-bit RGB)
- Single or dual SDI link (when the card supports dual link)

---

## Installation (pre-built binary)

The easiest way to install — no compiler required.

**Requirements:** Apple Silicon Mac, [nomacs 3.22.1](https://nomacs.org), [Blackmagic Desktop Video](https://www.blackmagicdesign.com/support).

```sh
curl -fsSL https://raw.githubusercontent.com/thatcherfreeman/nomacs-decklink/main/scripts/install.sh | bash
```

Or download `libnomacs-decklink.dylib` from the [Releases page](https://github.com/thatcherfreeman/nomacs-decklink/releases/latest) and drop it into `/Applications/nomacs.app/Contents/PlugIns/nomacs/` yourself.

Restart nomacs after installing — the plugin appears under **Plugins → DeckLink Output**.

---

## Platform

**macOS, Apple Silicon (arm64).** Intel Mac is untested but may work with minor changes. Windows and Linux are not supported.

---

## Prerequisites

Install all of the following before building.

### 1. Xcode

Install Xcode from the Mac App Store. The full Xcode app is required — Command Line Tools alone will not work because they ship an older libc++ that is missing symbols needed by Homebrew Qt 6.

After installing, accept the license and make sure `xcode-select` points to Xcode:

```sh
sudo xcode-select --switch /Applications/Xcode.app/Contents/Developer
```

### 2. Homebrew packages

```sh
brew install cmake qt
```

### 3. nomacs

Download and install **nomacs 3.22.1** from [nomacs.org](https://nomacs.org) and place it in `/Applications/nomacs.app`.

> The plugin links against `libnomacsCore-3.22.1.dylib` inside the app bundle. If your installed nomacs is a different version, adjust `NOMACS_CORE_LIB` in `CMakeLists.txt`.

You also need the **nomacs source code** for its header files (the plugin does not link against your build of nomacs — it uses the pre-built app — but the source headers are required at compile time).

```sh
git clone https://github.com/nomacs/nomacs.git
```

The build system expects the nomacs source to be in a directory called `nomacs` **next to** this repository:

```
parent-directory/
├── nomacs/            ← nomacs source (headers only needed)
└── nomacs-decklink/   ← this repository
```

If you want to put the nomacs source somewhere else, see [Custom paths](#custom-paths) below.

### 4. Blackmagic Desktop Video

Install the **Desktop Video** software package from [Blackmagic's support page](https://www.blackmagicdesign.com/support). This installs the DeckLink drivers and the `DeckLinkAPI.framework` that the plugin loads at runtime.

The DeckLink SDK headers are already included in this repository under `Blackmagic DeckLink SDK 16.0/`. You do not need to download them separately.

---

## Build

```sh
# From anywhere — use absolute paths for -S and -B
cmake \
  -S /path/to/nomacs-decklink \
  -B /path/to/nomacs-decklink/build \
  -G "Unix Makefiles" \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_OSX_SYSROOT=/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk

cmake --build /path/to/nomacs-decklink/build -j8
```

> **Why the explicit sysroot?** Homebrew Qt 6 is built against the Xcode SDK. If CMake picks up the Command Line Tools SDK instead, the build fails with a `__builtin_ctzg` error. Passing `-DCMAKE_OSX_SYSROOT=…` forces the correct SDK. This has no effect on your other projects.

---

## Install

```sh
cmake --install /path/to/nomacs-decklink/build
```

This copies `libnomacs-decklink.dylib` into `/Applications/nomacs.app/Contents/PlugIns/nomacs/`.

You do not need `sudo` as long as you own the nomacs app bundle. If you installed nomacs system-wide (owned by root), prefix the install command with `sudo`.

---

## Custom paths

If your directory layout differs from the defaults, create a file called `CMakeUser.txt` in the repository root (it is gitignored) and set any of these variables:

```cmake
# CMakeUser.txt — override defaults here

# Path to the nomacs source tree (only headers are used)
set(NOMACS_INCLUDE_DIRECTORY
    "/absolute/path/to/nomacs/ImageLounge/src/DkCore"
    "/absolute/path/to/nomacs/ImageLounge/src/DkGui"
    "/absolute/path/to/nomacs/ImageLounge/src")

# Path to the installed nomacs app bundle
set(NOMACS_APP "/Applications/nomacs.app")

# Path to the DeckLink API headers (default: vendor/DeckLinkAPI/ in this repo)
set(DECKLINK_INCLUDE_DIR "/path/to/DeckLink SDK/Mac/include")
```

---

## Usage

1. Launch nomacs and open an image.
2. Go to **Plugins → DeckLink Output → Start DeckLink Output**.
3. Select your device, output mode, pixel format, and levels, then click **OK**.
4. The current image appears on the DeckLink output immediately. Every subsequent image you open or navigate to is pushed automatically.
5. To stop, click **Stop Output** in the toolbar that appears at the top of the nomacs window, or quit nomacs.

---

## Troubleshooting

**"No DeckLink devices found" in the dialog**
Make sure Desktop Video is installed and your card is connected and recognised in System Settings → General → System Information → Hardware → Thunderbolt/USB4 or PCIe.

**"EnableVideoOutput failed"**
Another application (e.g. Media Express, Resolve) may have exclusive access to the card. Quit other DeckLink applications and try again.

**Output appears but is black until you navigate to a new image**
This should not happen with the current build. If it does, navigate to any other image and back; the next `updateImageContainer` call will refresh the output.

**Build fails with `__builtin_ctzg` undefined**
You are using the Command Line Tools SDK. Pass `-DCMAKE_OSX_SYSROOT=…` as shown in [Build](#build).

**Build fails with `DkBaseWidgets.h` or `nmc_config.h` not found**
Check that all three nomacs header directories are on the include path (DkCore, DkGui, and the src root). See [Custom paths](#custom-paths).
