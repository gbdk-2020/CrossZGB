#include "Banks/SetAutoBank.h"

#include "ZGBMain.h"
#include "Scroll.h"
#include "Print.h"
#include "Sound.h"
#include "Keys.h"

// declare the SFX assets

// SFX in the FXHammer format (same file for the each platform)
DECLARE_SFX(sfx_fxhammer);
// SFX in the VGM format (different files for the different platforms under the same name)
DECLARE_SFX(sfx_vgm);
#if defined(NINTENDO)
// SFX in the WAV format (feature is available only on the Game Boy)
DECLARE_SFX(sfx_wav);
#endif

IMPORT_MAP(map);
IMPORT_FONT(font);

void START(void) {
	InitScroll(BANK(map), &map, 0, 0);
	INIT_FONT_EX(font, PRINT_BKG);
	PRINT(0, 0, "SFX DEMO");

	PRINT(0, 2, "FXHammer");
	PRINT(0, 3, "VGM");
	PRINT(0, 4, "WAV");
	PRINT(0, 5, "LEGACY");

#if defined(NINTENDO)
	PRINT(10, 4, "SEL");
	PRINT(10, 5, "STA");
#elif defined(SEGA)
	PRINT(10, 4, "---");
	PRINT(10, 5, "---");
#endif
	PRINT(10, 2, "A");
	PRINT(10, 3, "B");
}

void UPDATE(void) {
	// Play FXHammer - the cross-platform format (with few differences in sounding between the GB/SN7, but very similar)
	if (KEY_TICKED(J_A)) ExecuteSFX(BANK(sfx_fxhammer), sfx_fxhammer, SFX_MUTE_MASK(sfx_fxhammer), SFX_PRIORITY_NORMAL);
	// play VGM - GB and SN7 assets MUST be different, because sound chips are not compatible
	if (KEY_TICKED(J_B)) ExecuteSFX(BANK(sfx_vgm),      sfx_vgm,      SFX_MUTE_MASK(sfx_vgm),      SFX_PRIORITY_NORMAL);
#if defined(NINTENDO)
	// play WAV (1920Hz mono) - only supported on the Game Boy
	if (KEY_TICKED(J_SELECT)) ExecuteSFX(BANK(sfx_wav), sfx_wav,      SFX_MUTE_MASK(sfx_wav),      SFX_PRIORITY_NORMAL);
	// play using legacy PlayFx() function - this is Game Boy only, obsolete and not recommended for using, but still valid
	if (KEY_TICKED(J_START))  PlayFx(CHANNEL_1, 10, 0x2C, 0xD8, 0xF3, 0x62, 0xC7);
#endif
}

void DESTROY(void) {
}