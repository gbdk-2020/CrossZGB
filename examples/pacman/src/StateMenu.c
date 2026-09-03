#include "Banks/SetAutoBank.h"

#include "Coroutines.h"

#include "Scroll.h"
#include "Keys.h"
#include "Palette.h"
#include "Print.h"
#include "ZGBMain.h"

IMPORT_MAP(menu);
IMPORT_FONT(font);

#define CENTER(len) (((SCREEN_TILES_W - (len)) >> 1) + 1)

NORETURN void MenuLogic(void * custom_data) BANKED {
	(void)custom_data;

	InitScroll(BANK(menu), &menu, 0, 0);
	MoveScroll(4, 0);
	INIT_FONT_EX(font, PRINT_BKG);
	PRINT(CENTER(17),  4, ".: P-A-C-M-A-N :.");
	PRINT(CENTER(13),  9, "PRESS ANY KEY");
	PRINT(CENTER(13), 10, "TO START GAME");
	YIELD;

	for (;; YIELD) {
		if (KEY_TICKED(J_START | J_A | J_B)) {
			SetState(StateGame);
		}
	}
}

STATE_COROUTINE(MenuLogic, NONE)
