#include "Banks/SetAutoBank.h"

#include "ZGBMain.h"
#include "Scroll.h"
#include "Keys.h"

IMPORT_MAP(logo);

void install_effect(void);
void uninstall_effect(void);

void START(void) {
	InitScroll(BANK(logo), &logo, 0, 0);
}

void UPDATE(void) {
	static UINT8 effect_enable = FALSE;
	if (KEY_TICKED(J_A)) {
		if (!effect_enable) {
			install_effect();
			effect_enable = TRUE;
		} else {
			uninstall_effect();
			effect_enable = FALSE;
		}
	}
}

void DESTROY(void) {
}
