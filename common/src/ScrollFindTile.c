#include "Scroll.h"

UINT8 ScrollFindTile(UINT8 map_bank, const struct MapInfo* map, UINT8 tile, UINT8 start_x, UINT8 start_y, UINT8 w, UINT8 h, UINT16* x, UINT16* y) {
	UINT8 __save = CURRENT_BANK;
	SWITCH_ROM(map_bank);
	for (UINT16 xt = start_x; xt != start_x + w; ++ xt) {
		for (UINT16 yt = start_y; yt != start_y + h; ++ yt) {
			if (map->data[map->width * yt + xt] == tile) {
				*x = xt;
				*y = yt;
				SWITCH_ROM(__save);
				return 1;
			}
		}
	}
	SWITCH_ROM(__save);

	*x = *y = 0;
	return 0;
}
