# nomacs-decklink

A [nomacs](https://nomacs.org) plugin that outputs the currently displayed image as a clean-feed still to a [Blackmagic DeckLink](https://www.blackmagicdesign.com/products/decklink) SDI output card.

When active, every image you open or navigate to in nomacs is automatically pushed to the DeckLink output with no additional button presses. A configuration dialog lets you choose the output format before streaming begins.

**Supported options:**
- Output resolution and frame rate (whatever the card supports)
- Pixel format: 8-bit RGBA, 10-bit RGB (r210), 8-bit YUV 4:2:2 (2vuy), 10-bit YUV 4:2:2 (v210)
- Legal (SMPTE 64–940) or full range levels (for YUV and 10-bit RGB)
- Single or dual SDI link (when the card supports dual link)

**Platform:** macOS (Apple Silicon arm64) and Windows (x64). Intel Mac is untested but may work. Linux is not supported.

---
- [nomacs-decklink](#nomacs-decklink)
  - [Usage](#usage)
  - [macOS](#macos)
    - [Easy installation (pre-built binary)](#easy-installation-pre-built-binary)
    - [Build and installation](#build-and-installation)
  - [Windows](#windows)
    - [Easy installation (pre-built binary)](#easy-installation-pre-built-binary-1)
    - [Build and installation](#build-and-installation-1)
  - [Custom paths](#custom-paths)
  - [Troubleshooting](#troubleshooting)


---

## Usage

1. Launch nomacs and open an image.
2. Go to **Plugins → DeckLink Output → Start DeckLink Output**.
3. Select your device, output mode, pixel format, and levels, then click **OK**.
4. The current image appears on the DeckLink output immediately. Every subsequent image you open or navigate to is pushed automatically.
5. To stop, click **Stop Output** in the toolbar that appears at the top of the nomacs window, or quit nomacs.

> Not installed yet? Jump to [macOS](#macos) or [Windows](#windows) for install instructions, and see [Troubleshooting](#troubleshooting) if something goes wrong.

---

## macOS

### Easy installation (pre-built binary)

The easiest way to install — no compiler required.

**Requirements:** Apple Silicon Mac, [nomacs 3.22.1](https://nomacs.org), [Blackmagic Desktop Video](https://www.blackmagicdesign.com/support).

#### Install nomacs

Download the latest release from the [nomacs GitHub releases page](https://github.com/nomacs/nomacs/releases/latest). Grab the `nomacs-…-macOS-…-arm64-….zip`, unzip it, and drag `nomacs.app` into `/Applications`.

macOS Gatekeeper will block it on first launch because it is not notarised by Apple. To clear the quarantine flag, run this in Terminal after dragging the app to `/Applications`:

```sh
sudo xattr -dr com.apple.quarantine /Applications/nomacs.app
```

Then double-click the app normally — it will open without the "malware" warning.

#### Install the plugin

```sh
curl -fsSL https://raw.githubusercontent.com/thatcherfreeman/nomacs-decklink/main/scripts/install.sh | bash
```

Or download `libnomacs-decklink.dylib` from the [Releases page](https://github.com/thatcherfreeman/nomacs-decklink/releases/latest) and drop it into `/Applications/nomacs.app/Contents/PlugIns/nomacs/` yourself.

Restart nomacs after installing — the plugin appears under **Plugins → DeckLink Output**.

### Build and installation

Build the plugin from source and install it into your nomacs app bundle.

#### Prerequisites

**1. Xcode**

Install Xcode from the Mac App Store. The full Xcode app is required — Command Line Tools alone will not work because they ship an older libc++ that is missing symbols needed by Homebrew Qt 6.

After installing, accept the license and make sure `xcode-select` points to Xcode:

```sh
sudo xcode-select --switch /Applications/Xcode.app/Contents/Developer
```

**2. Homebrew packages**

```sh
brew install cmake qt
```

**3. nomacs**

Download and install **nomacs 3.22.1** or the latest release from [nomacs.org](https://nomacs.org) and place it in `/Applications/nomacs.app`.

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

If you want to put the nomacs source somewhere else, see [Custom paths](#custom-paths).

**4. Blackmagic Desktop Video**

Install **Desktop Video** from [Blackmagic's support page](https://www.blackmagicdesign.com/support). This installs the DeckLink drivers and `DeckLinkAPI.framework` that the plugin loads at runtime.

#### Build

```sh
cmake \
  -S . \
  -B ./build \
  -G "Unix Makefiles" \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_OSX_SYSROOT=/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk

cmake --build ./build -j8
```

> **Why the explicit sysroot?** Homebrew Qt 6 is built against the Xcode SDK. If CMake picks up the Command Line Tools SDK instead, the build fails with a `__builtin_ctzg` error. Passing `-DCMAKE_OSX_SYSROOT=…` forces the correct SDK. This has no effect on your other projects.

#### Install

```sh
cmake --install build
```

This copies `libnomacs-decklink.dylib` into `/Applications/nomacs.app/Contents/PlugIns/nomacs/`.

You do not need `sudo` as long as you own the nomacs app bundle. If you installed nomacs system-wide (owned by root), prefix the install command with `sudo`.

---

## Windows

### Easy installation (pre-built binary)

The easiest way to install — no compiler required.

**Requirements:** Windows 10/11 x64, [nomacs 3.22.1](https://nomacs.org), [Blackmagic Desktop Video](https://www.blackmagicdesign.com/support).

**Option A — one-liner (PowerShell).** Opens PowerShell and runs the install script, which finds nomacs automatically and places the plugin in the right folder:

```powershell
irm https://raw.githubusercontent.com/thatcherfreeman/nomacs-decklink/main/scripts/install.ps1 | iex
```

**Option B — manual.** Download `nomacs-decklink.dll` from the [Releases page](https://github.com/thatcherfreeman/nomacs-decklink/releases/latest). Then:

1. Find your nomacs installation folder. The default is `C:\Program Files\nomacs\`, but if you installed it elsewhere go to **Help → About** in nomacs and note the path shown there.
2. Inside that folder, create the directory `plugins\nomacs\` if it does not already exist.
3. Copy `nomacs-decklink.dll` into `plugins\nomacs\`.

Either way, restart nomacs after installing — the plugin appears under **Plugins → DeckLink Output**.

### Build and installation

Build the plugin from source and install it into your nomacs installation.

#### Prerequisites

**1. Visual Studio 2022**

Install **Visual Studio 2022** with the "Desktop development with C++" workload, which includes the MSVC compiler and CMake integration.

**2. Qt 6 for Windows**

The plugin must be built against the **same Qt version that nomacs ships**, so it stays binary-compatible with the Qt nomacs loads at runtime. nomacs **3.22.1** is built against **Qt 6.9.3**. You can confirm the version on your machine from the file properties of `Qt6Core.dll` in `C:\Program Files\nomacs\bin`.

You need the **MSVC 2022 64-bit** build of Qt. Install it either way:

*Recommended — no Qt account, command line.* [`aqtinstall`](https://github.com/miurahr/aqtinstall) downloads Qt straight from the official mirrors — the same tool the CI uses, so no login is required:

```powershell
python -m pip install --user aqtinstall
python -m aqt install-qt windows desktop 6.9.3 win64_msvc2022_64 --outputdir C:\Qt
```

This installs Qt to `C:\Qt\6.9.3\msvc2022_64`.

*Alternative — official installer.* Download the [Qt Online Installer](https://www.qt.io/download-qt-installer) (free Qt account required) and, in the component tree, select **Qt 6.9.3 → MSVC 2022 64-bit**.

Either way, note the resulting kit path (e.g. `C:\Qt\6.9.3\msvc2022_64`) — you pass it to CMake as `CMAKE_PREFIX_PATH`.

**3. nomacs**

Download and install **nomacs 3.22.1** from [nomacs.org](https://nomacs.org). By default it installs to `C:\Program Files\nomacs`.

You also need the **nomacs source code** for its header files. Clone it **next to** this repository, at the tag matching your installed nomacs:

```powershell
git clone --branch 3.22.1 https://github.com/nomacs/nomacs.git
```

The build expects this layout (the default include path points at `..\nomacs`):

```
parent-directory/
├── nomacs/            ← nomacs source (headers only needed)
└── nomacs-decklink/   ← this repository
```

If you clone it somewhere else, see [Custom paths](#custom-paths).

**4. Blackmagic Desktop Video**

Install **Desktop Video** from [Blackmagic's support page](https://www.blackmagicdesign.com/support). This registers the DeckLink COM server that the plugin loads at runtime.

#### Build

Open a **Developer PowerShell for VS 2022** (or *Developer Command Prompt*) so the MSVC tools — `cmake`, `dumpbin`, `lib.exe` — are on your PATH, and `cd` into the repository. For step 1 you also need it running **as Administrator**, because it writes into `C:\Program Files`.

**1. Generate `nomacsCore.lib` (if nomacs didn't ship it)**

The nomacs Windows release ships `nomacsCore.dll` but usually **no** `.lib` import library, which the linker needs. Generate one from the DLL with the helper script in this repo:

```powershell
mkdir "C:\Program Files\nomacs\lib"
dumpbin /exports "C:\Program Files\nomacs\bin\nomacsCore.dll" > exports.txt
powershell -File scripts\make_importlib.ps1 exports.txt nomacsCore "C:\Program Files\nomacs\lib\nomacsCore.lib"
```

If your nomacs already contains `nomacsCore.lib`, skip this. CMake looks for it at `<NOMACS_DIR>\lib\nomacsCore.lib` or `<NOMACS_DIR>\nomacsCore.lib`; without it the configure step warns and the build fails at link time.

**2. Configure and build**

From the repository root (adjust the Qt path to your installed kit):

```powershell
cmake -S . `
      -B .\build `
      -G "Visual Studio 17 2022" -A x64 `
      -DCMAKE_PREFIX_PATH="C:\Qt\6.9.3\msvc2022_64" `
      -DNOMACS_DIR="C:\Program Files\nomacs"

cmake --build .\build --config Release -j8
```

This assumes the nomacs source is cloned at `..\nomacs` (see prerequisite 3). If it lives elsewhere, point CMake at its headers by adding:

```powershell
      "-DNOMACS_INCLUDE_DIRECTORY=C:\path\to\nomacs\ImageLounge\src\DkCore;C:\path\to\nomacs\ImageLounge\src\DkGui;C:\path\to\nomacs\ImageLounge\src"
```

The build produces `build\Release\nomacs-decklink.dll`.

#### Install

```powershell
cmake --install build --config Release
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

## Troubleshooting

**"No DeckLink devices found" in the dialog**
Make sure Desktop Video is installed and your card is connected and recognised in System Settings → General → System Information → Hardware → Thunderbolt/USB4 or PCIe.

**"EnableVideoOutput failed"**
Another application (e.g. Media Express, Resolve) may have exclusive access to the card. Quit other DeckLink applications and try again.

**Output appears but is black until you navigate to a new image**
This should not happen with the current build. If it does, navigate to any other image and back; the next `updateImageContainer` call will refresh the output.

**Build fails with `__builtin_ctzg` undefined (macOS)**
You are using the Command Line Tools SDK. Pass `-DCMAKE_OSX_SYSROOT=…` as shown in the macOS [Build](#build) section.

**Build fails with `DkBaseWidgets.h` or `nmc_config.h` not found**
Check that all three nomacs header directories are on the include path (DkCore, DkGui, and the src root). See [Custom paths](#custom-paths).
