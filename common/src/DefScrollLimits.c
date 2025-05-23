#include "Scroll.h"

// default clamping scroll limits function, if not overridden by the ZGBMain.c
void ClampScrollLimits(void) {
	if (clamp_enabled) {
		if (scroll_w < SCREEN_WIDTH) scroll_x = 0u;
		else if (scroll_x < 0) scroll_x = 0u;

		if (scroll_x > (scroll_w - SCREEN_WIDTH)) scroll_x = (scroll_w - SCREEN_WIDTH);

		if (scroll_h < (SCREEN_HEIGHT - scroll_h_border)) scroll_y = 0u;
		else if (scroll_y < 0) scroll_y = 0u;

		if (scroll_y > (scroll_h - SCREEN_HEIGHT + scroll_h_border)) scroll_y = (scroll_h - SCREEN_HEIGHT + scroll_h_border);
	}
}
