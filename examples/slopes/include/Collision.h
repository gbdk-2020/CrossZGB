#ifndef __COLLISION_H_INCLUDE__
#define __COLLISION_H_INCLUDE__

#include <gbdk/platform.h>

#define SUBPX_TO_PX(a)   (a >> 4)
#define SUBPX_TO_TILE(a) (a >> 7)
#define PX_TO_SUBPX(a)   (a << 4)
#define TILE_TO_SUBPX(a) (a << 7)
#define VEL_TO_SUBPX(a)  (a >> 8)

#define COLLISION_NONE                0x00u
#define COLLISION_TOP                 0x01u
#define COLLISION_BOTTOM              0x02u
#define COLLISION_LEFT                0x04u
#define COLLISION_RIGHT               0x08u
#define COLLISION_ALL                 0x0Fu

#define COLLISION_SLOPE_LEFT          0x10u
#define COLLISION_SLOPE_45            0x20u
#define COLLISION_SLOPE_225_BOT       0x40u
#define COLLISION_SLOPE_225_TOP       (COLLISION_SLOPE_45 | COLLISION_SLOPE_225_BOT)

#define COLLISION_SLOPE_45_RIGHT      COLLISION_SLOPE_45
#define COLLISION_SLOPE_225_RIGHT_BOT COLLISION_SLOPE_225_BOT
#define COLLISION_SLOPE_225_RIGHT_TOP COLLISION_SLOPE_225_TOP
#define COLLISION_SLOPE_45_LEFT       (COLLISION_SLOPE_LEFT | COLLISION_SLOPE_45)
#define COLLISION_SLOPE_225_LEFT_BOT  (COLLISION_SLOPE_LEFT | COLLISION_SLOPE_225_BOT) 
#define COLLISION_SLOPE_225_LEFT_TOP  (COLLISION_SLOPE_LEFT | COLLISION_SLOPE_225_TOP)
#define COLLISION_SLOPE_ANY           (COLLISION_SLOPE_45 | COLLISION_SLOPE_225_BOT | COLLISION_SLOPE_225_TOP)
#define COLLISION_SLOPE               0xF0u

#define DIV_2(a)                ((a) >> 1)
#define EXCLUSIVE_OFFSET(x)     ((x) + 1)
#define SUBPX_TILE_REMAINDER(a) ((UINT8)((a) & 0x7F))

// from gbvm/include/collision.h

extern UINT8 tile_hit_x;
extern UINT8 tile_hit_y;

/**
 * Test for a tile matching mask in a vertical range from ty_start to ty_end at column tx.
 * Updates globals tile_hit_x and tile_hit_y which can be read afterwards to determine which tile matched
 * 
 * @param tile_mask Tile bitmask to match
 * @param tx Tile x-coordinate
 * @param ty_start Starting tile y-coordinate
 * @param ty_end Ending tile y-coordinate
 * @return First matching tile value found, or 0 if none matched, COLLISION_ALL if out of bounds
 */
UINT8 tile_col_test_range_y(UINT8 tile_mask, UINT8 tx, UINT8 ty_start, UINT8 ty_end) NONBANKED;

/**
 * Test for a tile matching mask in a horizontal range from tx_start to tx_end at row ty.
 * Updates globals tile_hit_x and tile_hit_y which can be read afterwards to determine which tile matched
 *
 * @param tile_mask Tile bitmask to match
 * @param ty Tile y-coordinate
 * @param tx_start Starting tile x-coordinate
 * @param tx_end Ending tile x-coordinate
 * @return First matching tile value found, or 0 if none matched, COLLISION_ALL if out of bounds
 */
UINT8 tile_col_test_range_x(UINT8 tile_mask, UINT8 ty, UINT8 tx_start, UINT8 tx_end) NONBANKED;

#endif
