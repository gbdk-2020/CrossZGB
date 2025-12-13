#include "main.h"

#include "Math.h"
#include "Sprite.h"
#include "Scroll.h"

static UINT8* tile_coll;
UINT8 TranslateSprite(Sprite* sprite, INT8 x, INT8 y) {
	UINT8 ret = 0;
	INT16 pivot_x, pivot_y;
	UINT8 start_tile_x, end_tile_x;
	UINT8 start_tile_y, end_tile_y;
	UINT8 scroll_coll_group;
	UINT8 tmp;
	if (x) {
		if (x > 0) {
			pivot_x = sprite->x + (UINT8)(sprite->coll_w - 1u);
		} else {
			pivot_x = sprite->x;
		}
		
		//Check tile change
		tmp = PX_TO_TILE(pivot_x);
		pivot_x += x;
		start_tile_x = PX_TO_TILE(pivot_x);
		if(tmp == start_tile_x) {
			goto inc_x;
		}
		
		//Check scroll bounds
		if((UINT16)pivot_x >= scroll_w) { //This checks pivot_x < 0 || pivot_x >= scroll_W
			goto inc_x;
		}

		//start_tile_y clamped between scroll limits
		if(sprite->y >= scroll_h) { //This checks sprite->y < 0 || sprite->y >= scroll_h
			if((INT16)sprite->y < 0) 
				start_tile_y = 0;
			else 
				start_tile_y = scroll_tiles_h - 1;
		} else {
			start_tile_y = PX_TO_TILE(sprite->y);
		}

		//end_tile_y clamped between scroll limits
		pivot_y = sprite->y + sprite->coll_h - 1;
		if((UINT16)pivot_y >= scroll_h) { //This checks pivot_y < 0 || pivot_y >= scroll_h
			if(pivot_y < 0) 
				end_tile_y = 0;
			else  
				end_tile_y = scroll_tiles_h - 1;
		}	else {
			end_tile_y = PX_TO_TILE(pivot_y);
		}

		UINT8 __save = CURRENT_BANK;
		SWITCH_ROM(scroll_bank);
		tile_coll = scroll_map + (scroll_tiles_w * start_tile_y + start_tile_x);
		end_tile_y ++;
		scroll_coll_group = sprite->coll_group;
		for(tmp = start_tile_y; tmp != end_tile_y; tmp ++, tile_coll += scroll_tiles_w) {
			if(scroll_collisions[*tile_coll] & scroll_coll_group) {
				if(x > 0) {
					sprite->x = TILE_TO_PX(start_tile_x) - sprite->coll_w;
				} else {
					sprite->x = TILE_TO_PX(start_tile_x + 1);
				}

				ret = *tile_coll;
				SWITCH_ROM(__save);
				goto done_x;
			}
		}
		SWITCH_ROM(__save);
	}

inc_x:
	sprite->x += x;
done_x:
	
	if (y) {
		if (y > 0) {
			pivot_y = sprite->y + (UINT8)(sprite->coll_h - 1u);
		} else {
			pivot_y = sprite->y;
		}

		//Check tile change
		tmp = PX_TO_TILE(pivot_y);
		pivot_y += y;
		start_tile_y = PX_TO_TILE(pivot_y);
		if(tmp == start_tile_y) {
			goto inc_y;
		}

		//Check scroll bounds
		if((UINT16)pivot_y >= scroll_h) {
			goto inc_y;
		}

		//start_tile_x clamped between scroll limits
		if(sprite->x >= scroll_w){
			if((INT16)sprite->x < 0) 
				start_tile_x = 0;
			else 
				start_tile_x = scroll_tiles_w - 1;
		}	else { 
			start_tile_x = PX_TO_TILE(sprite->x);
		}

		//end_tile_x clamped between scroll limits
		pivot_x = sprite->x + sprite->coll_w - 1;
		if((UINT16)pivot_x >= scroll_w) {
			if(pivot_x < 0) 
				end_tile_x = 0;
			else 
				end_tile_x = scroll_tiles_w - 1;
		}	else {
			end_tile_x = PX_TO_TILE(pivot_x);
		}

		UINT8 __save = CURRENT_BANK;
		SWITCH_ROM(scroll_bank);
		tile_coll = scroll_map + (scroll_tiles_w * start_tile_y + start_tile_x);
		end_tile_x ++;
		scroll_coll_group = y < 0 ? sprite->coll_group : sprite->coll_group_down;
		for(tmp = start_tile_x; tmp != end_tile_x; tmp ++, tile_coll += 1) {
			if(scroll_collisions[*tile_coll] & scroll_coll_group) {
				if(y > 0) {
					sprite->y = TILE_TO_PX(start_tile_y) - sprite->coll_h;
				} else {
					sprite->y = TILE_TO_PX(start_tile_y + 1);
				}

				ret = *tile_coll;
				SWITCH_ROM(__save);
				goto done_y;
			}
		}
		SWITCH_ROM(__save);
	}

inc_y:
	sprite->y += y;
done_y:

	return ret;
}
