#include "Vector.h"
#include "Scroll.h"
#include "Sprite.h"
#include "SpriteManager.h"
#include "Math.h"
#include "main.h"

// To be defined on the main app
UINT8 GetTileReplacement(UINT8* tile_ptr, UINT8* tile);

// global variables, updated before each call to UPDATE_TILE()
extern unsigned char* scroll_ptr;
extern unsigned char* scroll_cptr;
extern unsigned char* dest_ptr;
extern unsigned char* dest_cptr;

#if defined(SEGA)
static INT16 __x, __y;
static UINT8 __type, __save, __i;
void UPDATE_TILE(INT16 x, INT16 y) NAKED {
	(void)x; (void)y;
__asm
	.ez80

	ld (___x), hl
	ld (___y), de

	ld hl, (___x)
	ld de, (_scroll_tiles_w)
	or a
	sbc hl, de
	jr nc, 0$
	ld hl, (___y)
	ld de, (_scroll_tiles_h)
	or a
	sbc hl, de
	jr c, 1$
0$:
	ld hl, (_dest_ptr)
	ld (hl), #0
	jp 2$

1$:
	ld hl, (_scroll_ptr)
	ld de, (_dest_ptr)
	ld a, (hl)
	ld (de), a

	call _GetTileReplacement
	ld (___type), a

	inc a
	jp z, 2$

	ld bc, #_sprite_manager_updatables
	ld a, (bc)
	or a
	jp z, 4$
	ld hl, #___i
	ld (hl), a

5$:
	ld l, a
	ld h, #0x00
	add hl, bc
	ld l, (hl)
	ld h, #0x00
	add hl, hl
	ld de, #_sprite_manager_sprites
	add hl, de
	ld e, (hl)
	inc hl
	ld d, (hl)
	ld iyl, e
	ld iyh, d

	ld a, (___type)
	cp ___offset_Sprite__type(iy)
	jp nz, 6$
	ld a, (_scroll_ptr+0)
	cp ___offset_Sprite__unique_id+0(iy)
	jp nz, 6$
	ld a, (_scroll_ptr+1)
	cp ___offset_Sprite__unique_id+1(iy)
	jp z, 2$

6$:
	ld hl, #___i
	dec (hl)
	ld a, (hl)
	or a
	jp nz, 5$

4$:
	ld a, (_MAP_FRAME1)
	ld (___save), a

	ld hl, #_spriteDataBanks
	ld a, (___type)
	ld e, a
	ld d, #0x00
	add hl, de
	ld a, (hl)
	ld (_MAP_FRAME1), a

	ld hl, (___y)
	inc hl
	add hl, hl
	add hl, hl
	add hl, hl
	ex de, hl
	ld h, #0x00
	ld a, (___type)
	ld l, a
	add hl, hl
	ld bc, #_spriteDatas
	add hl, bc
	ld a, (hl)
	inc hl
	ld h, (hl)	
	add #___offset_MetaSpriteInfo__height
	ld l, a
	adc h
	sub l
	ld h, a
	ld c, (hl)
	ld b, #0x00

	ex de, hl
	or a
	sbc hl, bc

	ld a, (___save)
	ld (_MAP_FRAME1), a

	ld de, (_scroll_ptr)
	push de                    ; scroll_ptr
	push hl                    ; sprite_y
	ld hl, (___x)
	add hl, hl
	add hl, hl
	add hl, hl
	ex de, hl                  ; sprite_x
	ld a, (___type)
	call _SpriteManagerAddEx

2$:
	ld hl, (_scroll_cptr)
	ld a, h
	or l
	ld a, (hl)
	jp nz, 3$
	ld de, #_scroll_tile_info
	ld hl, (_dest_ptr)
	ld l, (hl)
	ld h, #0
	add hl, de
	ld a, (hl)
3$:
	ld bc, (_dest_cptr)
	ld (bc), a

	ret
__endasm;
}
#else
void UPDATE_TILE(INT16 x, INT16 y) {
	if(((UINT16)x >= scroll_tiles_w) || ((UINT16)y >= scroll_tiles_h)) { //This also checks x < 0 || y < 0
		*dest_ptr = 0;
	} else {
		static UINT8 type;
		*dest_ptr = *scroll_ptr;
		type = GetTileReplacement(scroll_ptr, dest_ptr);
		if(type != 255u) {
			static UINT8 i, __save;
			for (i = VECTOR_LEN(sprite_manager_updatables); (i); i--) {
				Sprite* s = sprite_manager_sprites[sprite_manager_updatables[i]];
				if ((s->type == type) && (s->unique_id == (UINT16)scroll_ptr)) {
					break;
				}
			}
			if (i == 0) {
				__save = CURRENT_BANK;
				SWITCH_ROM(spriteDataBanks[type]);
				UINT16 sprite_y = TILE_TO_PX(y + 1) - spriteDatas[type]->height;
				SWITCH_ROM(__save);
				SpriteManagerAddEx(type, TILE_TO_PX(x), sprite_y, scroll_ptr);
			}
		}
	}
#if defined(SEGA) || (defined(NINTENDO) && defined(CGB))
	*dest_cptr = (scroll_cptr) ? *scroll_cptr : scroll_tile_info[*dest_ptr];
#endif
}
#endif