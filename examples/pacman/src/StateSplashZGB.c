#include "Banks/SetAutoBank.h"

#include "Coroutines.h"

#include "Scroll.h"
#include "Keys.h"
#include "Palette.h"
#include "ZGBMain.h"

IMPORT_MAP(splash_zgb);

#define SPLASH_SCREEN_TIMEOUT 3

NORETURN void SplashLogic(void * custom_data) BANKED {
	(void)custom_data;

	static UINT16 splash_timer;

	splash_timer = SPLASH_SCREEN_TIMEOUT * 60;
	SetBorderColor(RGB(0, 0, 0));
	InitScroll(BANK(splash_zgb), &splash_zgb, 0, 0);
	YIELD;

	for (;; YIELD) {
		if (--splash_timer == 0) 
			SetState(StateMenu);
		if (KEY_TICKED(J_START | J_A | J_B)) {
			SetState(StateMenu);
		}
	}
}

STATE_COROUTINE(SplashLogic, NONE)
