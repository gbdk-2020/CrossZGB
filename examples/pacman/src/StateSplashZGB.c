#include "Banks/SetAutoBank.h"

#include "Scroll.h"
#include "Keys.h"
#include "Palette.h"
#include "ZGBMain.h"

IMPORT_MAP(splash_zgb);

#define SPLASH_SCREEN_TIMEOUT 3

UINT16 splash_timer;

void START(void) {
	splash_timer = SPLASH_SCREEN_TIMEOUT * 60;
	SetBorderColor(RGB(0, 0, 0));
	InitScroll(BANK(splash_zgb), &splash_zgb, 0, 0);
}

void UPDATE(void) {
	if (--splash_timer == 0) 
		SetState(StateMenu);
	if (KEY_TICKED(J_START | J_A | J_B)) {
		SetState(StateMenu);
	}
}

void DESTROY(void) {
}
