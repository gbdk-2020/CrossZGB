#include "Banks/SetAutoBank.h"

#include <stdbool.h>

#include "SpriteManager.h"
#include "Keys.h"
#include "Vector.h"
#include "ZGBMain.h"

#include "dizzy_anim.h"

// define the animation speed conatsnt
#define ANIMATION_SPEED      16
#define ANIMATION_JUMP_SPEED 25

// define physics values
#define MAX_Y_VELOCITY 32
#define MAX_X_VELOCITY 16
#define Y_ACCELERATION 1

// subpixel coordinates, velocities and jump flag
static INT16 x, y;
static INT16 yv, xv;
static bool jump;

static const UINT8 anim_idle[]       = VECTOR( 0,  1 , 2,  3,  4,  5 , 6 , 7);
static const UINT8 anim_jump_left[]  = VECTOR( 8,  9, 10, 11, 12, 13, 14, 15);
static const UINT8 anim_jump_right[] = VECTOR(16, 17, 18, 19, 20, 21, 22, 23);
static const UINT8 anim_jump[]       = VECTOR(24, 25, 26, 27, 28, 29, 30, 31);
static const UINT8 anim_walk_left[]  = VECTOR(32, 33, 34, 35, 36, 37, 38, 39);
static const UINT8 anim_walk_right[] = VECTOR(40, 41, 42, 43, 44, 45, 46, 47);

// movement limits for the sprite
#define LEFT_BOUND 0
#define RIGHT_BOUND ((SCREEN_WIDTH - (dizzy_anim_HEIGHT << 3)) << 4)
#define FLOOR_LEVEL ((SCREEN_HEIGHT - (dizzy_anim_HEIGHT << 3) - 8) << 4)

// define the animation handler for the Dizzy sprite
extern const metasprite_t dizzy_metasprite[];
static UINT8 __save;
struct metasprite_t * DizzyAnimHandler(Sprite * sprite, UINT8 anim_idx) NONBANKED {
	sprite;
	__save = CURRENT_BANK;
	SWITCH_ROM(BANK(dizzy_anim));
	// load tile data for the 9 8x8 Dizzy hardware sprites
	set_sprite_native_data(spriteIdxs[SpriteDizzy], 9, *(dizzy_anim + anim_idx));
	SWITCH_ROM(__save);
	// return address of the metasprite (it is the same for the each animation frame, because we animate tiledata) 
	return dizzy_metasprite;
}

void START(void) {
	// initialize x and y subpixel coordinates
	x = THIS->x << 4; y = THIS->y << 4;
	// set animation handler
	THIS->anim_handler = DizzyAnimHandler;
	// initialize the animation state
	SetSpriteAnim(THIS, anim_idle, ANIMATION_SPEED);
	// not jumping
	jump = false;
	// set x and y velocity
	yv = MAX_Y_VELOCITY; xv = 0;
}

void UPDATE(void) {
	// check D-Pad buttons and set x and y velocities, update the animation state
	if (!jump) {
		if (KEY_PRESSED(J_LEFT)) {
			if (KEY_PRESSED(J_UP | J_A)) {
				SetSpriteAnim(THIS, anim_jump_left, ANIMATION_JUMP_SPEED);
				yv = -MAX_Y_VELOCITY;
				jump = true;
			} else {
				SetSpriteAnim(THIS, anim_walk_left, ANIMATION_SPEED);
			}
			xv = -MAX_X_VELOCITY;
		} else if (KEY_PRESSED(J_RIGHT)) {
			if (KEY_PRESSED(J_UP | J_A)) {
				SetSpriteAnim(THIS, anim_jump_right, ANIMATION_JUMP_SPEED);
				yv = -MAX_Y_VELOCITY;
				xv = MAX_X_VELOCITY;
				jump = true;
			} else {
				SetSpriteAnim(THIS, anim_walk_right, ANIMATION_SPEED);
			}
			xv = MAX_X_VELOCITY;
		} else  if (KEY_PRESSED(J_UP | J_A)) {
			SetSpriteAnim(THIS, anim_jump, ANIMATION_JUMP_SPEED);
			yv = -MAX_Y_VELOCITY;
			jump = true;
		} else {
			SetSpriteAnim(THIS, anim_idle, ANIMATION_SPEED);
			xv = 0;
		}
	}

	// gravity
	if (yv < MAX_Y_VELOCITY) yv += Y_ACCELERATION;

	// x/y position
	x += xv; y += yv;

	// check bounds (should be replaced with collision handling
	if (x < LEFT_BOUND) x = LEFT_BOUND;
	if (x > RIGHT_BOUND) x = RIGHT_BOUND;
	if (y > FLOOR_LEVEL) y = FLOOR_LEVEL, jump = false, xv = 0;

	// update sprite X and Y position
	THIS->x = x >> 4; THIS->y = y >> 4;
}

void DESTROY(void) {
}
