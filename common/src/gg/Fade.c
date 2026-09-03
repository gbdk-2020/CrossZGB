#pragma bank 1

#include "Fade.h"
#include "Palette.h"
#include "Math.h"
#include "main.h"

palette_color_t ZGB_Fading_BPal[16];
palette_color_t ZGB_Fading_SPal[16];
static INT8 BR, BB, BG;

palette_color_t UpdateColor(UINT8 i, palette_color_t col) {
	if (i == 0) return col;

	if (i == 3) return ZGB_Fading_SPal[0];

	INT8 R = (PAL_RED(col) << 3), G = (PAL_GREEN(col) << 3), B = (PAL_BLUE(col) << 3);

	INT8 DR = (INT8)(BR - R) >> 2, DG = (INT8)(BG - G) >> 2, DB = (INT8)(BB - B) >> 2;

	R += DR, G += DG, B += DB;
	if (i == 2) R += DR, G += DG, B += DB;

	return RGB(R >> 3, G >> 3, B >> 3);
}

void FadeStepColor(UINT8 i) {
	static palette_color_t palette[16];
	static palette_color_t palette_s[16];

	for (palette_color_t *src = ZGB_Fading_BPal, *dest = palette; src < ZGB_Fading_BPal + 16; ) {
		*dest++ = UpdateColor(i, *src++);
	}
	for (palette_color_t *src = ZGB_Fading_SPal, *dest = palette_s; src < ZGB_Fading_SPal + 16; ) {
		*dest++ = UpdateColor(i, *src++);
	}

	WAIT_WRITABLE_CRAM;	// avoid snow on screen

	set_bkg_palette(0, 1, palette);
	set_sprite_palette(0, 1, palette_s);

	DISPLAY_ON;
	SyncVBlank();
	SyncVBlank();
}

void FadeIn(void) BANKED {
	BR = (PAL_RED(ZGB_Fading_SPal[0]) << 3), BG = (PAL_GREEN(ZGB_Fading_SPal[0]) << 3), BB = (PAL_BLUE(ZGB_Fading_SPal[0]) << 3);
	FadeStepColor(0);
	FadeStepColor(1);
	FadeStepColor(2);
	FadeStepColor(3);
	DISPLAY_OFF;
}

void FadeOut(void) BANKED {
	BR = (PAL_RED(ZGB_Fading_SPal[0]) << 3), BG = (PAL_GREEN(ZGB_Fading_SPal[0]) << 3), BB = (PAL_BLUE(ZGB_Fading_SPal[0]) << 3);
	FadeStepColor(3);
	FadeStepColor(2);
	FadeStepColor(1);
	FadeStepColor(0);
}
