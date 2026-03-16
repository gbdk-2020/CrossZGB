#include <gbdk/platform.h>

extern UINT8 vbl_count;

UINT8 SyncVBlank(void) NONBANKED {
	if (!vbl_count) vsync();   // wait VBlank if not slowdown

	UINT8 delta_time = (vbl_count < 2u) ? 0u : 1u;
	vbl_count = 0;

	return delta_time;
}
