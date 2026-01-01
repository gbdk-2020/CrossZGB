#include "main.h"

#include "Vector.h"
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
	sprite->anim_data = NULL;
	
	SetFrame(sprite, 0);

	sprite->anim_speed = 33u;

	sprite->ctx = NULL;

	sprite->x = 0;
	sprite->y = 0;

	sprite->coll_group = COLL_GROUP_DEFAULT;
	sprite->coll_group_down = COLL_GROUP_DOWN;

	SetVisible(sprite, TRUE);
	SetPersistent(sprite, FALSE);

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
			if ((VECTOR_GET(THIS->anim_data, THIS->anim_frame + 1) == ANIM_STOP) || (THIS->anim_frame == ANIM_STOP)) {
				THIS->anim_frame = ANIM_STOP;
			} else {
				if (++THIS->anim_frame >= VECTOR_LEN(THIS->anim_data)) {
					THIS->anim_frame = 0;
				}
			}
 
			UINT8 tmp = 0; // Do this before changing banks, anim_data is stored on current bank
			if (THIS->anim_frame == ANIM_STOP) {
				// if ANIM_STOP is used, then load the frame before ANIM_STOP
				tmp = VECTOR_GET(THIS->anim_data, VECTOR_LEN(THIS->anim_data) - 2);
			} else {
				tmp = VECTOR_GET(THIS->anim_data, THIS->anim_frame);
			}
			UINT8 __save = CURRENT_BANK;
			SWITCH_ROM(THIS->mt_sprite_bank);
				THIS->mt_sprite = THIS->mt_sprite_info->metasprites[tmp];
			SWITCH_ROM(__save);
		}
	}

	if (
		(THIS->visible) &&
		(
			((UINT16)(screen_x + 32u) < (UINT16)(DEVICE_SCREEN_PX_WIDTH + 32u)) &&
			((UINT16)(screen_y + 32u) < (UINT16)(DEVICE_SCREEN_PX_HEIGHT + 32u))
		)
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
				case (H_MIRROR | V_MIRROR):
				case HV_MIRROR: next_oam_idx += move_metasprite_flipxy(THIS->mt_sprite, THIS->first_tile_HV, THIS->attr_add, next_oam_idx, screen_x + THIS->coll_w, screen_y + THIS->coll_h); break;
			}
		SWITCH_ROM(__save);
	} else {
		// check sprite for removal 
		if (
			(!(THIS->persistent)) && 
			(
				((UINT16)(screen_x + THIS->lim_x + 16u) > (UINT16)((THIS->lim_x << 1) + (DEVICE_SCREEN_PX_WIDTH + 16u))) || 
				((UINT16)(screen_y + THIS->lim_y + 16u) > (UINT16)((THIS->lim_y << 1) + (DEVICE_SCREEN_PX_HEIGHT + 16u)))
			)
		) {
			return SpriteManagerRemoveSprite(THIS);
		}
	}

}

UINT8 CheckCollision(Sprite* sprite1, Sprite* sprite2) {
	if ((sprite1->x + sprite1->coll_w) < sprite2->x) return 0;
	if ((sprite2->x + sprite2->coll_w) < sprite1->x) return 0;
	if ((sprite1->y + sprite1->coll_h) < sprite2->y) return 0;
	if ((sprite2->y + sprite2->coll_h) < sprite1->y) return 0;
	return 1;
}