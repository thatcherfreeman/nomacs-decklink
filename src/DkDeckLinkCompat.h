#pragma once
// Backward-compatibility shim for DeckLink SDK 15.x (pre-16.0 interfaces).
//
// SDK 16.0 changed the IIDs for IDeckLinkOutput, IDeckLinkConfiguration,
// IDeckLinkProfileAttributes, and IDeckLinkVideoBuffer.  A plugin built against
// the 16.0 SDK but running on DeckLink Desktop Video 15.x must fall back to the
// older IIDs when the 16.0 QueryInterface call fails.  This header provides those
// IIDs and the two interface classes whose methods we call on the older type
// (IDeckLinkVideoBuffer_v15_3_1 and IDeckLinkVideoFrame_v14_2_1).
//
// On macOS the Blackmagic SDK ships ready-made .h compatibility headers; we
// simply include them.  On Windows the SDK ships only IDL files; we provide the
// interface definitions here, sourced from DeckLinkAPI_v15_3_1.idl and
// DeckLinkAPI_v14_2_1.idl.

#include "DeckLinkAPI.h"

#ifdef __APPLE__

#include "DeckLinkAPIVideoOutput_v15_3_1.h"    // IDeckLinkOutput_v15_3_1 + IID
#include "DeckLinkAPIConfiguration_v15_3_1.h"  // IDeckLinkConfiguration_v15_3_1 + IID
#include "DeckLinkAPI_v15_3_1.h"               // IDeckLinkVideoBuffer_v15_3_1, IDeckLinkProfileAttributes_v15_3_1 + IIDs
#include "DeckLinkAPIVideoFrame_v14_2_1.h"      // IDeckLinkVideoFrame_v14_2_1 + IID

#elif defined(_WIN32)

// ── GUIDs ─────────────────────────────────────────────────────────────────────
// Values sourced from DeckLinkAPI_v15_3_1.idl and DeckLinkAPI_v14_2_1.idl.

// {1A8077F1-9FE2-4533-8147-2294305E253F}  — IDeckLinkOutput as of SDK 15.x
static const GUID IID_IDeckLinkOutput_v15_3_1 =
    { 0x1A8077F1, 0x9FE2, 0x4533, { 0x81, 0x47, 0x22, 0x94, 0x30, 0x5E, 0x25, 0x3F } };

// {912F634B-2D4E-40A4-8AAB-8D80B73F1289}  — IDeckLinkConfiguration as of SDK 15.x
static const GUID IID_IDeckLinkConfiguration_v15_3_1 =
    { 0x912F634B, 0x2D4E, 0x40A4, { 0x8A, 0xAB, 0x8D, 0x80, 0xB7, 0x3F, 0x12, 0x89 } };

// {17D4BF8E-4911-473A-80A0-731CF6FF345B}  — IDeckLinkProfileAttributes as of SDK 15.x
static const GUID IID_IDeckLinkProfileAttributes_v15_3_1 =
    { 0x17D4BF8E, 0x4911, 0x473A, { 0x80, 0xA0, 0x73, 0x1C, 0xF6, 0xFF, 0x34, 0x5B } };

// {CCB4B64A-5C86-4E02-B778-885D352709FE}  — IDeckLinkVideoBuffer, introduced in SDK 15.3.1
static const GUID IID_IDeckLinkVideoBuffer_v15_3_1 =
    { 0xCCB4B64A, 0x5C86, 0x4E02, { 0xB7, 0x78, 0x88, 0x5D, 0x35, 0x27, 0x09, 0xFE } };

// {3F716FE0-F023-4111-BE5D-EF4414C05B17}  — IDeckLinkVideoFrame as of SDK 14.x–15.2
static const GUID IID_IDeckLinkVideoFrame_v14_2_1 =
    { 0x3F716FE0, 0xF023, 0x4111, { 0xBE, 0x5D, 0xEF, 0x44, 0x14, 0xC0, 0x5B, 0x17 } };

// The interface class definitions for IDeckLinkVideoBuffer_v15_3_1 and
// IDeckLinkVideoFrame_v14_2_1 are already present in the MIDL-generated
// DeckLinkAPI.h (the main IDL imports the compat IDLs).  We only need the
// IID constants, which are not emitted into DeckLinkAPI_i.c.

#endif // __APPLE__ / _WIN32
