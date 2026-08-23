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

#ifndef MAXIMUM_SPRITES_SIZE
	#define MAXIMUM_SPRITES_SIZE 32
#endif

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
	sprite->anim_speed = 33u;

	SetFrame(sprite, 0);

	sprite->ctx = NULL;

	sprite->x = 0;
	sprite->y = 0;

	sprite->coll_group = COLL_GROUP_DEFAULT;
	sprite->coll_group_down = COLL_GROUP_DOWN;

	SetVisible(sprite, TRUE);
	SetPersistent(sprite, FALSE);
	SetAnimationLoop(sprite, TRUE);

	UINT8 __save = CURRENT_BANK;
	SWITCH_ROM(spriteDataBanks[sprite_type]);
	sprite->coll_w = mt_sprite_info->width;
	sprite->coll_h = mt_sprite_info->height;
	SWITCH_ROM(__save);
}

void SetSpriteAnim(Sprite* sprite, const UINT8* data, UINT8 speed) {
	if (sprite->anim_data == data) return;
		
	sprite->anim_data = (UINT8* )data;
	SetFrame(sprite, VECTOR_GET(data, 0));
	sprite->anim_frame = 0;
	sprite->anim_accum_ticks = 0;
	sprite->anim_speed = speed;
}

extern UINT8 delta_time;
extern UINT8 next_oam_idx;

static INT16 screen_x, screen_y;

#if defined(NINTENDO)
static void doDrawSprite(void) {
	static UINT8 __save; 
	__save = CURRENT_BANK;
	SWITCH_ROM(THIS->mt_sprite_bank);
	switch(THIS->mirror) {
		case NO_MIRROR: next_oam_idx += move_metasprite_ex    (THIS->mt_sprite, THIS->first_tile,    THIS->attr_add, next_oam_idx, screen_x,                screen_y               ); break;
		case H_MIRROR:  next_oam_idx += move_metasprite_flipy (THIS->mt_sprite, THIS->first_tile_H,  THIS->attr_add, next_oam_idx, screen_x,                screen_y + THIS->coll_h); break;
		case V_MIRROR:  next_oam_idx += move_metasprite_flipx (THIS->mt_sprite, THIS->first_tile_V,  THIS->attr_add, next_oam_idx, screen_x + THIS->coll_w, screen_y               ); break;
		case (H_MIRROR | V_MIRROR):
		case HV_MIRROR: next_oam_idx += move_metasprite_flipxy(THIS->mt_sprite, THIS->first_tile_HV, THIS->attr_add, next_oam_idx, screen_x + THIS->coll_w, screen_y + THIS->coll_h); break;
	}
	SWITCH_ROM(__save);
}
#elif defined(SEGA)
static void doDrawSprite(void) NAKED {
__asm
	ld a, (_MAP_FRAME1)
	push af

	ld iy, (_THIS)

	ld a, ___offset_Sprite__mt_sprite_bank(iy)
	ld (_MAP_FRAME1), a

	ld a, (_shadow_VDP_R1)
	and #0x02
	jp z, 7$
	ld bc, #-16
	jp 8$
7$:
	ld bc, #-8
8$:
	ld l, ___offset_Sprite__mt_sprite+0(iy)
	ld h, ___offset_Sprite__mt_sprite+1(iy)
	ld (___current_metasprite), hl

	ld d, #0
	ld e, ___offset_Sprite__mirror(iy)
	ld a, #4
	sub e
	jp c, 6$
	ld hl, #0$
	add hl, de
	add hl, de
	ld e, (hl)
	inc hl
	ld h, (hl)
	ld l, e
	jp (hl)

0$:
	.dw 1$
	.dw 2$
	.dw 3$
	.dw 4$
	.dw 4$

1$:
	ld a, ___offset_Sprite__first_tile(iy)
	ld (___current_base_tile), a

	ld de, (_screen_y)
	push de
	ld de, (_screen_x)
	ld a, (_next_oam_idx)
	call ___move_metasprite
	jp 5$

2$:
	ld a, ___offset_Sprite__first_tile_H(iy)
	ld (___current_base_tile), a

	ld hl, (_screen_y)
	ld d, #0
	ld e, ___offset_Sprite__coll_h+0(iy)
	add hl, de
	add hl, bc
	push hl
	ld de, (_screen_x)
	ld a, (_next_oam_idx)
	call ___move_metasprite_flipy
	jp 5$

3$:
	ld a, ___offset_Sprite__first_tile_V(iy)
	ld (___current_base_tile), a

	ld de, (_screen_y)
	push de
	ld hl, (_screen_x)
	ld d, #0
	ld e, ___offset_Sprite__coll_w+0(iy)
	add hl, de
	ld de, #-8
	add hl, de
	ex de, hl
	ld a, (_next_oam_idx)
	call ___move_metasprite_flipx
	jp 5$

4$:
	ld a, ___offset_Sprite__first_tile_HV(iy)
	ld (___current_base_tile), a

	ld hl, (_screen_y)
	ld d, #0
	ld e, ___offset_Sprite__coll_h+0(iy)
	add hl, de
	add hl, bc
	push hl
	ld hl, (_screen_x)
	ld d, #0
	ld e, ___offset_Sprite__coll_w+0(iy)
	add hl, de
	ld de, #-8
	add hl, de
	ex de, hl
	ld a, (_next_oam_idx)
	call ___move_metasprite_flipxy

5$:
	ld hl, #_next_oam_idx
	add (hl)
	ld (hl), a

6$:
	pop af
	ld (_MAP_FRAME1), a
	ret
__endasm;
}
#endif

void DrawSprite(void) {
	// calculate screen coordinates
	screen_x = THIS->x - scroll_x;
	screen_y = THIS->y - scroll_y;

	// tick sprite animation
	if (THIS->anim_data) {
		THIS->anim_accum_ticks += (THIS->anim_speed << delta_time);
		if (THIS->anim_accum_ticks >= SPRITE_ANIM_MAX_TICKS) {
			THIS->anim_accum_ticks -= SPRITE_ANIM_MAX_TICKS;

			if (++THIS->anim_frame >= VECTOR_LEN(THIS->anim_data)) {
				if (THIS->loop_anim) {
					THIS->anim_frame = 0;
				} else {
					--THIS->anim_frame;
				}
			}
			THIS->mt_sprite = GetSpriteAnimation(THIS, VECTOR_GET(THIS->anim_data, THIS->anim_frame));
		}
	}

	// if invisible skip rendering
	if (THIS->visible) {
		// render sprite on screen or remove it
		if (
			((UINT16)(screen_x + MAXIMUM_SPRITES_SIZE) < (UINT16)(DEVICE_SCREEN_PX_WIDTH + (MAXIMUM_SPRITES_SIZE << 1))) &&
			((UINT16)(screen_y + MAXIMUM_SPRITES_SIZE) < (UINT16)(DEVICE_SCREEN_PX_HEIGHT + (MAXIMUM_SPRITES_SIZE << 1)))
		) {
			// don't draw if too far off screen to avoid "ghost sprites" because of the move_metasprite_ex() coordinate overflow or not visible
			screen_x += (DEVICE_SPRITE_PX_OFFSET_X + SCREEN_SPR_OFFSET_X);
			screen_y += DEVICE_SPRITE_PX_OFFSET_Y;

			// render sprite on screen
			doDrawSprite();

			return;
		}
	}

	if (THIS->persistent) return;
	
	// check sprite for removal 
	if (
		((UINT16)(screen_x + THIS->lim_x + 16u) > (UINT16)((THIS->lim_x << 1) + (DEVICE_SCREEN_PX_WIDTH + (16u << 1)))) || 
		((UINT16)(screen_y + THIS->lim_y + 16u) > (UINT16)((THIS->lim_y << 1) + (DEVICE_SCREEN_PX_HEIGHT + (16u << 1))))
	) {
		SpriteManagerRemoveSprite(THIS);
	}

}
