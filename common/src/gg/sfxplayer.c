#include <gbdk/platform.h>

#include "Sound.h"

volatile UINT8 sfx_play_bank = SFX_STOP_BANK;
const UINT8 * sfx_play_sample = NULL;
UINT8 sfx_frame_skip;

UINT8 sfx_play_isr(void) NONBANKED NAKED {
#if defined(__SDCC)
#if defined(SEGA)
__asm
        ld de, (_sfx_play_sample)
        ld a, d
        or e
        ret z                       ; return FALSE

        ld hl, #_sfx_frame_skip
        xor a
        or (hl)
        jr z, 7$
        dec (hl)
        ret                         ; A != 0 that returns TRUE
7$:
	ex de, hl

        ld a, (_MAP_FRAME1)         ; save bank and switch
        ld e, a
        ld a, (_sfx_play_bank)
        inc a                       ; SFX_STOP_BANK ?
        ret z                       ; return FALSE
        dec a
        ld (_MAP_FRAME1), a

        ld d, #0x0f
        ld a, (hl)
        rlca
        rlca
        rlca
        rlca
        and d
        ld (_sfx_frame_skip), a

        ld a, (hl)
        inc hl
        and d
        ld b, a                     ; b = frame channel count
        jp z, 0$                    ; length 0 == terminator
	cp d
	jp z, 0$                    ; length 0x0f == empty row

        ld c, #_PSG
        otir

6$:
        inc b                       ; return TRUE if still playing
0$:
        ld (_sfx_play_sample), hl   ; save current position

        ld a, e                     ; restore bank
        ld (_MAP_FRAME1), a

        ld a, b                     ; result in a

        ret
__endasm;
#else
__asm
        xor a
        ld l, a
        ret
__endasm;
#endif
#endif
}

void sfx_sound_cut_mask(uint8_t mask) NAKED {
    mask;
#if defined(__SDCC)
#if defined(SEGA)
    __asm
        and #0b00001111
        ret z                       ; if nothing to retrigger then return
        ld c, #_PSG                 ; c points to the PSG port
        ld hl, #2$                  ; hl points to the muting data
0$:
        srl a
        jr nc, 1$
        outi

        or a
        ret z
        jr 0$
1$:
        inc hl
        jr 0$
2$:
        .irp ch,0,1,2,3
            .db #(PSG_LATCH | (ch << 5) | PSG_VOLUME | 0x0f)
        .endm
    __endasm;
#else
__asm
        ret
__endasm;
#endif
#endif
}
