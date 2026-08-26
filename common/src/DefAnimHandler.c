#include <gbdk/platform.h>

#include "MetaSpriteInfo.h"
#include "Sprite.h"

// default animation change handler

#if defined(SEGA)
const metasprite_t * GetSpriteAnimation(Sprite * sprite, UINT16 anim_idx) NAKED {
	(void)sprite; (void)anim_idx;
__asm
	.ez80

	ex de, hl
	ld iyl, e
	ld iyh, d

	ld a, (_MAP_FRAME1)
	ld c, a
	ld a, ___offset_Sprite__mt_sprite_bank(iy)
	ld (_MAP_FRAME1), a

	ld e, ___offset_Sprite__mt_sprite_info+0(iy)
	ld d, ___offset_Sprite__mt_sprite_info+1(iy)
	ld iyl, e
	ld iyh, d
	ld e, ___offset_MetaSpriteInfo__metasprites+0(iy)
	ld d, ___offset_MetaSpriteInfo__metasprites+1(iy)
	ex de, hl
	add hl, de
	add hl, de
	ld e, (hl)
	inc hl
	ld d, (hl)

	ld a, c
	ld (_MAP_FRAME1), a

	ret
__endasm;
}
#else
const metasprite_t * GetSpriteAnimation(Sprite * sprite, UINT16 anim_idx) {
	static UINT8 __save;
	__save = CURRENT_BANK;
	SWITCH_ROM(sprite->mt_sprite_bank);
	struct metasprite_t * res = sprite->mt_sprite_info->metasprites[anim_idx];
	SWITCH_ROM(__save);
	return res;
}
#endif