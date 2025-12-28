#include <gbdk/platform.h>

#include <stdint.h>
#include <rand.h>

#include "Shake.h"

uint8_t shake_type = SHAKE_NONE, shake_duration = 0, shake_intensivity;

const int8_t shake_offsets[SHAKE_RANGE << 1] = { 1, -1, 2, -2, 3, -3, 4, -4 };

void ShakeVBL_ISR(void) {
	if (shake_duration) {
		if (--shake_duration) {
			int8_t dx = (shake_type & SHAKE_X) ? shake_offsets[rand() % shake_intensivity] : 0;
			int8_t dy = (shake_type & SHAKE_Y) ? shake_offsets[rand() % shake_intensivity] : 0;
			scroll_bkg(dx, dy);
		}
	}
}