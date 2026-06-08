#pragma once
// Backward-compatibility shim for DeckLink SDK 15.x (pre-16.0 interfaces).
//
// SDK 16.0 changed the IIDs for IDeckLinkOutput, IDeckLinkConfiguration,
// IDeckLinkProfileAttributes, and IDeckLinkVideoBuffer.  A plugin built against
// the 16.0 SDK but running on DeckLink Desktop Video 15.x must fall back to the
// older IIDs when the 16.0 QueryInterface call fails.
//
// On macOS: the Blackmagic SDK ships ready-made .h compat headers; we include them.
// On Windows: DeckLinkAPI.idl #includes the compat IDLs, so MIDL inlines all compat
// interface classes into the generated DeckLinkAPI.h and all GUIDs into DeckLinkAPI_i.c.
// No extra definitions are needed here.

#include "DeckLinkAPI.h"

#ifdef __APPLE__

#include "DeckLinkAPIVideoOutput_v15_3_1.h"    // IDeckLinkOutput_v15_3_1 + IID
#include "DeckLinkAPIConfiguration_v15_3_1.h"  // IDeckLinkConfiguration_v15_3_1 + IID
#include "DeckLinkAPI_v15_3_1.h"               // IDeckLinkVideoBuffer_v15_3_1, IDeckLinkProfileAttributes_v15_3_1 + IIDs
#include "DeckLinkAPIVideoFrame_v14_2_1.h"      // IDeckLinkVideoFrame_v14_2_1 + IID

#elif defined(_WIN32)

// On Windows, DeckLinkAPI.idl #includes the compat IDLs (DeckLinkAPI_v15_3_1.idl,
// DeckLinkAPI_v14_2_1.idl, etc.) so MIDL inlines all compat interface class
// definitions into the generated DeckLinkAPI.h and all GUID definitions into
// DeckLinkAPI_i.c.  Nothing extra is needed here.

#endif // __APPLE__ / _WIN32
