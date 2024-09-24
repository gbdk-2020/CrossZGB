#include "Banks/SetAutoBank.h"

#include <stdbool.h>

#include "SpriteManager.h"
#include "Keys.h"
#include "ZGBMain.h"
#include "Vector.h"

// shooting trigger
bool shoot_now;

void START(void) {
	// reset the shooting trigger on initialization
	shoot_now = false;
}

void UPDATE(void) {
	// check the shooting trigger, if true, then reset the trigger, then spawn projectile
	if (shoot_now) {
		shoot_now = false;
		// create the projectile sprite
		SpriteManagerAdd(SpriteBullet, THIS->x + 8, THIS->y + 8);
	}
}

void DESTROY(void) {
}