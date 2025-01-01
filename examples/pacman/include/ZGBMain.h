#ifndef ZGBMAIN_H
#define ZGBMAIN_H

#define STATES \
_STATE_EX(StateGame)\
_STATE_EX(StateSplashZGB)\
_STATE_EX(StateMenu)\
STATE_DEF_END

#define SPRITES \
_SPRITE(SpritePlayer,      player,      FLIP_NONE)\
_SPRITE(SpriteInky,        inky,        FLIP_NONE)\
_SPRITE(SpritePinky,       pinky,       FLIP_NONE)\
_SPRITE(SpriteBlinky,      blinky,      FLIP_NONE)\
_SPRITE(SpriteClyde,       clyde,       FLIP_NONE)\
_SPRITE(SpriteGhostScared, ghostscared, FLIP_NONE)\
_SPRITE(SpriteGhostEaten,  ghosteaten,  FLIP_NONE)\
SPRITE_DEF_END

#include "ZGBMain_Init.h"

// define directions enum
typedef enum {
	DIR_NONE = 0,
	DIR_UP,
	DIR_DOWN,
	DIR_LEFT,
	DIR_RIGHT,
	N_DIRECTIONS
} direction_e;

// tile coodrdinates structure
typedef union position_t {
	struct {
		UINT8 x;
		UINT8 y;
	};
	UINT16 v;
} position_t;

#define IS_ODD_FRAME ((UINT8)sys_time & 1)
#define IS_EVEN_FRAME (!((UINT8)sys_time & 1))

#endif