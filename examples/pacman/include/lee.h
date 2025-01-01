#ifndef _LEE_H_INCLUDE
#define _LEE_H_INCLUDE

#include <gbdk/platform.h>

#define LEE_MAX_STEPS 128u

#define LEE_BITS 5
#define LEE_MASK 0x1fu

#define LEE_MAX_WIDTH (1 << LEE_BITS)
#define LEE_MAX_HEIGHT (1 << LEE_BITS) 

UINT8 lee_find_path(UINT8 x, UINT8 y, UINT8 dx, UINT8 dy);
UINT8 lee_restore_path(UINT8 x, UINT8 y, UINT16 * path);

inline UINT8 lee_get_coords_x(UINT16 pos) {
	return pos & LEE_MASK;
}
inline UINT8 lee_get_coords_y(UINT16 pos) {
	return pos >> LEE_BITS;
}

#endif