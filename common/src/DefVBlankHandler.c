#include <gbdk/platform.h>

#include "Scroll.h"

UINT8 vbl_count = 0;
void VBL_isr(void) NONBANKED {
	vbl_count ++;
#if defined(NINTENDO)
	move_bkg(scroll_x_vblank + (scroll_offset_x << 3), scroll_y_vblank + (scroll_offset_y << 3));
#elif defined(SEGA)
	if (_shadow_OAM_OFF == 0) {
		__WRITE_VDP_REG_UNSAFE(VDP_RSCX, -(scroll_x_vblank + (scroll_offset_x << 3)));
		__WRITE_VDP_REG_UNSAFE(VDP_RSCY, (((UINT16)(scroll_y_vblank + (scroll_offset_y << 3))) % (DEVICE_SCREEN_BUFFER_HEIGHT << 3)));
	}
#endif
}
