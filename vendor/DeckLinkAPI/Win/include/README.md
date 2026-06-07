# Windows DeckLink SDK Headers

This directory must contain the Windows-specific DeckLink API headers before
building on Windows or running Windows CI.

## How to populate this directory

1. Download **Blackmagic Desktop Video SDK 16.0** (or later) from
   https://www.blackmagicdesign.com/support
   Choose the Windows version.

2. Extract the SDK archive.

3. Copy the contents of `Win/include/` from the SDK into this directory.

The files you need are:

- `DeckLinkAPI_h.h`           — main API header (Windows, uses BSTR/COM)
- `DeckLinkAPIDispatch.cpp`   — COM factory dispatch (creates IDeckLinkIterator via CoCreateInstance)
- `DeckLinkAPI_i.c`           — COM CLSID/IID definitions
- `DeckLinkAPIVersion.h`      — SDK version constants

The Windows SDK headers use COM types (BSTR, HRESULT, REFIID) instead of
CoreFoundation types (CFStringRef) and therefore cannot be shared with the
Mac headers in `../Mac/include/`.

## Why not committed

The Blackmagic SDK EULA permits using these files to build products but does
not grant redistribution rights beyond what is necessary. The Mac headers are
committed because they are needed to build in CI without additional downloads.
The Windows headers are not committed; you must obtain them from Blackmagic
and add them locally (or to your CI environment).
