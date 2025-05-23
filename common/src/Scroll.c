#include <string.h>

#include "Vector.h"
#include "Scroll.h"
#include "Sprite.h"
#include "SpriteManager.h"
#include "Math.h"
#include "main.h"
#include "Palette.h"

#define TOP_MOVEMENT_LIMIT    30
#if defined(MASTERSYSTEM)
#define SCREEN_BKG_OFFSET_X   1

#define SCREEN_PAD_LEFT       0
#define SCREEN_PAD_RIGHT      1
#define SCREEN_PAD_TOP        0
#define SCREEN_PAD_BOTTOM     2

#define SCREEN_RESTORE_W      16
#define SCREEN_RESTORE_H      10

#define BOTTOM_MOVEMENT_LIMIT 150
#else                         
#define SCREEN_BKG_OFFSET_X   0

#define SCREEN_PAD_LEFT       1
#define SCREEN_PAD_RIGHT      2
#define SCREEN_PAD_TOP        1
#define SCREEN_PAD_BOTTOM     2

#define SCREEN_RESTORE_W      5
#define SCREEN_RESTORE_H      5

#define BOTTOM_MOVEMENT_LIMIT 100
#endif

#define SCREEN_TILE_REFRES_W (SCREEN_TILES_W + SCREEN_PAD_LEFT + SCREEN_PAD_RIGHT)
#define SCREEN_TILE_REFRES_H (SCREEN_TILES_H + SCREEN_PAD_TOP  + SCREEN_PAD_BOTTOM)

UINT8 scroll_top_movement_limit = TOP_MOVEMENT_LIMIT;
UINT8 scroll_bottom_movement_limit = BOTTOM_MOVEMENT_LIMIT;

// To be defined on the main app
UINT8 GetTileReplacement(UINT8* tile_ptr, UINT8* tile);

unsigned char* scroll_map = 0;
unsigned char* scroll_cmap = 0;
INT16 scroll_x = 0;
INT16 scroll_y = 0;
INT16 scroll_x_vblank = 0;
INT16 scroll_y_vblank = 0;
UINT16 scroll_w;
UINT16 scroll_h;
UINT16 scroll_tiles_w;
UINT16 scroll_tiles_h;
Sprite* scroll_target = 0;
UINT8 scroll_collisions[256];
UINT8 scroll_tile_info[256];
UINT8 scroll_bank;
UINT8 scroll_offset_x = 0;
UINT8 scroll_offset_y = 0;

INT8 scroll_h_border = 0;
UINT8 clamp_enabled = 1;

INT16 pending_h_x, pending_h_y;
UINT8 pending_h_i;
unsigned char* pending_h_map = 0;
unsigned char* pending_w_map = 0;
#ifdef CGB
unsigned char* pending_h_cmap = 0;
unsigned char* pending_w_cmap = 0;
#endif
INT16 pending_w_x, pending_w_y;
UINT8 pending_w_i;

UINT8 last_tile_loaded = 0;

UINT16 hud_map_offset;

// Keeping track of the current tiles loaded on offset 0
UINT8 tiles_bank_0;
const struct TilesInfo* tiles_0;

void UPDATE_TILE(INT16 x, INT16 y, UINT8* t, UINT8* c) {
	static UINT8 replacement;
	static UINT8 type;
	c;

	if(((UINT16)x >= scroll_tiles_w) || ((UINT16)y >= scroll_tiles_h)) { //This also checks x < 0 || y < 0
		replacement = 0;
	} else {
		replacement = *t;
		type = GetTileReplacement(t, &replacement);
		if(type != 255u) {
			static UINT16 id;
			static UINT8 i;
			id = SPRITE_UNIQUE_ID(x, y);
			for (i = VECTOR_LEN(sprite_manager_updatables); (i); i--) {
				Sprite* s = sprite_manager_sprites[sprite_manager_updatables[i]];
				if ((s->type == type) && (s->unique_id == id)) {
					break;
				}
			}
			if (i == 0) {
				UINT8 __save = CURRENT_BANK;
				SWITCH_ROM(spriteDataBanks[type]);
				UINT16 sprite_y = ((y + 1) << 3) - spriteDatas[type]->height;
				SWITCH_ROM(__save);
				SpriteManagerAdd(type, x << 3, sprite_y);
			}
		}
	}

#if defined(NINTENDO)
	UINT8* addr = set_bkg_tile_xy(x + scroll_offset_x, y + scroll_offset_y, replacement);
	#ifdef CGB
		if (_cpu == CGB_TYPE) {
			VBK_REG = 1;
			set_vram_byte(addr, (scroll_cmap) ? *c : scroll_tile_info[replacement]);
			VBK_REG = 0;
		}
	#endif
#elif defined(SEGA)
	set_attributed_tile_xy(SCREEN_BKG_OFFSET_X + x + scroll_offset_x, y + scroll_offset_y, (UINT16)(((scroll_cmap) ? *c : scroll_tile_info[replacement]) << 8) | replacement);
#endif
}

UINT16 ScrollSetTiles(UINT8 first_tile, UINT8 tiles_bank, const struct TilesInfo* tiles) {
	UINT8 i;
	UINT8 n_tiles;
	UINT8* palette_entries;

	UINT16 offset = first_tile;

	if(first_tile == 0) {
		tiles_bank_0 = tiles_bank;
		tiles_0 = tiles;
	}

	UINT8 __save = CURRENT_BANK;
	SWITCH_ROM(tiles_bank);
	n_tiles = tiles->num_frames;
	palette_entries = tiles->color_data;

	#if DEFAULT_COLOR_DEPTH == 4
	set_bkg_native_data(first_tile, n_tiles, tiles->data);
	#else
	set_bkg_data(first_tile, n_tiles, tiles->data);
	#endif

	last_tile_loaded = first_tile + n_tiles;
	for(i = first_tile; i != last_tile_loaded; ++i) {
		scroll_tile_info[i] = palette_entries ? palette_entries[i - first_tile] : 0;
	}

#if defined(SEGA) || (defined(NINTENDO) && defined(CGB))
	#if defined(CGB)
	//Load palettes
	for(i = 0; i != last_bg_pal_loaded; ++ i) {
		if (memcmp(ZGB_Fading_BPal + (i * N_PALETTE_COLORS), tiles->pals, tiles->num_pals * PALETTE_SIZE) == 0)
			break;
	}

	offset |= (UINT16)(i << 8);

	if(i == last_bg_pal_loaded) {
	#endif
		last_bg_pal_loaded += SetPalette(BG_PALETTE, last_bg_pal_loaded, tiles->num_pals, tiles->pals, tiles_bank);
	#if defined(CGB)
	}
	#endif
#endif
	SWITCH_ROM(__save);

	return offset;
}

void UpdateMapTile(UINT8 bg_or_win, UINT8 x, UINT8 y, UINT16 map_offset, UINT8 data, UINT8* attr)
{
attr;
#if defined(NINTENDO)
	UINT8* addr = (bg_or_win == TARGET_BKG) ? set_bkg_tile_xy(x, y, (UINT8)map_offset + data) : set_win_tile_xy(x, y, (UINT8)map_offset + data);
	#if defined(CGB)
	if (_cpu == CGB_TYPE) {
		VBK_REG = 1;
		set_vram_byte(addr, (attr) ? *attr : (UINT8)(map_offset >> 8) + scroll_tile_info[(UINT8)map_offset + data]);
		VBK_REG = 0;
	}
	#endif
#elif defined(SEGA)
	if (bg_or_win != TARGET_BKG) return;

	UINT8 c = ((UINT8)(map_offset >> 8)) + ((attr) ? *attr : scroll_tile_info[data]);
	set_attributed_tile_xy(SCREEN_BKG_OFFSET_X + x, y, (UINT16)(c << 8) | ((UINT8)map_offset + data));
#endif
}

UINT16 LoadMap(UINT8 bg_or_win, UINT8 x, UINT8 y, UINT8 map_bank, struct MapInfo* map) {
#if defined(SEGA)
	if (bg_or_win != TARGET_BKG) return 0;
#endif

	UINT8 __save = CURRENT_BANK;
	SWITCH_ROM(map_bank);

	//Load Tiles
	UINT16 map_offset;
	UINT8 load_tiles = (tiles_bank_0 != map->tiles_bank) || (tiles_0 != map->tiles); // If the tile set is the same as the one used for the scroll or the bg (which is stored in tiles_bank_0 and tiles0) then do not load the tiles again
	if (load_tiles) {
		if (map->extra_tiles) {
			map_offset = ScrollSetTiles(last_tile_loaded, map->extra_tiles_bank, map->extra_tiles);
			if (map->tiles) ScrollSetTiles(last_tile_loaded, map->tiles_bank, map->tiles);
		} else {
			map_offset = ScrollSetTiles(last_tile_loaded, map->tiles_bank, map->tiles);
		}
	} else {
		map_offset = 0;
	}

	// Load map (tile by tile because it there are not attributes when need to pick them from scroll_tile_info)
	UINT8* data = map->data;
	UINT8* attrs = map->attributes;
	for (UINT8 j = 0; j < map->height; ++j) {
		for (UINT8 i = 0; i < map->width; ++i) {
			UpdateMapTile(bg_or_win, x + i, y + j, map_offset, *data, attrs);

			++data;
			if (attrs)
				++attrs;
		}
	}

	SWITCH_ROM(__save);

	// Return the offset so the user can pass it as parameter to UpdateMapTile
	return map_offset;
}

void ClampScrollLimits(void);

void ScrollSetMap(UINT8 map_bank, const struct MapInfo* map) {
	UINT8 __save = CURRENT_BANK;
	SWITCH_ROM(map_bank);
	scroll_tiles_w = map->width;
	scroll_tiles_h = map->height;
	scroll_map = map->data;
	scroll_cmap = map->attributes;
	scroll_x = 0;
	scroll_y = 0;
	scroll_w = scroll_tiles_w << 3;
	scroll_h = scroll_tiles_h << 3;
	scroll_bank = map_bank;
	if (scroll_target) {
		scroll_x = scroll_target->x - (SCREEN_WIDTH >> 1);
		scroll_y = scroll_target->y - scroll_bottom_movement_limit; //Move the camera to its bottom limit
		ClampScrollLimits();
	}
	pending_h_i = 0;
	pending_w_i = 0;
	SWITCH_ROM(__save);
}

void ScrollInitTilesFromMap(UINT8 first_tile, UINT8 map_bank, const struct MapInfo* map) {
	last_tile_loaded = first_tile;
	UINT8 __save = CURRENT_BANK;
	SWITCH_ROM(map_bank);
		if (map->tiles) ScrollSetTiles(0, map->tiles_bank, map->tiles);
		if (map->extra_tiles) ScrollSetTiles(last_tile_loaded, map->extra_tiles_bank, map->extra_tiles);
	SWITCH_ROM(__save);
}


void ScrollCollisionsReset(void) {
	memset(scroll_collisions, 0, sizeof(scroll_collisions));
}

void ScrollInitCollisionGroup(UINT8 group, const UINT8* coll_list) {
	for(const UINT8 * ptr = coll_list; (*ptr); ptr++) 
		scroll_collisions[*ptr] |= group;
}


void ScrollInitCollisions(const UINT8* coll_list, const UINT8* coll_list_down) {
	ScrollCollisionsReset();
	if (coll_list) {
		ScrollInitCollisionGroup((COLL_GROUP_DEFAULT | COLL_GROUP_DOWN), coll_list);
	}
	if (coll_list_down) {
		ScrollInitCollisionGroup(COLL_GROUP_DOWN, coll_list_down);
	}
}

void ScrollScreenRedraw(void) {
	UINT8 __save = CURRENT_BANK;
	SWITCH_ROM(scroll_bank);
		INT16 y = scroll_y >> 3;
		for (UINT8 i = 0u; i != (SCREEN_TILE_REFRES_H) && y != scroll_h; ++i, y++) {
			ScrollUpdateRow((scroll_x >> 3) - SCREEN_PAD_LEFT,  y - SCREEN_PAD_TOP);
		}
	SWITCH_ROM(__save);
}

void InitScroll(UINT8 map_bank, const struct MapInfo* map, const UINT8* coll_list, const UINT8* coll_list_down) {
	// Init Tiles
	ScrollInitTilesFromMap(0, map_bank, map);

	// Init Map
	ScrollSetMap(map_bank, map);

	// Init Collisions
	ScrollInitCollisions(coll_list, coll_list_down);

	// Redraw screen
	ScrollScreenRedraw();
}

void ScrollUpdateRowR(void) {
	for (UINT8 i = 0u; i != SCREEN_RESTORE_W && pending_w_i != 0; ++i, --pending_w_i)  {
		#ifdef CGB
		UPDATE_TILE(pending_w_x++, pending_w_y, pending_w_map++, pending_w_cmap++);
		#else
		UPDATE_TILE(pending_w_x++, pending_w_y, pending_w_map++, 0);
		#endif
	}
}

void ScrollUpdateRowWithDelay(INT16 x, INT16 y) {
	while (pending_w_i) {
		ScrollUpdateRowR();
	}

	pending_w_i = SCREEN_TILE_REFRES_W;

	UINT16 delta = scroll_tiles_w * (pending_w_y = y) + (pending_w_x = x);
	pending_w_map = scroll_map + delta;
	#ifdef CGB
	pending_w_cmap = scroll_cmap + delta;
	#endif
}

void ScrollUpdateRow(INT16 x, INT16 y) {
	UINT16 offset = scroll_tiles_w * y + x;
	unsigned char* map = scroll_map + offset;
	#ifdef CGB
	unsigned char* cmap = scroll_cmap + offset;
	#endif

	UINT8 __save = CURRENT_BANK;
	SWITCH_ROM(scroll_bank);
	for (UINT8 i = 0u; i != (SCREEN_TILE_REFRES_W); ++i) {
		#ifdef CGB
		UPDATE_TILE(x + i, y, map ++, cmap ++);
		#else
		UPDATE_TILE(x + i, y, map ++, 0);
		#endif
	}
	SWITCH_ROM(__save);
}

void ScrollUpdateColumnR(void) {
	for (UINT8 i = 0u; i != SCREEN_RESTORE_H && pending_h_i != 0; ++i, pending_h_i --) {
		#ifdef CGB
		UPDATE_TILE(pending_h_x, pending_h_y ++, pending_h_map, pending_h_cmap);
		pending_h_map += scroll_tiles_w;
		pending_h_cmap += scroll_tiles_w;
		#else
		UPDATE_TILE(pending_h_x, pending_h_y ++, pending_h_map, 0);
		pending_h_map += scroll_tiles_w;
		#endif
	}
}

void ScrollUpdateColumnWithDelay(INT16 x, INT16 y) {
	while (pending_h_i) {
		ScrollUpdateColumnR();
	}

	pending_h_i = SCREEN_TILE_REFRES_H;

	UINT16 delta = scroll_tiles_w * (pending_h_y = y) + (pending_h_x = x);
	pending_h_map = scroll_map + delta;
	#ifdef CGB
	pending_h_cmap = scroll_cmap + delta;
	#endif
}

void ScrollUpdateColumn(INT16 x, INT16 y) {
	UINT16 offset = scroll_tiles_w * y + x;
	unsigned char* map = scroll_map + offset;
	#ifdef CGB
	unsigned char* cmap = scroll_cmap + offset;
	#endif

	UINT8 __save = CURRENT_BANK;
	SWITCH_ROM(scroll_bank);
	for (UINT8 i = 0u; i != (SCREEN_TILE_REFRES_H); ++i) {
		#ifdef CGB
		UPDATE_TILE(x, y + i, map, cmap);
		map += scroll_tiles_w;
		cmap += scroll_tiles_w;
		#else
		UPDATE_TILE(x, y + i, map, 0);
		map += scroll_tiles_w;
		#endif
	}
	SWITCH_ROM(__save);
}

void RefreshScroll(void) {
	if (scroll_target) {
		UINT16 ny = scroll_y;

		if (scroll_bottom_movement_limit < (scroll_target->y - scroll_y)) ny = scroll_target->y - scroll_bottom_movement_limit;
		else if (scroll_top_movement_limit > (scroll_target->y - scroll_y)) ny = scroll_target->y - scroll_top_movement_limit;

		MoveScroll(scroll_target->x - (SCREEN_WIDTH >> 1), ny);
	}
}

void MoveScroll(INT16 x, INT16 y) {
	INT16 current_column, new_column, current_row, new_row;

	UINT8 __save = CURRENT_BANK;
	SWITCH_ROM(scroll_bank);

	current_column = scroll_x >> 3;
	current_row    = scroll_y >> 3;

	scroll_x = x;
	scroll_y = y;
	ClampScrollLimits();

	new_column     = scroll_x >> 3;
	new_row        = scroll_y >> 3;

	if (current_column != new_column) {
		if (new_column > current_column) {
			ScrollUpdateColumnWithDelay(new_column - SCREEN_PAD_LEFT + SCREEN_TILE_REFRES_W - 1, new_row - SCREEN_PAD_TOP);
		} else {
			ScrollUpdateColumnWithDelay(new_column - SCREEN_PAD_LEFT,                            new_row - SCREEN_PAD_TOP);
		}
	}

	if (current_row != new_row) {
		if (new_row > current_row) {
			ScrollUpdateRowWithDelay(new_column - SCREEN_PAD_LEFT, new_row - SCREEN_PAD_TOP + SCREEN_TILE_REFRES_H - 1);
		} else if (new_row >= SCREEN_PAD_TOP) {
			ScrollUpdateRowWithDelay(new_column - SCREEN_PAD_LEFT, new_row - SCREEN_PAD_TOP);
		}
	}

	if (pending_w_i) {
		ScrollUpdateRowR();
	}
	if (pending_h_i) {
		ScrollUpdateColumnR();
	}
	SWITCH_ROM(__save);
}

UINT8 GetScrollTile(UINT16 x, UINT16 y) {
	UINT8 ret;
	UINT8 __save = CURRENT_BANK;
	SWITCH_ROM(scroll_bank);
	ret = *GetScrollTilePtr(x, y);
	SWITCH_ROM(__save);
	return ret;
}

void GetMapSize(UINT8 map_bank, const struct MapInfo* map, UINT8* tiles_w, UINT8* tiles_h) {
	UINT8 __save = CURRENT_BANK;
	SWITCH_ROM(map_bank);
	if (tiles_w) *tiles_w = map->width;
	if (tiles_h) *tiles_h = map->height;
	SWITCH_ROM(__save);
}

