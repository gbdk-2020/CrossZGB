#include "Banks/SetAutoBank.h"

#include <stdbool.h>

#include "SpriteManager.h"
#include "Coroutines.h"
#include "Keys.h"
#include "ZGBMain.h"

// shooting trigger declaration, initialized in the SpriteShooter.c
extern bool shoot_now;

// player sprite pointer
Sprite * PLAYER;

void PlayerLogic(void * custom_data) BANKED {
	(void)custom_data;
	// initialize the player sprite pointer
	PLAYER = THIS;
	while (TRUE) {
		// check D-Pad buttons and move the player within the screen boundaries
		if (KEY_PRESSED(J_LEFT)) {
			if (THIS->x) THIS->x--;
		} else if (KEY_PRESSED(J_RIGHT)) {
			if (THIS->x < (SCREEN_WIDTH - 16)) THIS->x++;
		}
		if (KEY_PRESSED(J_UP)) {
			if (THIS->y) THIS->y--;
		} else if (KEY_PRESSED(J_DOWN)) {
			if (THIS->y < (SCREEN_HEIGHT - 16)) THIS->y++;
		}
		// if A button pressed, then signal shooting the projectile
		if (KEY_TICKED(J_A)) {
			shoot_now = true;
		}
		YIELD;
	}
}

void PlayerLogicFinalizer(void * custom_data) BANKED {
	(void)custom_data;
	PLAYER = NULL;
}

SPRITE_COROUTINE(PlayerLogic, PlayerLogicFinalizer)
