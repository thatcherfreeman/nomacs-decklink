// DkDeckLinkDispatch.cpp — GPL-clean runtime loader for the DeckLink SDK.
//
// Replaces the Blackmagic-provided DeckLinkAPIDispatch.cpp on both platforms.
//
// macOS:   loads DeckLinkAPI.framework at first use via dlopen/dlsym.
//          No CoreFoundation bundle API needed; plain POSIX dlopen suffices.
//
// Windows: creates COM objects via CoCreateInstance.  The CLSIDs and IIDs
//          are defined in DkDeckLinkGuids.cpp (which replaces DeckLinkAPI_i.c).
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "DeckLinkAPI.h"

// -----------------------------------------------------------------------
// macOS — dlopen-based loader
// -----------------------------------------------------------------------
#ifdef __APPLE__

#include <dlfcn.h>
#include <mutex>

static std::once_flag s_once;
static void          *s_handle        = nullptr;

using CreateIteratorFn = IDeckLinkIterator *(*)();
static CreateIteratorFn s_createIterator = nullptr;

static void loadDeckLink()
{
    s_handle = dlopen(
        "/Library/Frameworks/DeckLinkAPI.framework/DeckLinkAPI",
        RTLD_LAZY | RTLD_LOCAL);
    if (!s_handle)
        return;

    // The framework exports a versioned symbol (same name the SDK dispatch uses).
    s_createIterator = reinterpret_cast<CreateIteratorFn>(
        dlsym(s_handle, "CreateDeckLinkIteratorInstance_0004"));
}

bool IsDeckLinkAPIPresent()
{
    std::call_once(s_once, loadDeckLink);
    return s_handle != nullptr;
}

IDeckLinkIterator *CreateDeckLinkIteratorInstance()
{
    std::call_once(s_once, loadDeckLink);
    return s_createIterator ? s_createIterator() : nullptr;
}

// -----------------------------------------------------------------------
// Windows — CoCreateInstance-based loader
// -----------------------------------------------------------------------
#elif defined(_WIN32)

#include <objbase.h>

static HRESULT comCreate(REFCLSID clsid, REFIID iid, void **ppv)
{
    return CoCreateInstance(clsid, nullptr, CLSCTX_ALL, iid, ppv);
}

bool IsDeckLinkAPIPresent()
{
    IDeckLinkIterator *p = nullptr;
    HRESULT hr = comCreate(CLSID_CDeckLinkIterator,
                           IID_IDeckLinkIterator,
                           reinterpret_cast<void **>(&p));
    if (SUCCEEDED(hr) && p) { p->Release(); return true; }
    return false;
}

IDeckLinkIterator *CreateDeckLinkIteratorInstance()
{
    IDeckLinkIterator *p = nullptr;
    comCreate(CLSID_CDeckLinkIterator,
              IID_IDeckLinkIterator,
              reinterpret_cast<void **>(&p));
    return p;
}

IDeckLinkAPIInformation *CreateDeckLinkAPIInformationInstance()
{
    IDeckLinkAPIInformation *p = nullptr;
    comCreate(CLSID_CDeckLinkAPIInformation,
              IID_IDeckLinkAPIInformation,
              reinterpret_cast<void **>(&p));
    return p;
}

IDeckLinkVideoConversion *CreateVideoConversionInstance()
{
    IDeckLinkVideoConversion *p = nullptr;
    comCreate(CLSID_CDeckLinkVideoConversion,
              IID_IDeckLinkVideoConversion,
              reinterpret_cast<void **>(&p));
    return p;
}

IDeckLinkDiscovery *CreateDeckLinkDiscoveryInstance()
{
    IDeckLinkDiscovery *p = nullptr;
    comCreate(CLSID_CDeckLinkDiscovery,
              IID_IDeckLinkDiscovery,
              reinterpret_cast<void **>(&p));
    return p;
}

IDeckLinkVideoFrameAncillaryPackets *CreateVideoFrameAncillaryPacketsInstance()
{
    IDeckLinkVideoFrameAncillaryPackets *p = nullptr;
    comCreate(CLSID_CDeckLinkVideoFrameAncillaryPackets,
              IID_IDeckLinkVideoFrameAncillaryPackets,
              reinterpret_cast<void **>(&p));
    return p;
}

#endif // __APPLE__ / _WIN32
