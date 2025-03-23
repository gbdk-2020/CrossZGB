        .include        "global.s"

        .globl _vwf_current_rotate, _vwf_current_mask, _vwf_inverse_map, _vwf_tile_data, _vwf_inverse_map
        .globl _set_bkg_1bpp_data

        .area OSEG (PAG, OVR)
        _vwf_memcpy_PARM_2::             .ds 2      ; Note: Overlay PARM must exactly match ___memcpy_PARM_2
        _vwf_memcpy_PARM_3::             .ds 2      ; Note: Overlay PARM must exactly match ___memcpy_PARM_3
        _vwf_memcpy_PARM_4::             .ds 1
        _vwf_read_banked_ubyte_PARM_2::
        _vwf_draw_frame_row_PARM_2::     .ds 1
        _vwf_draw_frame_row_PARM_3::     .ds 1
        _vwf_print_shift_char_PARM_2::   .ds 2
        _vwf_print_shift_char_PARM_3::   .ds 1
        
        .area GBDKOVR (PAG, OVR)
        _vwf_set_banked_data_PARM_3::    .ds 2      ; Note: Overlay PARM must exactly match _set_bkg_1bpp_data_PARM_3
        _vwf_set_banked_data_PARM_4::    .ds 1

        .area _CODE

; void vwf_memcpy(void* to, const void* from, size_t n, UBYTE bank);
_vwf_memcpy::
        sta *REGTEMP
        lda *__current_bank
        pha
        lda *_vwf_memcpy_PARM_4
        jsr __switch_prg0
        lda *REGTEMP
        jsr ___memcpy
        pla
        jsr __switch_prg0
        rts

; UBYTE vwf_read_banked_ubyte(const void * src, UBYTE bank)
_vwf_read_banked_ubyte::
        sta *REGTEMP
        stx *REGTEMP+1
        lda *__current_bank
        sta *REGTEMP+2
        lda *_vwf_read_banked_ubyte_PARM_2
        jsr __switch_prg0
        ldy #0
        lda [*REGTEMP],y
        tax
        lda *REGTEMP+2
        tay
        jsr __switch_prg0
        txa
        rts

; void vwf_set_banked_bkg_data(UBYTE i, UBYTE l, const unsigned char* ptr, UBYTE bank);
_vwf_set_banked_data::
        sta *REGTEMP
        lda *__current_bank
        pha
        lda *_vwf_set_banked_data_PARM_4
        jsr __switch_prg0
        lda *REGTEMP
        jsr _set_bkg_1bpp_data
        pla
        jsr __switch_prg0
        rts

; void vwf_print_shift_char(void * dest, const void * src, UBYTE bank);
_vwf_print_shift_char::
        .define .dest "REGTEMP"
        .local .src
        .src = _vwf_print_shift_char_PARM_2        
        .local .bank
        .bank = _vwf_print_shift_char_PARM_3
        sta *.dest
        stx *.dest+1
        ; Save old bank and switch to new 
        lda *__current_bank
        pha
        lda *.bank
        jsr __switch_prg0
        ;
        ldy #7
1$:
        ; RT = *src++ ^ _vwf_inverse_map
        lda [*.src],y
        eor _vwf_inverse_map
        ldx _vwf_current_rotate
        jsr .ui_print_shift_char_shift_CR
        sta *REGTEMP+2
        ; *dest++ = (*dest & _vwf_current_mask) | (RT & ~_vwf_current_mask)
        lda [*.dest],y
        and _vwf_current_mask
        tax
        lda _vwf_current_mask
        eor #0xFF
        and *REGTEMP+2
        ora .identity,x
        sta [*.dest],y
        dey
        bpl 1$
        ; Restore old bank and return
        pla
        jsr __switch_prg0
        rts

.ui_print_shift_char_shift_CR:
        bmi 1$
        beq 3$
0$:
        lsr
        dex
        bne 0$
        rts
1$:
        cpx #0x80
        beq 3$
2$:
        asl
        dex
        cpx #0x80
        bne 2$
3$:
        rts

; UBYTE * vwf_get_bkg/win_addr()
_vwf_get_bkg_addr::
_vwf_get_win_addr::
        lda #<0x2000
        ldx #>0x2000
        rts

_vwf_swap_tiles::
        ; Copy _vwf_tile_data+8 to _vwf_tile_data
        ldy #7
1$:
        lda _vwf_tile_data+8,y
        sta _vwf_tile_data,y
        dey
        bpl 1$
        ; Fill _vwf_tile_data+8 with _vwf_inverse_map
        ldy #7
        lda _vwf_inverse_map
2$:
        sta _vwf_tile_data+8,y
        dey
        bpl 2$
        rts
