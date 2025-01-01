#include "Banks/SetAutoBank.h"

#include "SpriteManager.h"
#include "Coroutines.h"
#include "ZGBMain.h"

// global reference to Inky
Sprite * PINKY;

// declare the logic coroutine
BANKREF_EXTERN(PinkyLogic)
void PinkyLogic(void * custom_data) BANKED;

void START(void) {
	// make sprites not die offscreen
	THIS->lim_x = THIS->lim_y = 256;
	// allocate coroutine context, set coroutine function and pass CUSTOM_DATA as data, remove sprite if failed
	INIT_CORO(BANK(PinkyLogic), PinkyLogic);
	// initialize global reference
	PINKY = THIS;
}

void UPDATE(void) {
	// iterate coroutine
	ITER_CORO;
}

void DESTROY(void) {
	// remove global reference
	PINKY = NULL;
	// deallocate coroutine context
	FREE_CORO;
}