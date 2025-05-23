#include "Banks/SetAutoBank.h"

#include "SpriteManager.h"
#include "Coroutines.h"
#include "Keys.h"
#include "ZGBMain.h"
#include "Scroll.h"
#include "Vector.h"

// player sprite pointer
Sprite * PLAYER;

// ghosts
extern Sprite * INKY;
extern Sprite * PINKY;
extern Sprite * BLINKY;
extern Sprite * CLYDE;

// level structures
extern UINT8 level_buffer[];
extern struct MapInfo current_level_desc;
extern UINT16 dot_count;
extern position_t init;
extern position_t blinky_pos, inky_pos, pinky_pos, clyde_pos;

// movement directions
direction_e player_dir, player_next_dir;
UINT8 player_check_collision;

static const INT8 speed_x[N_DIRECTIONS] = {  0,  0,  0, -1,  1 };
static const INT8 speed_y[N_DIRECTIONS] = {  0, -1,  1,  0,  0 };

// animation related stuff
#define ANIMATION_SPEED 24

static const UINT8 anim_none[]   = VECTOR( 9 );
static const UINT8 anim_up[]     = VECTOR( 0,  1,  2,  1 );
static const UINT8 anim_down[]   = VECTOR( 6,  7,  8,  7 );
static const UINT8 anim_left[]   = VECTOR( 9, 10, 11, 10 );
static const UINT8 anim_right[]  = VECTOR( 3,  4,  5,  4 );
static const UINT8 anim_eaten[]  = VECTOR( 0,  1,  2, 12, 13, 14, 15, 16, 17 );
static const UINT8 const * animations[N_DIRECTIONS] = { NULL, anim_up,  anim_down,  anim_left,  anim_right };

// power pill activity
#define PILL_TIMEOUT 10

UINT8 pill_active, pill_second;

// score
extern UINT16 game_score;

void ScaredSetType(Sprite * spr, UINT8 type) BANKED;

void PlayerLogic(void * custom_data) BANKED {
	custom_data;

	player_check_collision = TRUE;
	player_dir = player_next_dir = DIR_NONE;
	SetSpriteAnim(THIS, anim_none, ANIMATION_SPEED);
	// assign player collision groups to COLL_GROUP_1
	SetSpriteCollisionGroup(THIS, COLL_GROUP_1, COLL_GROUP_1);
	while (TRUE) {
		// slow down pacman movement
		if (IS_EVEN_FRAME) {
			// pill timeout
			if (pill_active) {
				if (!pill_second--) {
					pill_second = 30, pill_active--;
				}
			}

			// check D-Pad buttons and queue the direction
			if (KEY_PRESSED(J_LEFT)) {
				player_next_dir = DIR_LEFT;
			} else if (KEY_PRESSED(J_RIGHT)) {
				player_next_dir = DIR_RIGHT;
			}
			if (KEY_PRESSED(J_UP)) {
				player_next_dir = DIR_UP;
			} else if (KEY_PRESSED(J_DOWN)) {
				player_next_dir = DIR_DOWN;
			}

			// check collision with ghosts
			if ((CheckCollision(PLAYER, INKY)) || (CheckCollision(PLAYER, PINKY)) || (CheckCollision(PLAYER, BLINKY)) || (CheckCollision(PLAYER, CLYDE))) {
				// player got eaten by ghosts
				SetSpriteAnim(THIS, anim_eaten, ANIMATION_SPEED); 
				// play animation once
				while (THIS->anim_frame != 8) {
					YIELD;
				}
				// hide player
				THIS->visible = FALSE;
				// scroll level to initial position
				UINT16 tmp;
				tmp = init.x << 3;
				while (THIS->x != tmp) {
					if (THIS->x < tmp) THIS->x++; else THIS->x--;
					YIELD;
				}
				tmp = init.y << 3;
				while (THIS->y != tmp) {
					if (THIS->y < tmp) THIS->y++; else THIS->y--;
					YIELD;
				}
				// teleport enemies
				if (INKY)   INKY->x   = inky_pos.x << 3,   INKY->y   = inky_pos.y << 3;
				if (PINKY)  PINKY->x  = pinky_pos.x << 3,  PINKY->y  = pinky_pos.y << 3;
				if (BLINKY) BLINKY->x = blinky_pos.x << 3, BLINKY->y = blinky_pos.y << 3;
				if (CLYDE)  CLYDE->x  = clyde_pos.x << 3,  CLYDE->y  = clyde_pos.y << 3;
				// show player
				THIS->visible = TRUE;
				player_dir = player_next_dir = DIR_NONE;
				SetSpriteAnim(THIS, anim_none, ANIMATION_SPEED);
			}

			// movement and wall collision handling
			if ((player_next_dir) && (TranslateSprite(THIS, speed_x[player_next_dir], speed_y[player_next_dir]) == 0)) {
				player_dir = player_next_dir;
				if (animations[player_dir]) SetSpriteAnim(THIS, animations[player_dir], ANIMATION_SPEED);
				player_next_dir = DIR_NONE;
			} else {
				if (TranslateSprite(THIS, speed_x[player_dir], speed_y[player_dir]) != 0) {
					player_dir = DIR_NONE;
					// yield execution
					YIELD;
					// early exit
					continue;					
				}
			}

			// wrap player
			if ((INT16)THIS->x <= 0) {
				THIS->visible = player_check_collision = FALSE;
				for (UINT8 i = current_level_desc.width - 1; (i); i--, THIS->x += 8) {
					YIELD;
				}
				THIS->x = ((current_level_desc.width - 1) << 3);
				THIS->visible = player_check_collision = TRUE;
			} else if (THIS-> x >= ((current_level_desc.width - 1) << 3)) {
				THIS->visible = player_check_collision = FALSE;
				for (UINT8 i = current_level_desc.width - 1; (i); i--, THIS->x -= 8) {
					YIELD;
				}
				THIS->x = 0;
				THIS->visible = player_check_collision = TRUE;
			}

			// dot eating logic
			static UINT8 pos_x, pos_y;
			// calculate tile offset in the level buffer
			UINT8 * ofs = level_buffer + (((pos_y = (THIS->y + 4) >> 3) * current_level_desc.width) + (pos_x = (THIS->x + 4) >> 3));
			switch (*ofs) {
				case 0x02:
					// power pill scares each enemy if they are in the "normal" state
					if (INKY) {
						ScaredSetType(SpriteManagerAdd(SpriteGhostScared, INKY->x,   INKY->y),   SpriteInky); 
						SpriteManagerRemoveSprite(INKY);
					}
					if (PINKY) {
						ScaredSetType(SpriteManagerAdd(SpriteGhostScared, PINKY->x,  PINKY->y),  SpritePinky); 
						SpriteManagerRemoveSprite(PINKY);
					}
					if (BLINKY) {
						ScaredSetType(SpriteManagerAdd(SpriteGhostScared, BLINKY->x, BLINKY->y), SpriteBlinky); 
						SpriteManagerRemoveSprite(BLINKY);
					}
					if (CLYDE) {
						ScaredSetType(SpriteManagerAdd(SpriteGhostScared, CLYDE->x,  CLYDE->y),  SpriteClyde); 
						SpriteManagerRemoveSprite(CLYDE);
					}
					// activate power pill for PILL_TIMEOUT seconds
					pill_active = PILL_TIMEOUT, pill_second = 30;
					// fall through to dot collection
				case 0x01:
					// dot
					* ofs = 0;
					UpdateMapTile(TARGET_BKG, pos_x + scroll_offset_x, pos_y + scroll_offset_y, 0, 0x00, NULL);
					// increase score
					game_score++;
					dot_count--;
					break;			
			}
		}
		// yield execution
		YIELD;
	} 
}


void START(void) {
	// allocate coroutine context, set coroutine function and pass CUSTOM_DATA as data, remove sprite if failed
	INIT_CORO(BANK(SpritePlayer), PlayerLogic);
	// initialize global reference
	PLAYER = THIS;
}

void UPDATE(void) {
	// iterate coroutine
	ITER_CORO;
}

void DESTROY(void) {
	// remove global reference
	PLAYER = NULL;
	// deallocate coroutine context
	FREE_CORO;
}