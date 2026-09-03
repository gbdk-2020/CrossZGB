#include "Banks/SetAutoBank.h"

#include "Coroutines.h"

#include "SpriteManager.h"
#include "ZGBMain.h"

// global reference to Blinky
Sprite * BLINKY;

void EnemyLogic(void * custom_data) BANKED;

void BlinkyLogic(void * custom_data) BANKED {
	BLINKY = THIS;
	EnemyLogic(custom_data);
}

void BlinkyLogicFinalizer(void * custom_data) BANKED {
	(void) custom_data;
	BLINKY = NULL;
}

SPRITE_COROUTINE(BlinkyLogic, BlinkyLogicFinalizer)
