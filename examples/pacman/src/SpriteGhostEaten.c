#include "Banks/SetAutoBank.h"

#include <stdlib.h>
#include <rand.h>

#include "SpriteManager.h"
#include "Coroutines.h"
#include "Keys.h"
#include "ZGBMain.h"

// player sprite pointer declaration, initialized in the SpritePlayer.c
extern Sprite * PLAYER;

// set the type of the enemy which this scared ghost turn into
void EatenSetType(Sprite * spr, UINT8 type) BANKED {
	if (!spr) return;
	spr->custom_data[0] = type;
}

BANKREF_EXTERN(EatenLogic)
void EatenLogic(void * custom_data) BANKED;

void START(void) {
	// make sprites not die offscreen
	THIS->lim_x = THIS->lim_y = 256;
	// allocate coroutine context
	INIT_CORO(BANK(EatenLogic), EatenLogic);
}

void UPDATE(void) {
	// iterate coroutine
	ITER_CORO;
}

void DESTROY(void) {
	// deallocate coroutine context
	FREE_CORO;
}