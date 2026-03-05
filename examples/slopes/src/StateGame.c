#include "Banks/SetAutoBank.h"

#include "ZGBMain.h"
#include "Scroll.h"
#include "SpriteManager.h"
#include "Vector.h"

#include "Collision.h"

IMPORT_MAP(map);

const UINT8 collision_all[]                    = LIST(1);
const UINT8 collision_top[]                    = LIST(8);
const UINT8 collision_bottom[]                 = LIST_EMPTY; // unused in this scene
const UINT8 collision_left[]                   = LIST_EMPTY; // unused in this scene
const UINT8 collision_right[]                  = LIST_EMPTY; // unused in this scene
const UINT8 collision_slope_45_right[]         = LIST(2);
const UINT8 collision_slope_225_right_bottom[] = LIST(4);
const UINT8 collision_slope_225_right_top[]    = LIST(5);
const UINT8 collision_slope_45_left[]          = LIST(3);
const UINT8 collision_slope_225_left_bottom[]  = LIST(6);
const UINT8 collision_slope_225_left_top[]     = LIST(7);

void LocateObjects(UINT8 map_bank, struct MapInfo* map, UINT8* start_x, UINT8* start_y) NONBANKED {
	UINT8 * data, __save_bank = CURRENT_BANK;
	SWITCH_ROM(map_bank);
	data = map->data;
	for(UINT8 y = 0; y < map->height; ++ y) {
		for(UINT8 x = 0; x < map->width; ++ x) {
			UINT8 tile = *data++;
			if (tile == 255) {  //player
				*start_x = x;
				*start_y = y;
			}
		}
	}
	SWITCH_ROM(__save_bank);
}

void START(void) {
	static UINT8 start_x, start_y;

	LocateObjects(BANK(map), &map, &start_x, &start_y);
	InitScroll(BANK(map), &map, NULL, 0);

	ScrollInitCollisionGroup(COLLISION_ALL,    collision_all);
	ScrollInitCollisionGroup(COLLISION_TOP,    collision_top);
	ScrollInitCollisionGroup(COLLISION_BOTTOM, collision_bottom);
	ScrollInitCollisionGroup(COLLISION_LEFT,   collision_left);
	ScrollInitCollisionGroup(COLLISION_RIGHT,  collision_right);
	ScrollInitCollisionGroup(COLLISION_SLOPE_45_RIGHT,      collision_slope_45_right);
	ScrollInitCollisionGroup(COLLISION_SLOPE_225_RIGHT_BOT, collision_slope_225_right_bottom);
	ScrollInitCollisionGroup(COLLISION_SLOPE_225_RIGHT_TOP, collision_slope_225_right_top);
	ScrollInitCollisionGroup(COLLISION_SLOPE_45_LEFT,       collision_slope_45_left);
	ScrollInitCollisionGroup(COLLISION_SLOPE_225_LEFT_BOT,  collision_slope_225_left_bottom);
	ScrollInitCollisionGroup(COLLISION_SLOPE_225_LEFT_TOP,  collision_slope_225_left_top);
	
	scroll_target = SpriteManagerAdd(SpritePlayer, start_x << 3, (start_y - 1) << 3);
}

void UPDATE(void) {
}
