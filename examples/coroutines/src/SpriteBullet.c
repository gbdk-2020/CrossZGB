#include "Banks/SetAutoBank.h"

#include <stdlib.h>

#include "SpriteManager.h"
#include "Coroutines.h"
#include "Keys.h"
#include "ZGBMain.h"

// player sprite pointer declaration, initialized in the SpritePlayer.c
extern Sprite * PLAYER;

// player sprite dimensions
#define PLAYER_WIDTH  16
#define PLAYER_HEIGHT 16
// aim to the center of the player sprite
#define TARGET_X_COORD (PLAYER->x + (PLAYER_WIDTH / 2))
#define TARGET_Y_COORD (PLAYER->y + (PLAYER_HEIGHT / 2))

// bullet sprite logic coroutine
// since we are calling the sprite coroutine from within the UPDATE handler only, THIS and THIS_IDX are valid
void BulletLogic(void * custom_data) BANKED {
	custom_data;
	// initialize the Bresenham algo, use local variables to hold the sprite logic state
	INT16 dx  =  abs(TARGET_X_COORD - THIS->x);
	INT16 dy  = -abs(TARGET_Y_COORD - THIS->y);
	INT16 err = dx + dy; 
	INT8  sx  = (TARGET_X_COORD > THIS->x) ? 1 : -1;
	INT8  sy  = (TARGET_Y_COORD > THIS->y) ? 1 : -1;
	// iterate the Bresenham algo 50 times, then exit
	for (UINT8 i = 50; i != 0; i--) {
		INT16 e2 = err << 1;
		if (e2 > dy) { err += dy; THIS->x += sx; }
		if (e2 < dx) { err += dx; THIS->y += sy; }
		// yield coroutine execution
		YIELD;
	}
}

SPRITE_COROUTINE(BulletLogic, NONE)
