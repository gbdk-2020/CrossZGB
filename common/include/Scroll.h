#ifndef SCROLL_H
#define SCROLL_H

#include <gbdk/platform.h>

#include "TilesInfo.h"
#include "MapInfo.h"
#include "Sprite.h"
#include "Math.h"

#define SCREEN_TILES_H       DEVICE_SCREEN_HEIGHT
#if defined(MASTERSYSTEM)
#define SCREEN_TILES_W       (DEVICE_SCREEN_WIDTH - 1)
#else
#define SCREEN_TILES_W       DEVICE_SCREEN_WIDTH
#endif
#define SCREEN_WIDTH         TILE_TO_PX(SCREEN_TILES_W)
#define SCREEN_HEIGHT        TILE_TO_PX(SCREEN_TILES_H)

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

typedef enum {
	TARGET_BKG = 0,
	TARGET_WIN = 1
} LOAD_TARGET;

#define SPRITE_UNIQUE_ID(TILE_X, TILE_Y) ((0x00FF & TILE_X) | ((0xFF00 & (TILE_Y << 8))))

#if defined(NINTENDO)
void SetWindowPos(UINT8 x, UINT8 y, UINT8 h);
#define INIT_HUD_EX(MAP, Y, H)\
	hud_map_offset = LoadMap(TARGET_WIN, 0, 0, BANK(MAP), &MAP); \
	SetWindowPos(DEVICE_WINDOW_PX_OFFSET_X, (UINT8)(Y), (H))
#define INIT_HUD(MAP)\
	GetMapSize(BANK(MAP), &MAP, 0, &scroll_h_border); \
	scroll_h_border = TILE_TO_PX(scroll_h_border); \
	INIT_HUD_EX(MAP, ((DEVICE_WINDOW_PX_OFFSET_Y + DEVICE_SCREEN_PX_HEIGHT) - scroll_h_border), DEVICE_SCREEN_PX_HEIGHT)
#define HIDE_HUD SetWindowPos(DEVICE_WINDOW_PX_OFFSET_X, 0, 0)
#else
#define INIT_HUD_EX(MAP, Y, H)
#define INIT_HUD(MAP)
#define HIDE_HUD
void set_vram_word(uint8_t * addr, uint16_t v) PRESERVES_REGS(iyh, iyl);
#endif

#define INIT_BKG(MAP) LoadMap(TARGET_BKG, 0, 0, BANK(MAP), &MAP)

#define UPDATE_HUD_TILE(X, Y, TILE) UpdateMapTile(TARGET_WIN, X, Y, hud_map_offset, TILE, 0)

//This one updates the background with a tile from the hud
#define UPDATE_BKG_TILE(X, Y, TILE) UpdateMapTile(TARGET_BKG, X, Y, hud_map_offset, TILE, 0)

extern unsigned char* scroll_map;
extern INT16 scroll_x;
extern INT16 scroll_y;
extern INT16 scroll_x_vblank;
extern INT16 scroll_y_vblank;
extern UINT16 scroll_w;
extern UINT16 scroll_h;
extern UINT16 scroll_tiles_w;
extern UINT16 scroll_tiles_h;
extern UINT8 scroll_collisions[256];
extern UINT8 scroll_tile_info[256];
extern UINT8 scroll_bank;
extern UINT8 scroll_offset_x;
extern UINT8 scroll_offset_y;

extern Sprite* scroll_target;
extern INT16 scroll_h_border;
extern UINT8 clamp_enabled;

extern UINT8 scroll_top_movement_limit;
extern UINT8 scroll_bottom_movement_limit;

extern UINT8 last_tile_loaded;

extern UINT16 hud_map_offset;

UINT16 ScrollSetTiles(UINT8 first_tile, UINT8 tile_data_bank, const struct TilesInfo* tile_data);
#define LOAD_TILES(TILES) ScrollSetTiles(last_tile_loaded, BANK((TILES)), &(TILES))

void ScrollSetMap(UINT8 map_bank, const struct MapInfo* map);

void ScrollInitTilesFromMap(UINT8 first_tile, UINT8 map_bank, const struct MapInfo* map);
#define InitScrollTiles(FIRST_TILE, TILE_MAP) ScrollInitTilesFromMap((FIRST_TILE), BANK((TILE_MAP)), &(TILE_MAP))

typedef enum {
	COLL_GROUP_0 = 0x01,
	COLL_GROUP_DEFAULT = 0x01,
	COLL_GROUP_1 = 0x02,
	COLL_GROUP_2 = 0x04,
	COLL_GROUP_3 = 0x08,
	COLL_GROUP_4 = 0x10,
	COLL_GROUP_5 = 0x20,
	COLL_GROUP_6 = 0x40,
	COLL_GROUP_7 = 0x80,
	COLL_GROUP_DOWN = 0x80 
} CollisionGroup;

void ScrollInitCollisions(const UINT8* coll_list, const UINT8* coll_list_down);
void ScrollCollisionsReset(void);
void ScrollInitCollisionGroup(UINT8 group, const UINT8* coll_list);
void ScrollInitCollisionGroupVector(UINT8 group, const UINT8* coll_vector);
void ScrollRemoveCollisionGroupVector(UINT8 group, const UINT8* coll_vector);
void ScrollScreenRedraw(void);

void InitScroll(UINT8 map_bank, const struct MapInfo* map, const UINT8* coll_list, const UINT8* coll_list_down);

UINT16 LoadMap(UINT8 bg_or_win, UINT8 x, UINT8 y, UINT8 map_bank, struct MapInfo* map);
void UpdateMapTile(UINT8 bg_or_win, UINT8 x, UINT8 y, UINT16 map_offset, UINT8 data, UINT8* attr);

void ScrollUpdateRow(INT16 x, INT16 y);
void ScrollUpdateColumn(INT16 x, INT16 y);
void MoveScroll(INT16 x, INT16 y);
void RefreshScroll(void);
void FinishPendingScrollUpdates(void);

void GetMapSize(UINT8 map_bank, const struct MapInfo* map, UINT16* tiles_w, UINT16* tiles_h);

inline UINT8* GetScrollTilePtr(UINT16 x, UINT16 y) {
	//Ensure you have selected scroll_bank before calling this function
	//And it is returning a pointer so don't swap banks after you get the value
	return scroll_map + (scroll_tiles_w * y + x);
}

UINT8 GetScrollTile(UINT16 x, UINT16 y);
UINT8 ScrollFindTile(UINT8 map_bank, const struct MapInfo* map, UINT8 tile,
	UINT16 start_x, UINT16 start_y, UINT16 w, UINT16 h,
	UINT16* x, UINT16* y);

inline void ScrollSetAttrTileAddr(UINT8 *addr, UINT8 t, UINT8 c) {
	(void)c;
#if defined(NINTENDO)
	set_vram_byte(addr, t);
	#if defined(CGB)
	if (_cpu == CGB_TYPE) {
		VBK_REG = 1;
		set_vram_byte(addr, c);
		VBK_REG = 0;
	}
	#endif
#elif defined(SEGA)
	set_vram_word(addr, (UINT16)(c << 8) | t);
#endif
}

inline void ScrollSetAttrTileXY(UINT8 x, UINT8 y, UINT8 t, UINT8 c) {
	(void)c;
#if defined(NINTENDO)
	UINT8* addr = set_bkg_tile_xy(x, y, t);
	#if defined(CGB)
	if (_cpu == CGB_TYPE) {
		VBK_REG = 1;
		set_vram_byte(addr, c);
		VBK_REG = 0;
	}
	#endif
#elif defined(SEGA)
	set_attributed_tile_xy(x, y, (UINT16)(c << 8) | t);
#endif
}

#endif