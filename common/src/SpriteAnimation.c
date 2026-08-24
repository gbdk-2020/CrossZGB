#include "main.h"

#include "Vector.h"
#include "Sprite.h"

#if defined(SEGA)
void TickAnimSprite(void) NAKED {
__asm
	ld iy, (_THIS)
	ld l, ___offset_Sprite__anim_data+0(iy) 
	ld h, ___offset_Sprite__anim_data+1(iy)      ; hl: THIS->anim_data
	ld a, l
	or h
	ret z

	ld a, (_delta_time)
	or a
	ld a, ___offset_Sprite__anim_speed(iy)
	jp z, 0$
	add a
0$:
	add ___offset_Sprite__anim_accum_ticks(iy)
	ld ___offset_Sprite__anim_accum_ticks(iy), a

	cp #SPRITE_ANIM_MAX_TICKS
	ret c

	sub #SPRITE_ANIM_MAX_TICKS
1$:
	cp #SPRITE_ANIM_MAX_TICKS
	jp c, 2$
	sub #SPRITE_ANIM_MAX_TICKS
2$:
	ld ___offset_Sprite__anim_accum_ticks(iy), a

	ld a, ___offset_Sprite__anim_frame(iy)
	inc a
	cp (hl)
	jp c, 4$

	bit 2, ___offset_Sprite__flags(iy)
	jp z, 3$
	ld a, #1
3$:
	dec a 
4$:
	ld ___offset_Sprite__anim_frame(iy), a
	ld e, a
	ld d, #0
	add hl, de
	inc hl
	ld e, (hl)
	ld hl, (_THIS)
	call _GetSpriteAnimation
	ld iy, (_THIS)
	ld ___offset_Sprite__mt_sprite+0(iy), e 
	ld ___offset_Sprite__mt_sprite+1(iy), d
	ret
__endasm;
}
#else
void TickAnimSprite(void) {
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
}
#endif
