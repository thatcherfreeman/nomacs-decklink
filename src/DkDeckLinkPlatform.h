#pragma once

// Platform abstraction for DeckLink string types and COM initialisation.
//
// On macOS  BMDString == CFStringRef  (freed with CFRelease)
// On Windows BMDString == BSTR         (freed with SysFreeString)

#include <QString>

#ifdef __APPLE__

#include <CoreFoundation/CoreFoundation.h>

using BMDString = CFStringRef;

static inline QString bmdStringToQString(BMDString s)
{
    if (!s) return {};
    CFIndex len = CFStringGetMaximumSizeForEncoding(
                      CFStringGetLength(s), kCFStringEncodingUTF8) + 1;
    QByteArray buf(static_cast<int>(len), '\0');
    CFStringGetCString(s, buf.data(), len, kCFStringEncodingUTF8);
    return QString::fromUtf8(buf.constData());
}

static inline void bmdStringFree(BMDString s) { if (s) CFRelease(s); }

static inline void bmdPlatformInit()   {}
static inline void bmdPlatformUninit() {}

#elif defined(_WIN32)

#include <objbase.h>   // CoInitializeEx / CoUninitialize
#include <oleauto.h>   // SysStringLen / SysFreeString

using BMDString = BSTR;

static inline QString bmdStringToQString(BMDString s)
{
    if (!s) return {};
    return QString::fromWCharArray(s, static_cast<int>(SysStringLen(s)));
}

static inline void bmdStringFree(BMDString s) { if (s) SysFreeString(s); }

// Call once per thread before using the DeckLink COM factory.
static inline void bmdPlatformInit()
{
    CoInitializeEx(nullptr, COINIT_MULTITHREADED);
}
static inline void bmdPlatformUninit() { CoUninitialize(); }

#else
#error "Unsupported platform — only macOS and Windows are supported."
#endif
