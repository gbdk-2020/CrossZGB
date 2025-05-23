#include "Banks/SetAutoBank.h"

#include "SpriteManager.h"
#include "Coroutines.h"
#include "ZGBMain.h"

// global reference to Inky
Sprite * INKY;

// declare the logic coroutine
BANKREF_EXTERN(InkyLogic)
void InkyLogic(void * custom_data) BANKED;

void START(void) {
	// initialize global reference
	INKY = THIS;
	// make sprites not die offscreen
	THIS->lim_x = THIS->lim_y = 256;
	// allocate coroutine context, set coroutine function and pass CUSTOM_DATA as data, remove sprite if failed
	INIT_CORO(BANK(InkyLogic), InkyLogic);
}

void UPDATE(void) {
	// iterate coroutine
	ITER_CORO;
}

void DESTROY(void) {
	// remove global reference
	INKY = NULL;
	// deallocate coroutine context
	FREE_CORO;
}