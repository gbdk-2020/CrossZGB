#include "main.h"

#include "vector.h"
#include "Sprite.h"
#include "Scroll.h"
#include "SpriteManager.h"
#include "MetaSpriteInfo.h"

#if defined(MASTERSYSTEM)
#define SCREEN_SPR_OFFSET_X   8
#else
#define SCREEN_SPR_OFFSET_X   0
#endif

void SetFrame(Sprite* sprite, UINT8 frame)
{
	UINT8 __save = CURRENT_BANK;
	SWITCH_ROM(sprite->mt_sprite_bank);
		sprite->mt_sprite = sprite->mt_sprite_info->metasprites[frame];
	SWITCH_ROM(__save);
	sprite->anim_frame = frame; //anim_frame contains the animation frame if anim_data is assigned or the metasprite index otherwise
}

void InitSprite(Sprite* sprite, UINT8 sprite_type) {
	const struct MetaSpriteInfo* mt_sprite_info = spriteDatas[sprite_type];

	sprite->mt_sprite_info = mt_sprite_info;
	sprite->mt_sprite_bank = spriteDataBanks[sprite_type];

	sprite->flips = spriteFlips[sprite_type];
	sprite->first_tile = spriteIdxs[sprite_type];
	sprite->first_tile_H = spriteIdxsH[sprite_type];
	sprite->first_tile_V = spriteIdxsV[sprite_type];
	sprite->first_tile_HV = spriteIdxsHV[sprite_type];
#ifdef CGB
	sprite->attr_add = (_cpu == CGB_TYPE) ? spritePalsOffset[sprite_type] : 0;
#else
	sprite->attr_add = 0;
#endif
	sprite->anim_data = 0u;
	
	SetFrame(sprite, 0);

	sprite->anim_speed = 33u;

	sprite->x = 0;
	sprite->y = 0;

	sprite->visible = 1u;

	UINT8 __save = CURRENT_BANK;
	SWITCH_ROM(spriteDataBanks[sprite_type]);
		sprite->coll_w = mt_sprite_info->width;
		sprite->coll_h = mt_sprite_info->height;
	SWITCH_ROM(__save);
}

void SetSpriteAnim(Sprite* sprite, const UINT8* data, UINT8 speed) {
	if (sprite->anim_data != data) {
		sprite->anim_data = (UINT8* )data;
		SetFrame(sprite, VECTOR_GET(data, 0));
		sprite->anim_frame = 0;
		sprite->anim_accum_ticks = 0;
		sprite->anim_speed = speed;
	}
}

extern UINT8 delta_time;
extern UINT8 next_oam_idx;
void DrawSprite(void) {
	static INT16 screen_x, screen_y;
	screen_x = THIS->x - scroll_x;
	screen_y = THIS->y - scroll_y;

	// tick sprite animation
	if (THIS->anim_data) {	
		THIS->anim_accum_ticks += THIS->anim_speed << delta_time;
		if (THIS->anim_accum_ticks > 100u) {
			THIS->anim_accum_ticks -= 100u;

			if (++THIS->anim_frame >= VECTOR_LEN(THIS->anim_data)) {
				THIS->anim_frame = 0;
			}
 
			UINT8 tmp = VECTOR_GET(THIS->anim_data, THIS->anim_frame); // Do this before changing banks, anim_data is stored on current bank
			UINT8 __save = CURRENT_BANK;
			SWITCH_ROM(THIS->mt_sprite_bank);
				THIS->mt_sprite = THIS->mt_sprite_info->metasprites[tmp];
			SWITCH_ROM(__save);
		}
	}

	if (
		((UINT16)(screen_x + 32u) < (UINT16)(DEVICE_SCREEN_PX_WIDTH + 32u)) &&
		((UINT16)(screen_y + 32u) < (UINT16)(DEVICE_SCREEN_PX_HEIGHT + 32u)) &&
		(THIS->visible)
	) {
		// don't draw if too far off screen to avoid "ghost sprites" because of the move_metasprite_ex() coordinate overflow or not visible
		screen_x += (DEVICE_SPRITE_PX_OFFSET_X + SCREEN_SPR_OFFSET_X);
		screen_y += DEVICE_SPRITE_PX_OFFSET_Y;

		// render sprite on screen
		UINT8 __save = CURRENT_BANK;
		SWITCH_ROM(THIS->mt_sprite_bank);
			switch(THIS->mirror) {
				case NO_MIRROR: next_oam_idx += move_metasprite_ex    (THIS->mt_sprite, THIS->first_tile,    THIS->attr_add, next_oam_idx, screen_x,                screen_y               ); break;
				case H_MIRROR:  next_oam_idx += move_metasprite_flipy (THIS->mt_sprite, THIS->first_tile_H,  THIS->attr_add, next_oam_idx, screen_x,                screen_y + THIS->coll_h); break;
				case V_MIRROR:  next_oam_idx += move_metasprite_flipx (THIS->mt_sprite, THIS->first_tile_V,  THIS->attr_add, next_oam_idx, screen_x + THIS->coll_w, screen_y               ); break;
				case HV_MIRROR: next_oam_idx += move_metasprite_flipxy(THIS->mt_sprite, THIS->first_tile_HV, THIS->attr_add, next_oam_idx, screen_x + THIS->coll_w, screen_y + THIS->coll_h); break;
			}
		SWITCH_ROM(__save);
	} else {
		// check sprite for removal 
		if (
			((UINT16)(screen_x + THIS->lim_x + 16u) > (UINT16)((THIS->lim_x << 1) + (DEVICE_SCREEN_PX_WIDTH + 16u))) || 
			((UINT16)(screen_y + THIS->lim_y + 16u) > (UINT16)((THIS->lim_y << 1) + (DEVICE_SCREEN_PX_HEIGHT + 16u)))
		) {
			return SpriteManagerRemoveSprite(THIS);
		}
	}

}

unsigned char* tile_coll;
UINT8 TranslateSprite(Sprite* sprite, INT8 x, INT8 y) {
	UINT8 ret = 0;
	INT16 pivot_x, pivot_y;
	UINT8 start_tile_x, end_tile_x;
	UINT8 start_tile_y, end_tile_y;
	UINT8* scroll_coll_v;
	UINT8 tmp;
	if (x) {
		if (x > 0) {
			pivot_x = sprite->x + (UINT8)(sprite->coll_w - 1u);
		} else {
			pivot_x = sprite->x;
		}
		
		//Check tile change
		tmp = pivot_x >> 3;
		pivot_x += x;
		start_tile_x = pivot_x >> 3;
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
			start_tile_y = sprite->y >> 3;
		}

		//end_tile_y clamped between scroll limits
		pivot_y = sprite->y + sprite->coll_h - 1;
		if((UINT16)pivot_y >= scroll_h) { //This checks pivot_y < 0 || pivot_y >= scroll_h
			if(pivot_y < 0) 
				end_tile_y = 0;
			else  
				end_tile_y = scroll_tiles_h - 1;
		}	else {
			end_tile_y = pivot_y >> 3;
		}

		UINT8 __save = CURRENT_BANK;
		SWITCH_ROM(scroll_bank);
		tile_coll = scroll_map + (scroll_tiles_w * start_tile_y + start_tile_x);
		end_tile_y ++;
		for(tmp = start_tile_y; tmp != end_tile_y; tmp ++, tile_coll += scroll_tiles_w) {
			if(scroll_collisions[*tile_coll] == 1u) {
				if(x > 0) {
					sprite->x = (start_tile_x << 3) - sprite->coll_w;
				} else {
					sprite->x = (start_tile_x + 1) << 3;
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
		tmp = pivot_y >> 3;
		pivot_y += y;
		start_tile_y = pivot_y >> 3;
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
			start_tile_x = (sprite->x >> 3);
		}

		//end_tile_x clamped between scroll limits
		pivot_x = sprite->x + sprite->coll_w - 1;
		if((UINT16)pivot_x >= scroll_w) {
			if(pivot_x < 0) 
				end_tile_x = 0;
			else 
				end_tile_x = scroll_tiles_w - 1;
		}	else {
			end_tile_x = (pivot_x >> 3);
		}

		UINT8 __save = CURRENT_BANK;
		SWITCH_ROM(scroll_bank);
		tile_coll = scroll_map + (scroll_tiles_w * start_tile_y + start_tile_x);
		end_tile_x ++;
		scroll_coll_v = y < 0 ? scroll_collisions : scroll_collisions_down;
		for(tmp = start_tile_x; tmp != end_tile_x; tmp ++, tile_coll += 1) {
			if(scroll_coll_v[*tile_coll] == 1u) {
				if(y > 0) {
					sprite->y = (start_tile_y << 3) - sprite->coll_h;
				} else {
					sprite->y = (start_tile_y + 1) << 3;
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

UINT8 CheckCollision(Sprite* sprite1, Sprite* sprite2) {
	if ((sprite1->x + sprite1->coll_w) < sprite2->x) return 0;
	if ((sprite2->x + sprite2->coll_w) < sprite1->x) return 0;
	if ((sprite1->y + sprite1->coll_h) < sprite2->y) return 0;
	if ((sprite2->y + sprite2->coll_h) < sprite1->y) return 0;
	return 1;
}