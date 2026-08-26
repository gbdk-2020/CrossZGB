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

#if defined(SEGA)
void InitSprite(Sprite* sprite, UINT8 sprite_type) NAKED {
	(void)sprite; (void)sprite_type;
__asm
	.ez80

	pop de
	dec sp
	ex de, hl         ; de: sprite
	ex (sp), hl

	ld c, h
	ld b, #0          ; bc: sprite_type

	ld iyl, e
	ld iyh, d         ; iy: sprite

	ld hl, #_spriteDatas
	add hl, bc
	add hl, bc
	ld e, (hl)
	inc hl
	ld d, (hl)        ; de: mt_sprite_info
	ld ___offset_Sprite__mt_sprite_info+0(iy), e
	ld ___offset_Sprite__mt_sprite_info+1(iy), d

	push de
	ex (sp), ix       ; ix: mt_sprite_info

	ld hl, #_spriteDataBanks
	add hl, bc
	ld a, (hl)
	ld ___offset_Sprite__mt_sprite_bank(iy), a

	ld hl, #_spriteFlips
	add hl, bc
	ld a, (hl)
	ld ___offset_Sprite__flips(iy), a
	ld hl, #_spriteIdxs
	add hl, bc
	ld a, (hl)
	ld ___offset_Sprite__first_tile(iy), a
	ld hl, #_spriteIdxsH
	add hl, bc
	ld a, (hl)
	ld ___offset_Sprite__first_tile_H(iy), a
	ld hl, #_spriteIdxsV
	add hl, bc
	ld a, (hl)
	ld ___offset_Sprite__first_tile_V(iy), a
	ld hl, #_spriteIdxsHV
	add hl, bc
	ld a, (hl)
	ld ___offset_Sprite__first_tile_HV(iy), a

	ld ___offset_Sprite__anim_data+0(iy), #0
	ld ___offset_Sprite__anim_data+1(iy), #0

	ld ___offset_Sprite__anim_speed(iy), #33

	push iy
	ld hl, #0
	ld e, iyl
	ld d, iyh
	ex de, hl
	call _GetSpriteAnimation
	pop iy
	ld ___offset_Sprite__mt_sprite+0(iy), e
	ld ___offset_Sprite__mt_sprite+1(iy), d
	ld ___offset_Sprite__anim_frame(iy), #0

	ld ___offset_Sprite__ctx+0(iy), #0
	ld ___offset_Sprite__ctx+1(iy), #0

	ld ___offset_Sprite__x+0(iy), #0
	ld ___offset_Sprite__x+1(iy), #0

	ld ___offset_Sprite__y+0(iy), #0
	ld ___offset_Sprite__y+1(iy), #0

	ld ___offset_Sprite__coll_group(iy), #COLL_GR_DEFAULT
	ld ___offset_Sprite__coll_group_down(iy), #COLL_GR_DOWN

	ld ___offset_Sprite__flags(iy), #0b00000101
	
	ld a, (_MAP_FRAME1)
	ld e, a
	ld a, ___offset_Sprite__mt_sprite_bank(iy)
	ld (_MAP_FRAME1), a

	ld a, ___offset_MetaSpriteInfo__width(ix)
	ld ___offset_Sprite__coll_w(iy), a
	ld a, ___offset_MetaSpriteInfo__height(ix)
	ld ___offset_Sprite__coll_h(iy), a

	ld a, e
	ld (_MAP_FRAME1), a
	pop ix

	ret	
__endasm;
}
#else
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
#endif

INT16 sprite_screen_x, sprite_screen_y;
void RenderSprite(void);

void TickAnimSprite(void);

void DrawSprite(void) {
	// calculate screen coordinates
	sprite_screen_x = THIS->x - scroll_x;
	sprite_screen_y = THIS->y - scroll_y;

	// tick sprite animation
	TickAnimSprite();

	// if invisible skip rendering
	if (THIS->visible) {
		// render sprite on screen or remove it
		if (
			((UINT16)(sprite_screen_x + MAXIMUM_SPRITES_SIZE) < (UINT16)(DEVICE_SCREEN_PX_WIDTH + (MAXIMUM_SPRITES_SIZE << 1))) &&
			((UINT16)(sprite_screen_y + MAXIMUM_SPRITES_SIZE) < (UINT16)(DEVICE_SCREEN_PX_HEIGHT + (MAXIMUM_SPRITES_SIZE << 1)))
		) {
			// don't draw if too far off screen to avoid "ghost sprites" because of the move_metasprite_ex() coordinate overflow or not visible
			sprite_screen_x += (DEVICE_SPRITE_PX_OFFSET_X + SCREEN_SPR_OFFSET_X);
			sprite_screen_y += DEVICE_SPRITE_PX_OFFSET_Y;

			// render sprite on screen
			return RenderSprite();
		}
	}

	if (THIS->persistent) return;
	
	// check sprite for removal 
	if (
		((UINT16)(sprite_screen_x + THIS->lim_x + 16u) > (UINT16)((THIS->lim_x << 1) + (DEVICE_SCREEN_PX_WIDTH + (16u << 1)))) || 
		((UINT16)(sprite_screen_y + THIS->lim_y + 16u) > (UINT16)((THIS->lim_y << 1) + (DEVICE_SCREEN_PX_HEIGHT + (16u << 1))))
	) {
		SpriteManagerRemoveSprite(THIS);
	}
}
