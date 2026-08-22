#include "Sprite.h"

#if defined(NINTENDO)
UINT8 CheckCollision(Sprite* sprite1, Sprite* sprite2) {
	if ((sprite1->x + sprite1->coll_w) < sprite2->x) return 0;
	if ((sprite2->x + sprite2->coll_w) < sprite1->x) return 0;
	if ((sprite1->y + sprite1->coll_h) < sprite2->y) return 0;
	if ((sprite2->y + sprite2->coll_h) < sprite1->y) return 0;
	return 1;
}
#elif defined(SEGA)
UINT8 CheckCollision(Sprite* sprite1, Sprite* sprite2) NAKED {
	(void)sprite1; (void)sprite2;
__asm
	.ez80

	push ix

	ld iyh, d
	ld iyl, e    ; iy: sprite2

	ex de, hl
	ld ixh, d
	ld ixl, e    ; ix: sprite1

	xor a        ; 0 for the early return
	ld d, a

;if ((sprite1->x + sprite1->coll_w) < sprite2->x) return 0;

	ld l, ___offset_Sprite__x+0(ix)
	ld h, ___offset_Sprite__x+1(ix)
	ld e, ___offset_Sprite__coll_w(ix)
	add hl, de

	ld c, ___offset_Sprite__x+0(iy)
	ld b, ___offset_Sprite__x+1(iy)
	sbc hl, bc
	jp c, 0$

;if ((sprite2->x + sprite2->coll_w) < sprite1->x) return 0;

	ld l, ___offset_Sprite__x+0(iy)
	ld h, ___offset_Sprite__x+1(iy)
	ld e, ___offset_Sprite__coll_w(iy)
	add hl, de

	ld c, ___offset_Sprite__x+0(ix)
	ld b, ___offset_Sprite__x+1(ix)
	sbc hl, bc
	jp c, 0$

;if ((sprite1->y + sprite1->coll_h) < sprite2->y) return 0;

	ld l, ___offset_Sprite__y+0(ix)
	ld h, ___offset_Sprite__y+1(ix)
	ld e, ___offset_Sprite__coll_h(ix)
	add hl, de

	ld c, ___offset_Sprite__y+0(iy)
	ld b, ___offset_Sprite__y+1(iy)
	sbc hl, bc
	jp c, 0$

;if ((sprite2->y + sprite2->coll_h) < sprite1->y) return 0;

	ld l, ___offset_Sprite__y+0(iy)
	ld h, ___offset_Sprite__y+1(iy)
	ld e, ___offset_Sprite__coll_h(iy)
	add hl, de

	ld c, ___offset_Sprite__y+0(ix)
	ld b, ___offset_Sprite__y+1(ix)
	sbc hl, bc
	jp c, 0$

	ld a, #1     ; return 1 
0$:
	pop ix
	ret
__endasm;
}
#endif