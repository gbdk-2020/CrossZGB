#include "Banks/SetAutoBank.h"

#include <stdbool.h>

#include "ZGBMain.h"
#include "Scroll.h"
#include "SpriteManager.h"
#include "Music.h"
#include "Keys.h"

#define PLAYER_WIDTH  16
#define PLAYER_HEIGHT 16

DECLARE_MUSIC(ingamemusic);

bool music_playing = true;

void START(void) {
	// spawn the player sprite at the center of the screen
	SpriteManagerAdd(SpritePlayer, ((SCREEN_WIDTH - PLAYER_WIDTH) / 2), ((SCREEN_HEIGHT - PLAYER_HEIGHT)/ 2));
	PlayMusic(ingamemusic, 1);
}

void UPDATE(void) {
	if (KEY_TICKED(J_START)) {
		if (music_playing = !music_playing) PlayMusic(ingamemusic, 1); else StopMusic;
	}
}

void DESTROY(void) {
}
