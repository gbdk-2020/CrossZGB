#include "Banks/SetAutoBank.h"

#include "SpriteManager.h"
#include "Coroutines.h"
#include "ZGBMain.h"

// global reference to Clyde
Sprite * CLYDE;

void EnemyLogic(void * custom_data) BANKED;

void ClydeLogic(void * custom_data) BANKED {
	CLYDE = THIS;
	EnemyLogic(custom_data);
}

void ClydeLogicFinalizer(void * custom_data) BANKED {
	(void) custom_data;
	CLYDE = NULL;
}

SPRITE_COROUTINE(ClydeLogic, ClydeLogicFinalizer)
