#include "Banks/SetAutoBank.h"

#include "SpriteManager.h"
#include "Coroutines.h"
#include "ZGBMain.h"

// global reference to Blinky
Sprite * BLINKY;

// declare the logic coroutine
BANKREF_EXTERN(BlinkyLogic)
void BlinkyLogic(void * custom_data) BANKED;

void START(void) {
	// make sprites not die offscreen
	THIS->lim_x = THIS->lim_y = 256;
	// allocate coroutine context
	INIT_CORO(BANK(BlinkyLogic), BlinkyLogic);
	// initialize global reference
	BLINKY = THIS;
}

void UPDATE(void) {
	// iterate coroutine
	ITER_CORO;
}

void DESTROY(void) {
	// remove global reference
	BLINKY = NULL;
	// deallocate coroutine context
	FREE_CORO;
}