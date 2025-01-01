#include "Banks/SetAutoBank.h"

#include <rand.h>

#include "SpriteManager.h"
#include "Coroutines.h"
#include "ZGBMain.h"

#include "lee.h"

// player sprite pointer declaration, initialized in the SpritePlayer.c
extern Sprite * PLAYER;
extern direction_e player_dir;
extern UINT8 player_check_collision;

// game level structures
extern UINT8 level_buffer[];
extern struct MapInfo current_level_desc;
extern position_t target; 

// speeds by the each direction
static const INT8 speed_x[N_DIRECTIONS] = {  0,  0,  0, -1,  1 };
static const INT8 speed_y[N_DIRECTIONS] = {  0, -1,  1,  0,  0 };

// animation related stuff
#define ANIMATION_SPEED 16

// animation arrays must be in RAM here, because they are defined 
// outside the sprite file (thus in the different bank)
static UINT8 anim_up[]     = VECTOR( 0,  1);
static UINT8 anim_down[]   = VECTOR( 4,  5);
static UINT8 anim_left[]   = VECTOR( 6,  7);
static UINT8 anim_right[]  = VECTOR( 2,  3);

static const UINT8 const * animations[N_DIRECTIONS] = { NULL, anim_up,  anim_down,  anim_left,  anim_right };

// lee algo structures and functions
UINT16 paths_buf0[LEE_MAX_STEPS];
UINT16 paths_buf1[LEE_MAX_STEPS];
UINT16 paths_buf2[LEE_MAX_STEPS];
UINT16 paths_buf3[LEE_MAX_STEPS];
UINT16 * const * path_buffers[N_SPRITE_TYPES] = {
	[SpriteInky]   = paths_buf0,
	[SpritePinky]  = paths_buf1,
	[SpriteBlinky] = paths_buf2,
	[SpriteClyde]  = paths_buf3
};

UINT16 * allocate_path(UINT8 type) {
	return (type < N_SPRITE_TYPES) ? path_buffers[type] : NULL;
}

inline UBYTE playfield_get(UBYTE x, UBYTE y) {
    return level_buffer[(y * current_level_desc.width) + x];
}

UINT8 lee_test_collision(UINT8 x, UINT8 y) {
	if (x > (current_level_desc.width - 1)) return FALSE;
	if (y > (current_level_desc.height - 1)) return FALSE;
	UINT8 tile = playfield_get(x, y);
	return ((tile < 3) || (tile == 19));
}

// since we are calling the sprite coroutine from within the UPDATE handler only, THIS and THIS_IDX are valid in the coroutines

// wrap enemy on the level edges
inline void WrapEnemy(void) {
	if ((INT16)THIS->x <= 0) {
		THIS->x = (current_level_desc.width - 1) << 3; 
	} else if (THIS->x >= ((current_level_desc.width - 1) << 3)) {
		THIS->x = 0;
	}
}

// Inky logic coroutine
BANKREF(InkyLogic)
void InkyLogic(void * custom_data) BANKED {
	custom_data;

	direction_e current_dir, next_dir;

	next_dir = DIR_UP, current_dir = DIR_RIGHT;

        SetSpriteAnim(THIS, animations[current_dir], ANIMATION_SPEED);
	while (TRUE) {
		// move every other frame
		if (IS_EVEN_FRAME) {
			// movement and collision handling
			if ((next_dir) && (TranslateSprite(THIS, speed_x[next_dir], speed_y[next_dir]) == 0)) {
				current_dir = next_dir;
				if (animations[current_dir]) SetSpriteAnim(THIS, animations[current_dir], ANIMATION_SPEED);
				next_dir = DIR_NONE;
			} else {
				if (TranslateSprite(THIS, speed_x[current_dir], speed_y[current_dir]) != 0) {
					current_dir = (rand() & 3) + 1;
					next_dir = (rand() & 3) + 1;
				}
			}
			// wrap enemies
			WrapEnemy();
		}
		// yield execution
		YIELD;
	}
}

// Pinky logic coroutine
BANKREF(PinkyLogic)
void PinkyLogic(void * custom_data) BANKED {
	InkyLogic(custom_data);
}

// Blinky logic coroutine
BANKREF(BlinkyLogic)
void BlinkyLogic(void * custom_data) BANKED {
	InkyLogic(custom_data);
}

// Clyde logic coroutine
BANKREF(ClydeLogic)
void ClydeLogic(void * custom_data) BANKED {
	InkyLogic(custom_data);
}

// pill active
extern UINT8 pill_active;

// animation arrays must be in RAM here, because they are defined 
// outside the sprite file (thus in the different bank)
static UINT8 anim_scared[] = VECTOR(0, 1);
static UINT8 anim_scared_warn[] = VECTOR(0, 1, 2, 3);

#define SCARED_SPEED 3
#define INIT_SKIP_FRAMES(count) UINT8 frame_counter=(count)
#define SKIP_FRAMES(count) if (frame_counter--) {YIELD; continue;} frame_counter=(count)			

// declare the eaten set type function
void EatenSetType(Sprite * spr, UINT8 type) BANKED;

// Scared sprite logic coroutine
BANKREF(ScaredLogic)
void ScaredLogic(void * custom_data) BANKED {
	custom_data;

	direction_e current_dir, next_dir;

	INIT_SKIP_FRAMES(SCARED_SPEED);

	// select the initial direction
	if (PLAYER->x < THIS->x) current_dir = DIR_RIGHT; else current_dir = DIR_LEFT;
	if (PLAYER->y < THIS->y) next_dir = DIR_DOWN; else next_dir = DIR_UP;

	// loop while pill is active
	while (pill_active) {
		// move every third frame
		SKIP_FRAMES(SCARED_SPEED);
		// set different animations
		if (pill_active > 2) {
			SetSpriteAnim(THIS, anim_scared, ANIMATION_SPEED); 
		} else {
			SetSpriteAnim(THIS, anim_scared_warn, ANIMATION_SPEED);
		}
		// movement and collision handling
		if ((next_dir) && (TranslateSprite(THIS, speed_x[next_dir], speed_y[next_dir]) == 0)) {
			current_dir = next_dir;
			next_dir = DIR_NONE;
		} else {
			if (TranslateSprite(THIS, speed_x[current_dir], speed_y[current_dir]) != 0) {
				current_dir = (rand() & 3) + 1;
				next_dir = (rand() & 3) + 1;
				
			}
		}
		// wrap enemies
		WrapEnemy();
		// check collision
		if ((player_check_collision) && (CheckCollision(THIS, PLAYER))) {
			// death
			EatenSetType(SpriteManagerAdd(SpriteGhostEaten, THIS->x, THIS->y), THIS->custom_data[0]); 
			SpriteManagerRemoveSprite(THIS); 
		}
		// yield execution
		YIELD;
	}
	// spawn previous sprite
	SpriteManagerAdd(THIS->custom_data[0], THIS->x, THIS->y);
}


// animation arrays must be in RAM here, because they are defined 
// outside the sprite file (thus in the different bank)
static UINT8 anim_eaten_up[]    = VECTOR( 0 );
static UINT8 anim_eaten_down[]  = VECTOR( 2 );
static UINT8 anim_eaten_left[]  = VECTOR( 3 );
static UINT8 anim_eaten_right[] = VECTOR( 1 );

BANKREF(EatenLogic)
void EatenLogic(void * custom_data) BANKED {
	custom_data;
	UINT8 src_x = (THIS->x + 4) >> 3, src_y = (THIS->y + 4) >> 3; 
	UINT8 path_length = lee_find_path(src_x, src_y, target.x, target.y);
	if (path_length != LEE_MAX_STEPS) {
	        // allocate path buffer
		UINT16 * path = allocate_path(THIS->custom_data[0]);
		if (!path) return;
		// restore path into the buffer
		lee_restore_path(target.x, target.y, path);
		// walk along the path
		for (; (path_length); --path_length, ++path) {
			UINT16 tmp;
			tmp = lee_get_coords_x(*path) << 3;
			while (THIS->x != tmp) {
				if (THIS->x < tmp) {
					THIS->x++;
					SetSpriteAnim(THIS, anim_eaten_right, ANIMATION_SPEED);
				} else {
					THIS->x--;
					SetSpriteAnim(THIS, anim_eaten_left, ANIMATION_SPEED);
				}
				// yield execution
				YIELD;
			}
			tmp = lee_get_coords_y(*path) << 3;
			while (THIS->y != tmp) {
				if (THIS->y < tmp) {
					THIS->y++; 
					SetSpriteAnim(THIS, anim_eaten_down, ANIMATION_SPEED);
				} else {
					THIS->y--;
					SetSpriteAnim(THIS, anim_eaten_up, ANIMATION_SPEED);
				}
				// yield execution
				YIELD;
			}

		}
	}
	// spawn previous sprite
	SpriteManagerAdd(THIS->custom_data[0], THIS->x, THIS->y);
}
