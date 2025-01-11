#include "Banks/SetAutoBank.h"

#include "Keys.h"
#include "ZGBMain.h"

IMPORT_HICOLOR(test_pattern_tall);

UINT8 scroll_limit;

void START(void) {
	// uninstall the standard LCD routine (used for the overlay window)
	LCD_uninstall();
	// disable fading between the state transitions
	fade_enabled = FALSE;
	// initialize the internal scroll position variables
	scroll_y = scroll_x = 0;
	// start display
	scroll_limit = HICOLOR_START(test_pattern_tall);
}

void UPDATE(void) {
	// scroll image up and down with keys within the scroll limits
	if (KEY_PRESSED(J_UP | J_A)) {
		if (scroll_y) scroll_y--;
	} else if (KEY_PRESSED(J_DOWN | J_B)) {
		if (scroll_y < scroll_limit) scroll_y++;
	}
}

void DESTROY(void) {
	// stop display
	HICOLOR_STOP;
	// reinstall the standard LCD routine
	LCD_install();
	// endble fading back
	fade_enabled = TRUE;
}
