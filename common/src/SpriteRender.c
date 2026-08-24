#include "main.h"

#include "Vector.h"
#include "Sprite.h"

extern UINT8 delta_time;
extern UINT8 next_oam_idx;

extern INT16 sprite_screen_x, sprite_screen_y;

#if defined(SEGA)
void RenderSprite(void) NAKED {
__asm
	ld a, (_MAP_FRAME1)
	push af

	ld iy, (_THIS)

	ld a, ___offset_Sprite__mt_sprite_bank(iy)
	ld (_MAP_FRAME1), a

	ld a, (_shadow_VDP_R1)
	and #R1_SPR_8X16
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

	ld de, (_sprite_screen_y)
	push de
	ld de, (_sprite_screen_x)
	ld a, (_next_oam_idx)
	call ___move_metasprite
	jp 5$

2$:
	ld a, ___offset_Sprite__first_tile_H(iy)
	ld (___current_base_tile), a

	ld hl, (_sprite_screen_y)
	ld d, #0
	ld e, ___offset_Sprite__coll_h+0(iy)
	add hl, de
	add hl, bc
	push hl
	ld de, (_sprite_screen_x)
	ld a, (_next_oam_idx)
	call ___move_metasprite_flipy
	jp 5$

3$:
	ld a, ___offset_Sprite__first_tile_V(iy)
	ld (___current_base_tile), a

	ld de, (_sprite_screen_y)
	push de
	ld hl, (_sprite_screen_x)
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

	ld hl, (_sprite_screen_y)
	ld d, #0
	ld e, ___offset_Sprite__coll_h+0(iy)
	add hl, de
	add hl, bc
	push hl
	ld hl, (_sprite_screen_x)
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
#elif defined(NINTENDO)
void RenderSprite(void) {
	static UINT8 __save; 
	__save = CURRENT_BANK;
	SWITCH_ROM(THIS->mt_sprite_bank);

	__current_metasprite = THIS->mt_sprite;
	__current_base_prop = THIS->attr_add;

	switch(THIS->mirror) {
		case NO_MIRROR:
			__current_base_tile = THIS->first_tile;
			next_oam_idx += __move_metasprite(next_oam_idx, (sprite_screen_y << 8) | (uint8_t)sprite_screen_x);
			break;
		case H_MIRROR:  
			__current_base_tile = THIS->first_tile_H;
			next_oam_idx += __move_metasprite_flipy(next_oam_idx, ((sprite_screen_y + THIS->coll_h + ((LCDC_REG & LCDCF_OBJ16) ? -16 : -8)) << 8) | (uint8_t)sprite_screen_x);
			break;
		case V_MIRROR:  
			__current_base_tile = THIS->first_tile_V;
			next_oam_idx += __move_metasprite_flipx(next_oam_idx, (sprite_screen_y << 8) | (uint8_t)(sprite_screen_x + THIS->coll_w - 8));
			break;
		case (H_MIRROR | V_MIRROR):
		case HV_MIRROR: 
			__current_base_tile = THIS->first_tile_HV;
			next_oam_idx += __move_metasprite_flipxy(next_oam_idx, ((sprite_screen_y + THIS->coll_h + ((LCDC_REG & LCDCF_OBJ16) ? -16 : -8)) << 8) | (uint8_t)(sprite_screen_x + THIS->coll_w - 8));
			break;
	}
	SWITCH_ROM(__save);
}
#else
void RenderSprite(void) {
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
#endif
