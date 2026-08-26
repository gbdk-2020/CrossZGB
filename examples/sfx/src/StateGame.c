#include "Banks/SetAutoBank.h"

#include "ZGBMain.h"
#include "Scroll.h"
#include "Print.h"
#include "Sound.h"
#include "Keys.h"

DECLARE_SFX(sfx_fxhammer);
DECLARE_SFX(sfx_vgm);
#if defined(NINTENDO)
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
	if (KEY_TICKED(J_A)) ExecuteSFX(BANK(sfx_fxhammer), sfx_fxhammer, SFX_MUTE_MASK(sfx_fxhammer), SFX_PRIORITY_NORMAL);
	if (KEY_TICKED(J_B)) ExecuteSFX(BANK(sfx_vgm),      sfx_vgm,      SFX_MUTE_MASK(sfx_vgm),      SFX_PRIORITY_NORMAL);
#if defined(NINTENDO)
	if (KEY_TICKED(J_SELECT)) ExecuteSFX(BANK(sfx_wav), sfx_wav,      SFX_MUTE_MASK(sfx_wav),      SFX_PRIORITY_NORMAL);
	if (KEY_TICKED(J_START))  PlayFx(CHANNEL_1, 10, 0x2C, 0xD8, 0xF3, 0x62, 0xC7);
#endif
}

void DESTROY(void) {
}