#include <string.h>

#include "main.h"

#include "Vector.h"
#include "SpriteManager.h"

void SpriteManagerFlushRemove(void) {
	static UINT8 __save, current, sprite_idx;
	__save = CURRENT_BANK;
	// iterate updatable sprites and render or remove marked
	current = 0;
	SPRITEMANAGER_ITERATE(THIS_IDX, THIS) {
		// get global sprite index
		sprite_idx = VECTOR_GET(sprite_manager_updatables, THIS_IDX);
		// remove sprite or render to screen
		if (THIS->marked_for_removal) {
			// switch ROM bank and call the sprite destroy handler
			SWITCH_ROM(spriteBanks[THIS->type]);
			spriteDestroyFuncs[THIS->type]();
			// return sprite to sprite pool
			StackPush(sprite_manager_sprites_pool, sprite_idx);
		} else {
			// shrink the vector
			VECTOR_SET_DIRECT(sprite_manager_updatables, current++, sprite_idx);
		}

	}
	// update the updateables vector length
	VECTOR_LEN(sprite_manager_updatables) = current;
	SWITCH_ROM(__save);
	// reset purge flag
	sprite_manager_purge = TRUE;
}
