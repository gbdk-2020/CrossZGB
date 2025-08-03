#include <gbdk/platform.h>
#include <rand.h>

#include "Scroll.h"

#include "Shake.h"

INT8 shake_x_offset = 0, shake_y_offset = 0;
UINT8 shake_duration = 0, shake_type = SHAKE_X | SHAKE_Y, shake_intensivity; 

// module MUST export and update the vbl_count variable
UINT8 vbl_count = 0;

// the name of the ISR MUST be VBL_isr, so the standard handler from the CrossZGB library will be substituted
void VBL_isr(void) NONBANKED {
	vbl_count ++;
#if defined(NINTENDO)
	move_bkg(scroll_x_vblank + (scroll_offset_x << 3) + shake_x_offset, scroll_y_vblank + (scroll_offset_y << 3) + shake_y_offset);
#elif defined(SEGA)
	if (_shadow_OAM_OFF == 0) {
		__WRITE_VDP_REG_UNSAFE(VDP_RSCX, -(scroll_x_vblank + (scroll_offset_x << 3) + shake_x_offset));
		INT16 ypos = scroll_y_vblank + (scroll_offset_y << 3) + shake_y_offset;
		if (ypos < 0) ypos = 0;
		__WRITE_VDP_REG_UNSAFE(VDP_RSCY, ypos % (DEVICE_SCREEN_BUFFER_HEIGHT << 3));
	}
#endif
	if (shake_duration) {
		if (--shake_duration) {
			if (shake_type & SHAKE_X) shake_x_offset = (rand() % (shake_intensivity << 1)) - shake_intensivity;
			if (shake_type & SHAKE_Y) shake_y_offset = (rand() % (shake_intensivity << 1)) - shake_intensivity;
		} else {
			shake_x_offset = shake_y_offset = 0;
		}
	}
}
