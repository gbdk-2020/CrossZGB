#include "Banks/SetAutoBank.h"

#include "SpriteManager.h"
#include "Coroutines.h"
#include "ZGBMain.h"

// global reference to Inky
Sprite * INKY;

void EnemyLogic(void * custom_data) BANKED;

void InkyLogic(void * custom_data) BANKED {
	INKY = THIS;
	EnemyLogic(custom_data);
}

void InkyLogicFinalizer(void * custom_data) BANKED {
	(void) custom_data;
	INKY = NULL;
}

SPRITE_COROUTINE(InkyLogic, InkyLogicFinalizer)
