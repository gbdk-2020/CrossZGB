#pragma bank 1

#include "Fade.h"
#include "Palette.h"
#include "Math.h"

inline UINT8 FadeInOp(UINT8 c, UINT8 i) {
	return (c < i) ? 0: (c - i);
}

static UINT8* const pals[] = {&BGP_REG, &OBP0_REG, &OBP1_REG};

void FadeDMG(UINT8 fadeout) {
	static UINT8 colors[12];
	UINT8 i, j; 
	UINT8* c = colors;
	UINT8 p;

	//Pick current palette colors
	for(i = 0; i != 3; ++i) {
		p = (UINT8)*(pals[i]);
		for(j = 0; j != 8; j += 2, ++c) {
			*c = (DespRight(p, j)) & 0x3;
		}
	}

	for(i = 0; i != 4; ++i) {
		p = fadeout ? 3 - i : i;
		for(j = 0; j != 3; ++j) {
			c = &colors[j << 2];
			*pals[j] = DMG_PALETTE(FadeInOp(c[0], p), FadeInOp(c[1], p), FadeInOp(c[2], p), FadeInOp(c[3], p));
		}
		vsync();
		vsync();
		vsync();
	}
}

void FadeIn(void) BANKED {
	FadeDMG(0);
	DISPLAY_OFF;
}

void FadeOut(void) BANKED {
	DISPLAY_ON;
	FadeDMG(1);
}

