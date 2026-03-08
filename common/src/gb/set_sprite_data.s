        .include        "global.s"

        .area   _HOME

_set_sprite_native_data_wrap::
        ld c, e     ; Number of tiles
        ld e, a     ; ID of the first tile
        ldhl sp, #2
        ld a, (hl+) ; source data pointer
        ld h, (hl)
        ld l, a
        
        ; Compute dest ptr
        swap e ; *16 (size of a tile)
        ld a, e
        and #0x0F ; Get high bits
        add #0x80 ; Add base offset of target tile "block"
        ld d, a
        ld a, e
        and #0xF0 ; Get low bits only
        ld e, a
2$:
        ; Wrap from past $8FFF to $8000 onwards
        res 4, d
1$:
        ld b, #16
3$:
        WAIT_STAT
        ld a, (hl+)
        ld (de), a
        inc de
        
        dec b
        jr nz, 3$

        dec c
        jr nz, 2$
        
        pop hl
        pop af
        jp (hl)
