#ifndef PALETTE_H
#define PALETTE_H

#include <gbdk/platform.h>

#if defined(NINTENDO)

#define PAL_DEF(C3, C2, C1, C0) (UINT8)((C0 << 4 << 2) | (C1 << 4) | (C2 << 2) | C3)

#ifdef CGB

#define PALETTE_INDEX(PAL, IDX) PAL##CGBPal##IDX##c0, PAL##CGBPal##IDX##c1, PAL##CGBPal##IDX##c2, PAL##CGBPal##IDX##c3                       
#define PALETTE_FROM_HEADER(PAL) PALETTE_INDEX(PAL, 0), \
                                 PALETTE_INDEX(PAL, 1), \
                                 PALETTE_INDEX(PAL, 2), \
                                 PALETTE_INDEX(PAL, 3), \
                                 PALETTE_INDEX(PAL, 4), \
                                 PALETTE_INDEX(PAL, 5), \
                                 PALETTE_INDEX(PAL, 6), \
                                 PALETTE_INDEX(PAL, 7)

#define PAL_RED(C)   (((C)      ) & 0x1Fu)
#define PAL_GREEN(C) (((C) >>  5) & 0x1Fu)
#define PAL_BLUE(C)  (((C) >> 10) & 0x1Fu)

#define MAX_PALETTES     8
#define N_PALETTE_COLORS 4

#else
#define PALETTE_INDEX(PAL, IDX)  0
#define PALETTE_FROM_HEADER(PAL) 0
#endif

#define WAIT_WRITABLE_CRAM

#elif defined(SEGA)

#if defined(MASTERSYSTEM)
#define PAL_RED(C)   (((C)      ) & 0x03u)
#define PAL_GREEN(C) (((C) >>  2) & 0x03u)
#define PAL_BLUE(C)  (((C) >>  4) & 0x03u)
#else
#define PAL_RED(C)   (((C)      ) & 0x0Fu)
#define PAL_GREEN(C) (((C) >>  4) & 0x0Fu)
#define PAL_BLUE(C)  (((C) >>  8) & 0x0Fu)
#endif

#define MAX_PALETTES     1
#define N_PALETTE_COLORS 16

#define WAIT_WRITABLE_CRAM while (VCOUNTER < 216) {}

#else 
#define WAIT_WRITABLE_CRAM
#endif

#if defined(SEGA) || (defined(NINTENDO) && defined(CGB))

#define PALETTE_SIZE (sizeof(palette_color_t) * N_PALETTE_COLORS)
#define N_TOTAL_COLORS (N_PALETTE_COLORS * MAX_PALETTES)

extern palette_color_t ZGB_Fading_BPal[N_TOTAL_COLORS];
extern palette_color_t ZGB_Fading_SPal[N_TOTAL_COLORS];

extern UINT8 last_bg_pal_loaded;

typedef enum {
	BG_PALETTE,
	SPRITES_PALETTE
} PALETTE_TYPE;

INT8 SetPalette(PALETTE_TYPE t, UINT8 first_palette, UINT8 nb_palettes, const palette_color_t *rgb_data, UINT8 bank);

void SetDefaultColorPalettes(void) BANKED;

#endif

#if defined(SEGA)
inline void SetBorderColor(palette_color_t color) {
	set_sprite_palette_entry(0, (__READ_VDP_REG(VDP_R7) & 0x0fu), ZGB_Fading_SPal[0] = color);
}
#else
#define SetBorderColor(A)
#endif

#endif