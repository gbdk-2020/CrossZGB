#include "vwf.h"

#define DEVICE_TILE_SIZE 8u

vwf_farptr_t vwf_fonts[4];

uint8_t vwf_current_offset = 0;
uint8_t vwf_tile_data[DEVICE_TILE_SIZE * 2];
uint8_t vwf_current_mask;
uint8_t vwf_current_rotate;
uint8_t vwf_inverse_map = 0;
uint8_t vwf_current_tile;
uint8_t vwf_mode = VWF_MODE_PRINT;

#if defined(NINTENDO_NES)
// GSINIT code is not used in gbdk-nes. Make sure this is allocated to _DATA instead of _BSS, and has an initial value.
uint8_t * vwf_render_base_address = (uint8_t*)0x2000;
#else
uint8_t * vwf_render_base_address;
#endif

font_desc_t vwf_current_font_desc;
uint8_t vwf_current_font_bank;

#if defined(NINTENDO)
void vwf_print_shift_char(void * dest, const void * src, uint8_t bank) OLDCALL;
void vwf_memcpy(void* to, const void* from, size_t n, uint8_t bank) OLDCALL;
uint8_t vwf_read_banked_ubyte(const void * src, uint8_t bank) OLDCALL __preserves_regs(b, c) ;
uint8_t * vwf_get_win_addr(void) OLDCALL __preserves_regs(b, c, h, l) ;
uint8_t * vwf_get_bkg_addr(void) OLDCALL __preserves_regs(b, c, h, l) ;
void vwf_set_banked_data(uint8_t i, uint8_t l, const unsigned char* ptr, uint8_t bank) OLDCALL;
void vwf_swap_tiles(void) OLDCALL;
#elif defined(NINTENDO_NES)
void vwf_print_shift_char(void * dest, const void * src, uint8_t bank);
void vwf_memcpy(void* to, const void* from, size_t n, uint8_t bank);
uint8_t vwf_read_banked_ubyte(const void * src, uint8_t bank);
uint8_t * vwf_get_win_addr(void);
uint8_t * vwf_get_bkg_addr(void);
void vwf_set_banked_data(uint8_t i, uint8_t l, const unsigned char* ptr, uint8_t bank);
void vwf_swap_tiles(void);
#elif defined(SEGA)
void vwf_print_shift_char(void * dest, const void * src, uint8_t bank) Z88DK_CALLEE;
void vwf_memcpy(void* to, const void* from, size_t n, uint8_t bank) Z88DK_CALLEE;
uint8_t vwf_read_banked_ubyte(const void * src, uint8_t bank) Z88DK_CALLEE;
uint8_t * vwf_get_win_addr(void) OLDCALL;
uint8_t * vwf_get_bkg_addr(void) OLDCALL;
void vwf_set_banked_data(uint8_t i, uint8_t l, const unsigned char* ptr, uint8_t bank) Z88DK_CALLEE;
void vwf_swap_tiles(void) OLDCALL;
#endif

void vwf_set_destination(vwf_reder_dest_e destination) {
    vwf_render_base_address = (destination == VWF_RENDER_BKG) ? vwf_get_bkg_addr() : vwf_get_win_addr();
}

void vwf_print_reset(uint8_t tile) {
    vwf_current_tile = tile;
    vwf_current_offset = 0;
    vwf_swap_tiles(); 
    vwf_swap_tiles(); 
}

uint8_t vwf_print_render(const unsigned char ch) {
    uint8_t letter = vwf_read_banked_ubyte(vwf_current_font_desc.recode_table + (ch & ((vwf_current_font_desc.attr & RECODE_7BIT) ? 0x7fu : 0xffu)), vwf_current_font_bank);
    const uint8_t * bitmap = vwf_current_font_desc.faces + (uint16_t)letter * 8;
    if (vwf_current_font_desc.attr & FONT_VWF) {
        uint8_t width = vwf_read_banked_ubyte(vwf_current_font_desc.widths + letter, vwf_current_font_bank);
        uint8_t dx = (8u - vwf_current_offset);
        vwf_current_mask = (0xffu << dx) | (0xffu >> (vwf_current_offset + width));

        vwf_current_rotate = vwf_current_offset;
        vwf_print_shift_char(vwf_tile_data, bitmap, vwf_current_font_bank);
        if ((uint8_t)(vwf_current_offset + width) > 8u) {
            vwf_current_rotate = dx | 0x80u;
            vwf_current_mask = 0xffu >> (width - dx);
            vwf_print_shift_char(vwf_tile_data + DEVICE_TILE_SIZE, bitmap, vwf_current_font_bank);
        }
        vwf_current_offset += width;

        if (vwf_current_offset > 7u) {
            vwf_current_offset -= 8u;
            set_bkg_1bpp_data(vwf_current_tile, (vwf_current_offset) ? 2 : 1, vwf_tile_data);
            vwf_current_tile++;
            vwf_swap_tiles();
            return TRUE;
        };
        set_bkg_1bpp_data(vwf_current_tile, 1, vwf_tile_data);
        return FALSE;
    } else {
        vwf_set_banked_data(vwf_current_tile++, 1, bitmap, vwf_current_font_bank);
        vwf_current_offset = 0;
        return TRUE;
    }
}

uint8_t vwf_draw_text(uint8_t x, uint8_t y, uint8_t base_tile, const unsigned char * str) {
    static uint8_t * ui_dest_base, *ui_dest_ptr;
    static const uint8_t * ui_text_ptr;
    ui_dest_ptr = ui_dest_base = vwf_render_base_address + (y + DEVICE_SCREEN_Y_OFFSET) * (DEVICE_SCREEN_BUFFER_WIDTH * DEVICE_SCREEN_MAP_ENTRY_SIZE) + ((x + DEVICE_SCREEN_X_OFFSET) * DEVICE_SCREEN_MAP_ENTRY_SIZE);
    ui_text_ptr = str;

    vwf_print_reset(base_tile);
    while (*ui_text_ptr) {
        switch (*ui_text_ptr) {
            case 0x01:
                vwf_activate_font(*++ui_text_ptr);
                break;
            case 0x02:
                ui_dest_ptr = ui_dest_base = vwf_render_base_address + (*++ui_text_ptr + DEVICE_SCREEN_Y_OFFSET) * (DEVICE_SCREEN_BUFFER_WIDTH * DEVICE_SCREEN_MAP_ENTRY_SIZE) + ((*++ui_text_ptr + DEVICE_SCREEN_X_OFFSET) * DEVICE_SCREEN_MAP_ENTRY_SIZE);
                if (vwf_current_offset) vwf_print_reset(vwf_current_tile + 1u);
                break; 
            case 0x03:
                vwf_inverse_map = *++ui_text_ptr;
                break;
            case '\n':
                ui_dest_ptr = ui_dest_base += (DEVICE_SCREEN_BUFFER_WIDTH * DEVICE_SCREEN_MAP_ENTRY_SIZE);
                if (vwf_current_offset) vwf_print_reset(vwf_current_tile + 1u);
                break; 
            default:
                if (vwf_print_render(*ui_text_ptr)) {
                    if (vwf_mode & VWF_MODE_PRINT) set_vram_byte(ui_dest_ptr, vwf_current_tile - 1);
                    ui_dest_ptr += DEVICE_SCREEN_MAP_ENTRY_SIZE;
                }
                if ((vwf_current_offset) && (vwf_mode & VWF_MODE_PRINT)) set_vram_byte(ui_dest_ptr, vwf_current_tile);
                break;
        }
        ui_text_ptr++;
    }
    return vwf_next_tile() - base_tile;
}

void vwf_load_font(uint8_t idx, const void * font, uint8_t bank) {
    vwf_fonts[idx].bank = bank;
    vwf_fonts[idx].ptr = (void *)font;
    vwf_activate_font(idx); 
}

void vwf_activate_font(uint8_t idx) {
    vwf_current_font_bank = vwf_fonts[idx].bank;
    vwf_memcpy(&vwf_current_font_desc, vwf_fonts[idx].ptr, sizeof(font_desc_t), vwf_current_font_bank);    
}

uint8_t vwf_next_tile(void) {
    return (vwf_current_offset) ? vwf_current_tile + 1u : vwf_current_tile;
}