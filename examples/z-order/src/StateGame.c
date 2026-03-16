#include "Banks/SetAutoBank.h"

#include "Scroll.h"
#include "SpriteManager.h"

#include "ZGBMain.h"

#define PLAYER_WIDTH  16
#define PLAYER_HEIGHT 16

// actor sorting function
void sort_actor_zorder(void);

void START(void) {
        // disable sprite flickering
	enable_flickering = FALSE;
	// spawn the player sprite at the center of the screen
	Sprite * player = SpriteManagerAdd(SpritePlayer, ((SCREEN_WIDTH - PLAYER_WIDTH) / 2), ((SCREEN_HEIGHT - PLAYER_HEIGHT)/ 2));
	// add NPC's
	SpriteManagerAdd(SpriteNPC, player->x - 32, player->y - 32);
	SpriteManagerAdd(SpriteNPC, player->x - 32, player->y + 48);
	SpriteManagerAdd(SpriteNPC, player->x + 32, player->y + 32);
	SpriteManagerAdd(SpriteNPC, player->x + 32, player->y - 48);
}

void UPDATE(void) {
	// sort actor Z-order (may be called if any of the actor's Y coordinate actually changed, not every frame)
	sort_actor_zorder();
}

void DESTROY(void) {
}