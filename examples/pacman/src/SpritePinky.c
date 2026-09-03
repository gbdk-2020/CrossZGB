#include "Banks/SetAutoBank.h"

#include "SpriteManager.h"
#include "Coroutines.h"
#include "ZGBMain.h"

// global reference to Pinky
Sprite * PINKY;

void EnemyLogic(void * custom_data) BANKED;

void PinkyLogic(void * custom_data) BANKED {
	PINKY = THIS;
	EnemyLogic(custom_data);
}

void PinkyLogicFinalizer(void * custom_data) BANKED {
	(void) custom_data;
	PINKY = NULL;
}

SPRITE_COROUTINE(PinkyLogic, PinkyLogicFinalizer)
