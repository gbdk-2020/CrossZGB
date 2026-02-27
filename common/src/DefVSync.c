#include <gbdk/platform.h>

void SyncVBlank(void) NONBANKED {
	vsync();
}
