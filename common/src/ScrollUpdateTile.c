#include "Vector.h"
#include "Scroll.h"
#include "Sprite.h"
#include "SpriteManager.h"
#include "Math.h"
#include "main.h"

// To be defined on the main app
UINT8 GetTileReplacement(UINT8* tile_ptr, UINT8* tile);

// global variables, updated before each call to UPDATE_TILE()
extern unsigned char* scroll_ptr;
extern unsigned char* scroll_cptr;

void UPDATE_TILE(INT16 x, INT16 y) {
	static UINT8 replacement;

	if(((UINT16)x >= scroll_tiles_w) || ((UINT16)y >= scroll_tiles_h)) { //This also checks x < 0 || y < 0
		replacement = 0;
	} else {
		static UINT8 type;
		replacement = *scroll_ptr;
		type = GetTileReplacement(scroll_ptr, &replacement);
		if(type != 255u) {
			static UINT8 i;
			static UINT16 id;
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
				UINT16 sprite_y = TILE_TO_PX(y + 1) - spriteDatas[type]->height;
				SWITCH_ROM(__save);
				SpriteManagerAdd(type, TILE_TO_PX(x), sprite_y);
			}
		}
	}

	ScrollSetAttrTileXY(SCREEN_BKG_OFFSET_X + x + scroll_offset_x, y + scroll_offset_y, replacement, ((scroll_cptr) ? *scroll_cptr : scroll_tile_info[replacement]));
}
