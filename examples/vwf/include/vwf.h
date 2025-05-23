#ifndef _VWF_H_INCLUDE
#define _VWF_H_INCLUDE

#include <gbdk/platform.h>

#include <stdint.h>

#include "Font.h"

#define __VWF_BANK_PREFIX(A) __bank_##A
#define TO_VWF_FARPTR(A) {.bank = (char)&(__VWF_BANK_PREFIX(A)), .ptr = (void *)&(A)}

#define RECODE_7BIT 1
#define FONT_VWF 2

#define VWF_MODE_RENDER 0
#define VWF_MODE_PRINT 1

typedef struct vwf_farptr_t {
    UINT8 bank;
    void * ptr;
} vwf_farptr_t;

typedef enum { 
    VWF_RENDER_BKG, 
    VWF_RENDER_WIN 
} vwf_reder_dest_e;

extern vwf_farptr_t vwf_fonts[4];
extern uint8_t vwf_mode;

void vwf_set_destination(vwf_reder_dest_e destination);
void vwf_load_font(uint8_t idx, const void * font, uint8_t bank);
void vwf_activate_font(uint8_t idx);
uint8_t vwf_draw_text(uint8_t x, uint8_t y, uint8_t base_tile, const unsigned char * str);
uint8_t vwf_next_tile(void);

inline void vwf_set_mode(uint8_t mode) {
   vwf_mode = mode; 
}
inline void vwf_set_colors(uint8_t fgcolor, uint8_t bgcolor) {
    set_1bpp_colors(fgcolor, bgcolor);
}

#define IMPORT_VWF_FONT(FONT) extern const void FONT##_font; extern const void __bank_##FONT
#define FONT(FONT) &FONT##_font

#endif