#pragma once
// Backward-compatibility shim for DeckLink SDK pre-16.0 interfaces.
//
// SDK 16.0 changed IIDs for IDeckLinkOutput, IDeckLinkConfiguration,
// IDeckLinkProfileAttributes, and IDeckLinkVideoBuffer.  The IDeckLinkOutput
// vtable also changed between the 14.x and 15.x SDK eras, so a typed shim
// (DkOutputInterface) dispatches calls to whichever pointer was obtained.
//
// On macOS: the Blackmagic SDK ships ready-made .h compat headers; we include them.
// On Windows: DeckLinkAPI.idl #includes the compat IDLs, so MIDL inlines all compat
// interface classes into the generated DeckLinkAPI.h and all GUIDs into DeckLinkAPI_i.c.
// No extra definitions are needed here.

#include "DeckLinkAPI.h"
#include "DkDeckLinkPlatform.h"

#ifdef __APPLE__

// Output interfaces
#include "DeckLinkAPIVideoOutput_v15_3_1.h"    // IDeckLinkOutput_v15_3_1 + IID
#include "DeckLinkAPIVideoOutput_v14_2_1.h"    // IDeckLinkOutput_v14_2_1, IDeckLinkVideoOutputCallback_v14_2_1,
                                               // IDeckLinkMutableVideoFrame_v14_2_1 + IIDs
                                               // (also pulls in DeckLinkAPIVideoFrame_v14_2_1.h)

// Configuration interfaces (vtable identical across all versions; only IID differs)
#include "DeckLinkAPIConfiguration_v15_3_1.h"  // IDeckLinkConfiguration_v15_3_1 + IID
#include "DeckLinkAPIConfiguration_v10_11.h"   // IDeckLinkConfiguration_v10_11 + IID
#include "DeckLinkAPIConfiguration_v10_9.h"    // IDeckLinkConfiguration_v10_9 + IID
#include "DeckLinkAPIConfiguration_v10_4.h"    // IDeckLinkConfiguration_v10_4 + IID

// Attributes / profile-attributes interfaces (vtable identical; only IID and name differ)
#include "DeckLinkAPI_v15_3_1.h"               // IDeckLinkProfileAttributes_v15_3_1,
                                               // IDeckLinkVideoBuffer_v15_3_1 + IIDs
#include "DeckLinkAPI_v10_11.h"                // IDeckLinkAttributes_v10_11 + IID

#elif defined(_WIN32)

// On Windows, DeckLinkAPI.idl #includes the compat IDLs (DeckLinkAPI_v15_3_1.idl,
// DeckLinkAPI_v14_2_1.idl, etc.) so MIDL inlines all compat interface class
// definitions into the generated DeckLinkAPI.h.  GUIDs are defined in
// DkDeckLinkGuids.cpp — nothing extra is needed here.

#endif // __APPLE__ / _WIN32

// -----------------------------------------------------------------------
// DkOutputInterface
//
// IDeckLinkOutput's vtable layout changed between the 14.x and 15.x SDK eras
// (SetVideoOutputFrameMemoryAllocator was removed; CreateVideoFrameWithBuffer
// and RowBytesForPixelFormat were added), so reinterpret_cast is not safe
// across that boundary.  This shim holds whichever versioned pointer was
// obtained and routes each call to the correct vtable.
//
// The 15.3.1 and 16.0 interfaces share an identical vtable (only the IID
// changed), so both are stored in v16 and dispatched identically.
// -----------------------------------------------------------------------

struct DkOutputInterface {
    IDeckLinkOutput         *v16 = nullptr;  // SDK 15.3.1 / 16.0 (same vtable)
    IDeckLinkOutput_v14_2_1 *v14 = nullptr;  // SDK 14.x

    bool valid() const { return v16 || v14; }

    void release() {
        if (v16) { v16->Release(); v16 = nullptr; }
        if (v14) { v14->Release(); v14 = nullptr; }
    }

    HRESULT GetDisplayModeIterator(IDeckLinkDisplayModeIterator **it) {
        return v16 ? v16->GetDisplayModeIterator(it) : v14->GetDisplayModeIterator(it);
    }

    // Same 7-parameter signature in both v14_2_1 and v15_3_1+
    HRESULT DoesSupportVideoMode(BMDVideoConnection conn, BMDDisplayMode mode,
                                  BMDPixelFormat fmt, BMDVideoOutputConversionMode conv,
                                  BMDSupportedVideoModeFlags flags,
                                  BMDDisplayMode *actualMode, BMDBool *supported) {
        return v16 ? v16->DoesSupportVideoMode(conn, mode, fmt, conv, flags, actualMode, supported)
                   : v14->DoesSupportVideoMode(conn, mode, fmt, conv, flags, actualMode, supported);
    }

    HRESULT EnableVideoOutput(BMDDisplayMode mode, BMDVideoOutputFlags flags) {
        return v16 ? v16->EnableVideoOutput(mode, flags) : v14->EnableVideoOutput(mode, flags);
    }

    HRESULT DisableVideoOutput() {
        return v16 ? v16->DisableVideoOutput() : v14->DisableVideoOutput();
    }

    // v14 CreateVideoFrame returns IDeckLinkMutableVideoFrame_v14_2_1**; we
    // store the result as IDeckLinkMutableVideoFrame* (same pointer value).
    // Only GetWidth/Height/RowBytes (slots 1-3) and QueryInterface are ever
    // called on this pointer, and those slots are identical in both vtables.
    HRESULT CreateVideoFrame(int32_t w, int32_t h, int32_t rb, BMDPixelFormat fmt,
                              BMDFrameFlags flags, IDeckLinkMutableVideoFrame **outFrame) {
        if (v16) return v16->CreateVideoFrame(w, h, rb, fmt, flags, outFrame);
        IDeckLinkMutableVideoFrame_v14_2_1 *f14 = nullptr;
        HRESULT hr = v14->CreateVideoFrame(w, h, rb, fmt, flags, &f14);
        *outFrame = reinterpret_cast<IDeckLinkMutableVideoFrame *>(f14);
        return hr;
    }

    // v14 ScheduleVideoFrame expects IDeckLinkVideoFrame_v14_2_1*; frames
    // created by v14 CreateVideoFrame are stored as IDeckLinkMutableVideoFrame*,
    // so we cast back to the original pointer type for the v14 path.
    HRESULT ScheduleVideoFrame(IDeckLinkMutableVideoFrame *frame,
                                BMDTimeValue t, BMDTimeValue dur, BMDTimeScale scale) {
        return v16 ? v16->ScheduleVideoFrame(frame, t, dur, scale)
                   : v14->ScheduleVideoFrame(
                         reinterpret_cast<IDeckLinkVideoFrame_v14_2_1 *>(frame), t, dur, scale);
    }

    // v14 expects IDeckLinkVideoOutputCallback_v14_2_1*; the vtable layout is
    // identical (same two methods, same order) so the reinterpret_cast is safe.
    HRESULT SetScheduledFrameCompletionCallback(IDeckLinkVideoOutputCallback *cb) {
        if (v16) return v16->SetScheduledFrameCompletionCallback(cb);
        return v14->SetScheduledFrameCompletionCallback(
            reinterpret_cast<IDeckLinkVideoOutputCallback_v14_2_1 *>(cb));
    }

    HRESULT StartScheduledPlayback(BMDTimeValue startTime, BMDTimeScale scale, double speed) {
        return v16 ? v16->StartScheduledPlayback(startTime, scale, speed)
                   : v14->StartScheduledPlayback(startTime, scale, speed);
    }

    HRESULT StopScheduledPlayback(BMDTimeValue stopTime, BMDTimeValue *actual,
                                   BMDTimeScale scale) {
        return v16 ? v16->StopScheduledPlayback(stopTime, actual, scale)
                   : v14->StopScheduledPlayback(stopTime, actual, scale);
    }
};
