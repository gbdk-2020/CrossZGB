#include <gbdk/platform.h>

// default empty tile replacement function, if not overridden by the ZGBMain.c
UINT8 GetTileReplacement(UINT8* tile_ptr, UINT8* tile) {
	tile_ptr; tile; // suppress warning
	return 255u;    // don't replace anything
}