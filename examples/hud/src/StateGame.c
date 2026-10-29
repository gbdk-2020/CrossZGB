#include "Banks/SetAutoBank.h"

#include "ZGBMain.h"
#include "Scroll.h"

IMPORT_MAP(logo);
IMPORT_MAP(keyboard);

void START(void) {
	InitScroll(BANK(logo), &logo, 0, 0);
	INIT_HUD_EX(keyboard, SCREEN_HEIGHT - 48, 48);
}

void UPDATE(void) {
}

void DESTROY(void) {
}