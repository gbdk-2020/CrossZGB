#include "Banks/SetAutoBank.h"

#include "ZGBMain.h"
#include "Scroll.h"
#include "SpriteManager.h"

IMPORT_MAP(map);

UINT8 collision_tiles[] = {1, 0};

void START(void) {
	static UINT16 map_w, map_h, start_x, start_y;

	GetMapSize(BANK(map), &map, &map_w, &map_h);
	ScrollFindTile(BANK(map), &map, 255, 0, 0, map_w, map_h, &start_x, &start_y); // player is 255

	InitScroll(BANK(map), &map, collision_tiles, 0);
	scroll_target = SpriteManagerAdd(SpritePlayer, start_x << 3, (start_y - 1) << 3);
}

void UPDATE(void) {
}
