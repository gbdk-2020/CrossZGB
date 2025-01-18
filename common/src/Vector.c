#include <string.h>

#include "Vector.h"

void BufferExchange(UINT8* v, UINT16 xy) NAKED {
	v; xy;
	__asm
#if defined(__TARGET_gb) || defined(__TARGET_ap) || defined(__TARGET_duck)
		ld a, e
		add c
		ld l, a
		adc d
		sub l
		ld h, a

		ld c, (hl)
		push hl
		 
		ld a, e
		add b
		ld l, a
		adc d
		sub l
		ld h, a

		ld a, (hl)
		ld (hl), c
		pop hl
		ld (hl), a
		
		ret
#elif defined(__TARGET_sms) || defined(__TARGET_gg)
		ld b, d
		ld c, e
		ex de, hl

		ld a, e
		add c
		ld l, a
		adc d
		sub l
		ld h, a

		ld c, (hl)
		push hl
		 
		ld a, e
		add b
		ld l, a
		adc d
		sub l
		ld h, a

		ld a, (hl)
		ld (hl), c
		pop hl
		ld (hl), a

		ret
#else
	#error Unrecognized port
#endif
	__endasm;
}

void BufferRotate(UINT8* v, UINT8 len) NAKED {
	v; len;
	__asm
#if defined(__TARGET_gb) || defined(__TARGET_ap) || defined(__TARGET_duck)
		sub #1
		ret c
		ret z
		ld c, a

		ld h, d
		ld l, e
		inc de
		
		ld b, (hl)
1$:
		ld a, (de)
		ld (hl+), a
		inc de
		dec c
		jr nz, 1$

		ld (hl), b
				
		ret
#elif defined(__TARGET_sms) || defined(__TARGET_gg)
		ex de, hl
		ld hl, #2
		add hl, sp
		ld a, (hl)
		sub #1
		ret c
		ret z

		ld c, a
		ld b, #0

		ld h, d
		ld l, e
		ld a, (de)

		inc hl
		ldir

		ld (de), a

		ret
#else
	#error Unrecognized port
#endif
	__endasm;
}

void VectorRemovePos(UINT8* v, UINT8 pos) {
	memcpy(&v[pos + 1], &v[pos + 2], (UINT8)((UINT8)(v[0]--) - pos));
}
