#include <gbdk/platform.h>

#include "SpriteManager.h"
#include "Vector.h"

void sort_actor_zorder(void) {
	// using the simplest bubble sort method
	UINT8 n = VECTOR_LEN(sprite_manager_updatables);
	UINT8 swapped;  
	for (int i = 0; i < n - 1; i++) {
		swapped = FALSE;
		for (int j = 0; j < n - i - 1; j++) {
			UINT8 n1 = VECTOR_GET(sprite_manager_updatables, j);
			UINT8 n2 = VECTOR_GET(sprite_manager_updatables, j + 1);
			// we need to calculate and compare the bottom edges of collision box
			INT16 y1 = sprite_manager_sprites[n1]->y + sprite_manager_sprites[n1]->coll_h;
			INT16 y2 = sprite_manager_sprites[n2]->y + sprite_manager_sprites[n2]->coll_h;
			if (y1 < y2) {
				VECTOR_SET_DIRECT(sprite_manager_updatables, j, n2);
				VECTOR_SET_DIRECT(sprite_manager_updatables, j + 1, n1);
				swapped = TRUE;
			}
		}
		if (!swapped) break;
	}
}