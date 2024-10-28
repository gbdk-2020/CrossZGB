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

// define the CUSTOM_DATA struct which holds the pointer to the allocated coroutine context
typedef struct {
	void * ctx;
} CUSTOM_DATA;
CHECK_CUSTOM_DATA_SIZE(CUSTOM_DATA);
// define helper macro for the easy access to the allocated coroutine context pointer
#define THIS_CTX (((CUSTOM_DATA*)THIS->custom_data)->ctx)

// bullet sprite logic coroutine
// since we are calling the sprite coroutine from within the UPDATE handler only, THIS and THIS_IDX are valid
void SpriteBulletLogic(void * custom_data) BANKED {
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
		coro_yield();
	}
}

void START(void) {
	// allocate coroutine context, set coroutine function and pass THIS as data, remove sprite if failed
	if (!(THIS_CTX = coro_runner_alloc(SpriteBulletLogic, BANK(SpriteBullet), NULL))) SpriteManagerRemoveSprite(THIS);
}

void UPDATE(void) {
	// iterate coroutine, if it finishes normally - remove sprite
	if (!coro_runner_process(THIS_CTX)) SpriteManagerRemoveSprite(THIS);
}

void DESTROY(void) {
	// deallocate coroutine context
	coro_runner_free(THIS_CTX);
}