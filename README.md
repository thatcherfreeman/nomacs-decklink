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

### macOS (Apple Silicon)

**Requirements:** Apple Silicon Mac, [nomacs 3.22.1](https://nomacs.org), [Blackmagic Desktop Video](https://www.blackmagicdesign.com/support).

```sh
curl -fsSL https://raw.githubusercontent.com/thatcherfreeman/nomacs-decklink/main/scripts/install.sh | bash
```

Or download `libnomacs-decklink.dylib` from the [Releases page](https://github.com/thatcherfreeman/nomacs-decklink/releases/latest) and drop it into `/Applications/nomacs.app/Contents/PlugIns/nomacs/` yourself.

### Windows (x64)

**Requirements:** Windows 10/11 x64, [nomacs 3.22.1](https://nomacs.org), [Blackmagic Desktop Video](https://www.blackmagicdesign.com/support).

```powershell
irm https://raw.githubusercontent.com/thatcherfreeman/nomacs-decklink/main/scripts/install.ps1 | iex
```

Or download `nomacs-decklink.dll` from the [Releases page](https://github.com/thatcherfreeman/nomacs-decklink/releases/latest) and drop it into `C:\Program Files\nomacs\plugins\nomacs\` yourself.

Restart nomacs after installing — the plugin appears under **Plugins → DeckLink Output**.

---

## Platform

**macOS** (Apple Silicon arm64) and **Windows** (x64). Intel Mac is untested but may work. Linux is not supported.

---

## Prerequisites

### macOS prerequisites

#### 1. Xcode

Install Xcode from the Mac App Store. The full Xcode app is required — Command Line Tools alone will not work because they ship an older libc++ that is missing symbols needed by Homebrew Qt 6.

After installing, accept the license and make sure `xcode-select` points to Xcode:

```sh
sudo xcode-select --switch /Applications/Xcode.app/Contents/Developer
```

#### 2. Homebrew packages

```sh
brew install cmake qt
```

#### 3. nomacs (macOS)

Download and install **nomacs 3.22.1** from [nomacs.org](https://nomacs.org) and place it in `/Applications/nomacs.app`.

You also need the **nomacs source code** for its header files:

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

#### 4. Blackmagic Desktop Video (macOS)

Install **Desktop Video** from [Blackmagic's support page](https://www.blackmagicdesign.com/support). This installs the DeckLink drivers and `DeckLinkAPI.framework` that the plugin loads at runtime.

The macOS DeckLink SDK headers are already committed to this repository under `vendor/DeckLinkAPI/Mac/include/`. You do not need to download them separately.

---

### Windows prerequisites

#### 1. Visual Studio 2022

Install **Visual Studio 2022** with the "Desktop development with C++" workload, which includes the MSVC compiler and CMake integration.

#### 2. Qt 6 for Windows

Download and install **Qt 6.7** (or the version nomacs was built against) from [qt.io](https://www.qt.io/download-qt-installer). Install the `MSVC 2019 64-bit` component.

After installing, note the Qt installation path (e.g. `C:\Qt\6.7.0\msvc2019_64`) — you will need it for CMake.

#### 3. nomacs (Windows)

Download and install **nomacs 3.22.1** from [nomacs.org](https://nomacs.org). By default it installs to `C:\Program Files\nomacs`.

You also need the **nomacs source code** for its header files:

```powershell
git clone https://github.com/nomacs/nomacs.git
```

#### 4. Blackmagic Desktop Video (Windows)

Install **Desktop Video** from [Blackmagic's support page](https://www.blackmagicdesign.com/support). This registers the DeckLink COM server that the plugin loads at runtime.

#### 5. DeckLink SDK headers (Windows) — **required before building**

The Windows DeckLink SDK headers are **not committed** to this repository (the Mac headers are; see `vendor/DeckLinkAPI/Mac/include/`). You must obtain them from Blackmagic:

1. Download **Blackmagic Desktop Video SDK 16.0** from [blackmagicdesign.com/support](https://www.blackmagicdesign.com/support) (Windows version).
2. Extract the archive.
3. Copy the contents of `Win/include/` into `vendor/DeckLinkAPI/Win/include/` in this repository.

The files needed are: `DeckLinkAPI_h.h`, `DeckLinkAPIDispatch.cpp`, `DeckLinkAPI_i.c`, and `DeckLinkAPIVersion.h`. See `vendor/DeckLinkAPI/Win/include/README.md` for details.

---

## Build

### macOS build

```sh
cmake \
  -S /path/to/nomacs-decklink \
  -B /path/to/nomacs-decklink/build \
  -G "Unix Makefiles" \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_OSX_SYSROOT=/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk

cmake --build /path/to/nomacs-decklink/build -j8
```

> **Why the explicit sysroot?** Homebrew Qt 6 is built against the Xcode SDK. If CMake picks up the Command Line Tools SDK instead, the build fails with a `__builtin_ctzg` error. Passing `-DCMAKE_OSX_SYSROOT=…` forces the correct SDK. This has no effect on your other projects.

### Windows build

Open a **Developer Command Prompt for VS 2022** (or use `cmake-gui`):

```powershell
cmake -S C:\path\to\nomacs-decklink `
      -B C:\path\to\nomacs-decklink\build `
      -G "Visual Studio 17 2022" -A x64 `
      -DCMAKE_PREFIX_PATH="C:\Qt\6.7.0\msvc2019_64" `
      -DNOMACS_DIR="C:\Program Files\nomacs" `
      "-DNOMACS_INCLUDE_DIRECTORY=C:\path\to\nomacs\ImageLounge\src\DkCore;C:\path\to\nomacs\ImageLounge\src\DkGui;C:\path\to\nomacs\ImageLounge\src"

cmake --build C:\path\to\nomacs-decklink\build --config Release -j8
```

> **nomacsCore.lib** — if the nomacs Windows release does not include a `.lib` import library, CMake will warn and the build will fail at link time. In that case, generate one with `scripts/make_importlib.ps1` (see the script for usage) or add `NOMACS_DIR` to a `CMakeUser.txt` pointing to a directory that contains the `.lib`.

---

## Install

### macOS install

```sh
cmake --install /path/to/nomacs-decklink/build
```

This copies `libnomacs-decklink.dylib` into `/Applications/nomacs.app/Contents/PlugIns/nomacs/`.

You do not need `sudo` as long as you own the nomacs app bundle. If you installed nomacs system-wide (owned by root), prefix the install command with `sudo`.

### Windows install

```powershell
cmake --install C:\path\to\nomacs-decklink\build --config Release
```

This copies `nomacs-decklink.dll` into `C:\Program Files\nomacs\plugins\nomacs\`. Run from an elevated prompt if that directory requires admin rights.

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

# macOS: path to the installed nomacs app bundle
set(NOMACS_APP "/Applications/nomacs.app")

# Windows: path to the nomacs installation directory
set(NOMACS_DIR "C:/Program Files/nomacs")

# Path to the platform DeckLink API headers
# (defaults to vendor/DeckLinkAPI/Mac/include or vendor/DeckLinkAPI/Win/include)
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
