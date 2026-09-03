#pragma bank 1

#include "Fade.h"
#include "Palette.h"
#include "Math.h"
#include "main.h"

palette_color_t ZGB_Fading_BPal[16];
palette_color_t ZGB_Fading_SPal[16];

palette_color_t UpdateColor(UINT8 i, palette_color_t col) {
	INT8 tmp;

	tmp = PAL_RED(ZGB_Fading_SPal[0]); 
	INT8 R = PAL_RED(col); 
	R = (R > tmp) ? MAX(tmp, R - i) : MIN(tmp, R + i);

	tmp = PAL_GREEN(ZGB_Fading_SPal[0]);
	INT8 G = PAL_GREEN(col); 
	G = (G > tmp) ? MAX(tmp, G - i) : MIN(tmp, G + i);

	tmp = PAL_BLUE(ZGB_Fading_SPal[0]);
	INT8 B = PAL_BLUE(col);
	B = (B > tmp) ? MAX(tmp, B - i) : MIN(tmp, B + i);

	return RGB(R, G, B);
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
	FadeStepColor(0);
	FadeStepColor(1);
	FadeStepColor(2);
	FadeStepColor(3);
	DISPLAY_OFF;
}

void FadeOut(void) BANKED {
	FadeStepColor(3);
	FadeStepColor(2);
	FadeStepColor(1);
	FadeStepColor(0);
}
