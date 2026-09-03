#include "Banks/SetAutoBank.h"

#include <stdlib.h>
#include <rand.h>

#include "SpriteManager.h"
#include "Coroutines.h"
#include "Keys.h"
#include "ZGBMain.h"

// set the type of the enemy which this scared ghost turn into
void EatenSetType(Sprite * spr, UINT8 type) BANKED {
	if (!spr) return;
	spr->custom_data[0] = type;
}

void EatenLogic(void * custom_data) BANKED;

SPRITE_COROUTINE(EatenLogic, NONE)
