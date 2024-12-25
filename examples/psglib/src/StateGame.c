#include "Banks/SetAutoBank.h"

#include "ZGBMain.h"
#include "Scroll.h"
#include "SpriteManager.h"
#include "Music.h"

#define PLAYER_WIDTH  16
#define PLAYER_HEIGHT 16

DECLARE_MUSIC(ingamemusic);

void START(void) {
	// spawn the player sprite at the center of the screen
	SpriteManagerAdd(SpritePlayer, ((SCREEN_WIDTH - PLAYER_WIDTH) / 2), ((SCREEN_HEIGHT - PLAYER_HEIGHT)/ 2));
	PlayMusic(ingamemusic, 1);
}

void UPDATE(void) {
}

void DESTROY(void) {
}
