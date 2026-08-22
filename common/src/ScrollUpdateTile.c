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
extern unsigned char* dest_ptr;
extern unsigned char* dest_cptr;

void UPDATE_TILE(INT16 x, INT16 y) {
	if(((UINT16)x >= scroll_tiles_w) || ((UINT16)y >= scroll_tiles_h)) { //This also checks x < 0 || y < 0
		*dest_ptr = 0;
	} else {
		static UINT8 type;
		*dest_ptr = *scroll_ptr;
		type = GetTileReplacement(scroll_ptr, dest_ptr);
		if(type != 255u) {
			static UINT8 i, __save;
			for (i = VECTOR_LEN(sprite_manager_updatables); (i); i--) {
				Sprite* s = sprite_manager_sprites[sprite_manager_updatables[i]];
				if ((s->type == type) && (s->unique_id == (UINT16)scroll_ptr)) {
					break;
				}
			}
			if (i == 0) {
				__save = CURRENT_BANK;
				SWITCH_ROM(spriteDataBanks[type]);
				UINT16 sprite_y = TILE_TO_PX(y + 1) - spriteDatas[type]->height;
				SWITCH_ROM(__save);
				SpriteManagerAddEx(type, TILE_TO_PX(x), sprite_y, scroll_ptr);
			}
		}
	}
#if defined(SEGA) || (defined(NINTENDO) && defined(CGB))
	*dest_cptr = (scroll_cptr) ? *scroll_cptr : scroll_tile_info[*dest_ptr];
#endif
}
