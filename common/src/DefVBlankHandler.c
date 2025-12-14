#include <gbdk/platform.h>

#include "Math.h"
#include "Scroll.h"

extern UINT8 vbl_count;
void VBL_isr(void) NONBANKED {
	vbl_count ++;
#if defined(NINTENDO)
	move_bkg(scroll_x_vblank + TILE_TO_PX(scroll_offset_x), scroll_y_vblank + TILE_TO_PX(scroll_offset_y));
#elif defined(SEGA)
	if (_shadow_OAM_OFF == 0) {
		__WRITE_VDP_REG_UNSAFE(VDP_RSCX, -(scroll_x_vblank + TILE_TO_PX(scroll_offset_x)));
		__WRITE_VDP_REG_UNSAFE(VDP_RSCY, (((UINT16)(scroll_y_vblank + TILE_TO_PX(scroll_offset_y))) % TILE_TO_PX(DEVICE_SCREEN_BUFFER_HEIGHT)));
	}
#endif
}
