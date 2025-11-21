#include "Vector.h"
#include "Scroll.h"
#include "Sprite.h"
#include "SpriteManager.h"
#include "main.h"

// To be defined on the main app
UINT8 GetTileReplacement(UINT8* tile_ptr, UINT8* tile);

extern unsigned char* scroll_map;
extern unsigned char* scroll_cmap;

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
