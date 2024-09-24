#include "Banks/SetAutoBank.h"

#include <stdlib.h>

#include "SpriteManager.h"
#include "Keys.h"
#include "ZGBMain.h"

#include "coroutines_runner.h"

// player sprite pointer declaration, initialized in the SpritePlayer.c
extern Sprite * PLAYER;

// player sprite dimensions
#define PLAYER_WIDTH  16
#define PLAYER_HEIGHT 16
// aim to the center of the player sprite
#define TARGET_X_COORD (PLAYER->x + (PLAYER_WIDTH / 2))
#define TARGET_Y_COORD (PLAYER->y + (PLAYER_HEIGHT / 2))

typedef struct {
	void * ctx;
} CUSTOM_DATA;
CHECK_CUSTOM_DATA_SIZE(CUSTOM_DATA);

// bullet sprite logic coroutine
// use local variables to hold the sprite logic state
void SpriteBulletLogic(void * user_data) BANKED {
	// initialize the Bresenham algo state
	INT16 dx  =  abs(TARGET_X_COORD - ((Sprite *)user_data)->x);
	INT16 dy  = -abs(TARGET_Y_COORD - ((Sprite *)user_data)->y);
	INT16 err = dx + dy; 
	INT8  sx  = (TARGET_X_COORD > ((Sprite *)user_data)->x) ? 1 : -1;
	INT8  sy  = (TARGET_Y_COORD > ((Sprite *)user_data)->y) ? 1 : -1;

	// iterate one step of the Bresenham algo 50 times, then exit
	for (UINT8 i = 50; i != 0; i--) {
		INT16 e2 = err << 1;
		if (e2 > dy) { err += dy; ((Sprite *)user_data)->x += sx; }
		if (e2 < dx) { err += dx; ((Sprite *)user_data)->y += sy; }

		coro_yield();
	}
}

void START(void) {
	CUSTOM_DATA * data = (CUSTOM_DATA*)THIS->custom_data;
	// allocate coroutine context, set coroutine function and pass THIS as data
	data->ctx = coro_runner_alloc(SpriteBulletLogic, BANK(SpriteBullet), (void *)THIS);
}

void UPDATE(void) {
	void * ctx = ((CUSTOM_DATA*)THIS->custom_data)->ctx;
	// if context was allocated, iterate coroutine, otherwise remove sprite
	if (ctx) { 
		// iterate coroutine, if it finishes normally - remove sprite
		if (!coro_runner_process(ctx)) SpriteManagerRemove(THIS_IDX);
	} else SpriteManagerRemove(THIS_IDX);
}

void DESTROY(void) {
	// deallocate coroutine context
	coro_runner_free(((CUSTOM_DATA*)THIS->custom_data)->ctx);
}