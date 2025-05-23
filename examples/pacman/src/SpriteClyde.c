#include "Banks/SetAutoBank.h"

#include "SpriteManager.h"
#include "Coroutines.h"
#include "ZGBMain.h"

// global reference to Blinky
Sprite * CLYDE;

// declare the logic coroutine
BANKREF_EXTERN(ClydeLogic)
void ClydeLogic(void * custom_data) BANKED;

void START(void) {
	// make sprites not die offscreen
	THIS->lim_x = THIS->lim_y = 256;
	// allocate coroutine context
	INIT_CORO(BANK(ClydeLogic), ClydeLogic);
	// initialize global reference
	CLYDE = THIS;
}

void UPDATE(void) {
	// iterate coroutine
	ITER_CORO;
}

void DESTROY(void) {
	// remove global reference
	CLYDE = NULL;
	// deallocate coroutine context
	FREE_CORO;
}