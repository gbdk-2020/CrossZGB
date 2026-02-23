#include "Banks/SetAutoBank.h"

#include <stdbool.h>

#include "SpriteManager.h"
#include "Coroutines.h"
#include "Keys.h"
#include "ZGBMain.h"
#include "Vector.h"

// shooting trigger
bool shoot_now;

void ShooterLogic(void * custom_data) BANKED {
	(void)custom_data;
	while (TRUE) {
		// check the shooting trigger, if true, then reset the trigger, then spawn projectile
		if (shoot_now) {
			shoot_now = false;
			// create the projectile sprite
			SpriteManagerAdd(SpriteBullet, THIS->x + 8, THIS->y + 8);
		}
		YIELD;
	}
}

SPRITE_COROUTINE(ShooterLogic, NONE)
