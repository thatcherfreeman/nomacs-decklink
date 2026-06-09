// DkDeckLinkGuids.cpp — Windows COM GUID definitions.
//
// Replaces DeckLinkAPI_i.c (generated from Blackmagic's IDL) so that no
// Blackmagic-owned compiled source files are required to build the plugin.
//
// GUIDs are extracted from the DeckLinkAPI 16.0 IDL files (which are
// permissively licensed by Blackmagic Design).  Only the GUIDs actually
// referenced by our plugin code are defined here; the linker only needs
// definitions for symbols that are actually used.
//
// SPDX-License-Identifier: GPL-2.0-or-later

#ifdef _WIN32

// INITGUID causes DEFINE_GUID (from <guiddef.h>) to emit a definition
// (DECLSPEC_SELECTANY const GUID) rather than a mere declaration.
#define INITGUID
#include <guiddef.h>

// -----------------------------------------------------------------------
// Factory / iterator
// -----------------------------------------------------------------------

// IDeckLinkIterator  {50FB36CD-3063-4B73-BDBB-958087F2D8BA}
DEFINE_GUID(IID_IDeckLinkIterator,
    0x50FB36CD, 0x3063, 0x4B73, 0xBD, 0xBB, 0x95, 0x80, 0x87, 0xF2, 0xD8, 0xBA);

// CDeckLinkIterator  {BA6C6F44-6DA5-4DCE-94AA-EE2D1372A676}
DEFINE_GUID(CLSID_CDeckLinkIterator,
    0xBA6C6F44, 0x6DA5, 0x4DCE, 0x94, 0xAA, 0xEE, 0x2D, 0x13, 0x72, 0xA6, 0x76);

// IDeckLink (device)  {C418FBDD-0587-48ED-8FE5-640F0A14AF91}
DEFINE_GUID(IID_IDeckLink,
    0xC418FBDD, 0x0587, 0x48ED, 0x8F, 0xE5, 0x64, 0x0F, 0x0A, 0x14, 0xAF, 0x91);

// -----------------------------------------------------------------------
// Output interfaces
// -----------------------------------------------------------------------

// IDeckLinkOutput (SDK 16.0)  {5F227C95-39D7-46C7-8B7D-9C81795FBBE4}
DEFINE_GUID(IID_IDeckLinkOutput,
    0x5F227C95, 0x39D7, 0x46C7, 0x8B, 0x7D, 0x9C, 0x81, 0x79, 0x5F, 0xBB, 0xE4);

// IDeckLinkOutput_v15_3_1  {1A8077F1-9FE2-4533-8147-2294305E253F}
DEFINE_GUID(IID_IDeckLinkOutput_v15_3_1,
    0x1A8077F1, 0x9FE2, 0x4533, 0x81, 0x47, 0x22, 0x94, 0x30, 0x5E, 0x25, 0x3F);

// IDeckLinkOutput_v14_2_1  {BE2D9020-461E-442F-84B7-E949CB953B9D}
DEFINE_GUID(IID_IDeckLinkOutput_v14_2_1,
    0xBE2D9020, 0x461E, 0x442F, 0x84, 0xB7, 0xE9, 0x49, 0xCB, 0x95, 0x3B, 0x9D);

// -----------------------------------------------------------------------
// Configuration interfaces
// -----------------------------------------------------------------------

// IDeckLinkConfiguration (SDK 16.0)  {5A68FFD4-1C12-4EDE-A6D2-45451D385FC1}
DEFINE_GUID(IID_IDeckLinkConfiguration,
    0x5A68FFD4, 0x1C12, 0x4EDE, 0xA6, 0xD2, 0x45, 0x45, 0x1D, 0x38, 0x5F, 0xC1);

// IDeckLinkConfiguration_v15_3_1  {912F634B-2D4E-40A4-8AAB-8D80B73F1289}
DEFINE_GUID(IID_IDeckLinkConfiguration_v15_3_1,
    0x912F634B, 0x2D4E, 0x40A4, 0x8A, 0xAB, 0x8D, 0x80, 0xB7, 0x3F, 0x12, 0x89);

// IDeckLinkConfiguration_v10_11  {EF90380B-4AE5-4346-9077-E288E149F129}
DEFINE_GUID(IID_IDeckLinkConfiguration_v10_11,
    0xEF90380B, 0x4AE5, 0x4346, 0x90, 0x77, 0xE2, 0x88, 0xE1, 0x49, 0xF1, 0x29);

// IDeckLinkConfiguration_v10_9  {CB71734A-FE37-4E8D-8E13-802133A1C3F2}
DEFINE_GUID(IID_IDeckLinkConfiguration_v10_9,
    0xCB71734A, 0xFE37, 0x4E8D, 0x8E, 0x13, 0x80, 0x21, 0x33, 0xA1, 0xC3, 0xF2);

// IDeckLinkConfiguration_v10_4  {1E69FCF6-4203-4936-8076-2A9F4CFD50CB}
DEFINE_GUID(IID_IDeckLinkConfiguration_v10_4,
    0x1E69FCF6, 0x4203, 0x4936, 0x80, 0x76, 0x2A, 0x9F, 0x4C, 0xFD, 0x50, 0xCB);

// -----------------------------------------------------------------------
// Profile-attributes interfaces
// -----------------------------------------------------------------------

// IDeckLinkProfileAttributes (SDK 16.0)  {F47551D7-AD22-47AF-BCFD-6BE88AA879D9}
DEFINE_GUID(IID_IDeckLinkProfileAttributes,
    0xF47551D7, 0xAD22, 0x47AF, 0xBC, 0xFD, 0x6B, 0xE8, 0x8A, 0xA8, 0x79, 0xD9);

// IDeckLinkProfileAttributes_v15_3_1  {17D4BF8E-4911-473A-80A0-731CF6FF345B}
DEFINE_GUID(IID_IDeckLinkProfileAttributes_v15_3_1,
    0x17D4BF8E, 0x4911, 0x473A, 0x80, 0xA0, 0x73, 0x1C, 0xF6, 0xFF, 0x34, 0x5B);

// IDeckLinkAttributes_v10_11  {ABC11843-D966-44CB-96E2-A1CB5D3135C4}
DEFINE_GUID(IID_IDeckLinkAttributes_v10_11,
    0xABC11843, 0xD966, 0x44CB, 0x96, 0xE2, 0xA1, 0xCB, 0x5D, 0x31, 0x35, 0xC4);

// -----------------------------------------------------------------------
// Video buffer / frame interfaces
// -----------------------------------------------------------------------

// IDeckLinkVideoBuffer (SDK 16.0)  {81F03D70-DE13-4B17-873A-C8AC9689C682}
DEFINE_GUID(IID_IDeckLinkVideoBuffer,
    0x81F03D70, 0xDE13, 0x4B17, 0x87, 0x3A, 0xC8, 0xAC, 0x96, 0x89, 0xC6, 0x82);

// IDeckLinkVideoBuffer_v15_3_1  {CCB4B64A-5C86-4E02-B778-885D352709FE}
DEFINE_GUID(IID_IDeckLinkVideoBuffer_v15_3_1,
    0xCCB4B64A, 0x5C86, 0x4E02, 0xB7, 0x78, 0x88, 0x5D, 0x35, 0x27, 0x09, 0xFE);

// IDeckLinkVideoFrame_v14_2_1  {3F716FE0-F023-4111-BE5D-EF4414C05B17}
DEFINE_GUID(IID_IDeckLinkVideoFrame_v14_2_1,
    0x3F716FE0, 0xF023, 0x4111, 0xBE, 0x5D, 0xEF, 0x44, 0x14, 0xC0, 0x5B, 0x17);

// -----------------------------------------------------------------------
// Callback interfaces
// -----------------------------------------------------------------------

// IDeckLinkVideoOutputCallback  {5BE6DF26-02CE-433E-99D9-9A87C3AC171F}
DEFINE_GUID(IID_IDeckLinkVideoOutputCallback,
    0x5BE6DF26, 0x02CE, 0x433E, 0x99, 0xD9, 0x9A, 0x87, 0xC3, 0xAC, 0x17, 0x1F);

// -----------------------------------------------------------------------
// Display mode interfaces (used by DeckLink internally, not QI'd by us
// directly, but included for completeness in case the runtime needs them)
// -----------------------------------------------------------------------

// IDeckLinkDisplayModeIterator  {9C88499F-F601-4021-B80B-032E4EB41C35}
DEFINE_GUID(IID_IDeckLinkDisplayModeIterator,
    0x9C88499F, 0xF601, 0x4021, 0xB8, 0x0B, 0x03, 0x2E, 0x4E, 0xB4, 0x1C, 0x35);

// IDeckLinkDisplayMode  {3EB2C1AB-0A3D-4523-A3AD-F40D7FB14E78}
DEFINE_GUID(IID_IDeckLinkDisplayMode,
    0x3EB2C1AB, 0x0A3D, 0x4523, 0xA3, 0xAD, 0xF4, 0x0D, 0x7F, 0xB1, 0x4E, 0x78);

// -----------------------------------------------------------------------
// Additional factory function CLSIDs / IIDs (used in DkDeckLinkDispatch.cpp)
// -----------------------------------------------------------------------

// IDeckLinkAPIInformation  {7BEA3C68-730D-4322-AF34-8A7152B532A4}
DEFINE_GUID(IID_IDeckLinkAPIInformation,
    0x7BEA3C68, 0x730D, 0x4322, 0xAF, 0x34, 0x8A, 0x71, 0x52, 0xB5, 0x32, 0xA4);

// CDeckLinkAPIInformation  {263CA19F-ED09-482E-9F9D-84005783A237}
DEFINE_GUID(CLSID_CDeckLinkAPIInformation,
    0x263CA19F, 0xED09, 0x482E, 0x9F, 0x9D, 0x84, 0x00, 0x57, 0x83, 0xA2, 0x37);

// IDeckLinkVideoConversion  {94C536D6-C821-42F5-A600-C66629955101}
DEFINE_GUID(IID_IDeckLinkVideoConversion,
    0x94C536D6, 0xC821, 0x42F5, 0xA6, 0x00, 0xC6, 0x66, 0x29, 0x95, 0x51, 0x01);

// CDeckLinkVideoConversion  {771AD62D-671F-4442-AC90-B070C541090A}
DEFINE_GUID(CLSID_CDeckLinkVideoConversion,
    0x771AD62D, 0x671F, 0x4442, 0xAC, 0x90, 0xB0, 0x70, 0xC5, 0x41, 0x09, 0x0A);

// IDeckLinkDiscovery  {CDBF631C-BC76-45FA-B44D-C55059BC6101}
DEFINE_GUID(IID_IDeckLinkDiscovery,
    0xCDBF631C, 0xBC76, 0x45FA, 0xB4, 0x4D, 0xC5, 0x50, 0x59, 0xBC, 0x61, 0x01);

// CDeckLinkDiscovery  {22FBFC33-8D07-495C-A5BF-DAB5EA9B82DB}
DEFINE_GUID(CLSID_CDeckLinkDiscovery,
    0x22FBFC33, 0x8D07, 0x495C, 0xA5, 0xBF, 0xDA, 0xB5, 0xEA, 0x9B, 0x82, 0xDB);

// IDeckLinkVideoFrameAncillaryPackets  {8A72D630-8070-4D05-8A93-E60C40EE088A}
DEFINE_GUID(IID_IDeckLinkVideoFrameAncillaryPackets,
    0x8A72D630, 0x8070, 0x4D05, 0x8A, 0x93, 0xE6, 0x0C, 0x40, 0xEE, 0x08, 0x8A);

// CDeckLinkVideoFrameAncillaryPackets  {6F47097E-B390-4650-BCB6-C4D52FAA1643}
DEFINE_GUID(CLSID_CDeckLinkVideoFrameAncillaryPackets,
    0x6F47097E, 0xB390, 0x4650, 0xBC, 0xB6, 0xC4, 0xD5, 0x2F, 0xAA, 0x16, 0x43);

#endif // _WIN32
