#include "main.h"
#include "Flip.h"

#define _SPRITE(SPRITE_ID, DATA, FLIP) _SPRITE_(SPRITE_ID, DATA, FLIP)
#define _SPRITE_DMG(SPRITE_ID, DATA) _SPRITE_(SPRITE_ID, DATA, FLIP_NONE)
#define _SPRITE_DMG_FLIP(SPRITE_ID, DATA, FLIP) _SPRITE_(SPRITE_ID, DATA, FLIP)
#define _SPRITE_DMG_MIRROR(SPRITE_ID, DATA, FLIP) _SPRITE_(SPRITE_ID, DATA, FLIP)
#define _SPRITE_COLOR(SPRITE_ID, DATA) _SPRITE_(SPRITE_ID, DATA, FLIP_NONE)
#define _SPRITE_COLOR_FLIP(SPRITE_ID, DATA, FLIP) _SPRITE_(SPRITE_ID, DATA, FLIP)
#define _SPRITE_COLOR_MIRROR(SPRITE_ID, DATA, FLIP) _SPRITE_(SPRITE_ID, DATA, FLIP)
#define SPRITE_DEF_END

#define _STATE(STATE_ID) _STATE_(STATE_ID, StateDestroyDefault)
#define _STATE_EX(STATE_ID) _STATE_(STATE_ID, Destroy_##STATE_ID)
#define STATE_DEF_END

#define _STATE_(STATE_ID, DESTROY) STATE_ID,
typedef enum {
	STATES

	N_STATES
} STATE;
#undef _STATE_

#define _SPRITE_(SPRITE_ID, DATA, FLIP) SPRITE_ID,
typedef enum {
	SPRITES

	N_SPRITE_TYPES
} SPRITE_TYPE;
#undef _SPRITE_