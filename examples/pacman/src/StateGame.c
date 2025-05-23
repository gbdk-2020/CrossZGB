#include "Banks/SetAutoBank.h"

#include "Scroll.h"
#include "SpriteManager.h"
#include "Fade.h"
#include "Vector.h"
#include "ZGBMain.h"

#include "lee.h"
#include "bankutils.h"

IMPORT_MAP(level01);
IMPORT_MAP(level02);

#define BANKED_MAP(MAP) {BANK(MAP), &MAP}
#define LEVELS_END {0, NULL}

// dynamic map buffer
UINT8 level_buffer[LEE_MAX_HEIGHT * LEE_MAX_WIDTH];

// level list structure
typedef struct MapInfoBanked_t {
	UINT8 bank;
	struct MapInfo* map;
} MapInfoBanked_t;

// level list
const MapInfoBanked_t levels[] = {
	BANKED_MAP(level01),
	BANKED_MAP(level02),

	LEVELS_END
};

// current level structures
struct MapInfo current_level_desc;
UINT8 current_level_bank;
UINT8 current_level;
UINT16 current_level_size;
UINT16 dot_count;
position_t init, target; 
position_t blinky_pos, inky_pos, pinky_pos, clyde_pos;

// score
UINT16 game_score;

const UINT8 coll_tiles[] = LIST( 19, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18 );

// ghosts
extern Sprite * INKY;
extern Sprite * PINKY;
extern Sprite * BLINKY;
extern Sprite * CLYDE;

void copy_level_desc(UINT8 level) {
	// current level data bank
	current_level_bank = levels[level].bank;
	// copy mapinfo structure into the RAM so we can alter the data pointer
	memcpy_banked(&current_level_desc, levels[level].map, sizeof(current_level_desc), current_level_bank);
        // calculate the level map size
	current_level_size = (current_level_desc.width * current_level_desc.height);
        // copy map into the buffer in RAM
	memcpy_banked(level_buffer, current_level_desc.data, current_level_size, current_level_bank);
	// alter the data pointer of the current level mapinfo in RAM
	current_level_desc.data = level_buffer;
}

void offset_to_pos(UINT16 ofs, position_t * res) {
	res->x = ofs % current_level_desc.width;
	res->y = ofs / current_level_desc.width;
}

void locate_objects(void) {
	dot_count = 0;
	init.x = init.y = 1;
	target.x = target.y = 1;
	blinky_pos.x = inky_pos.x = pinky_pos.x = clyde_pos.x = 0xff;

	for (UINT8 * ptr = level_buffer; ptr != (level_buffer + current_level_size); ptr++) {
		switch (*ptr) {
			case 0x01:
			case 0x02:
				// dot or power pill
				dot_count++;
				continue;
			case 0xfa:
 				// blinky position
				offset_to_pos(ptr - level_buffer, &blinky_pos);
				break;
			case 0xfb:
 				// inky position
				offset_to_pos(ptr - level_buffer, &inky_pos);
				break;
			case 0xfc:
 				// pinky position
				offset_to_pos(ptr - level_buffer, &pinky_pos);
				break;
			case 0xfd:
 				// clyde position
				offset_to_pos(ptr - level_buffer, &clyde_pos);
				break;
			case 0xfe:
 				// target position
				offset_to_pos(ptr - level_buffer, &target);
				break;
			case 0xff:
				// player position
				offset_to_pos(ptr - level_buffer, &init);
				break;
			default:
				continue;
		}
		*ptr = 0;
	}
}

void load_level(void) {
	// destroy all sprites
	SpriteManagerReset();
	// initialize current level
	copy_level_desc(current_level);
	// initialize player position, pelletes count, ghost
	locate_objects();
	INKY = PINKY = BLINKY = CLYDE = NULL;
	// spawn the player sprite
	scroll_target = SpriteManagerAdd(SpritePlayer, init.x << 3, init.y << 3);
	// spawn enemies if exist
	if (inky_pos.x   != 0xff) SpriteManagerAdd(SpriteInky,   inky_pos.x   << 3, inky_pos.y   << 3);
	if (pinky_pos.x  != 0xff) SpriteManagerAdd(SpritePinky,  pinky_pos.x  << 3, pinky_pos.y  << 3);
	if (blinky_pos.x != 0xff) SpriteManagerAdd(SpriteBlinky, blinky_pos.x << 3, blinky_pos.y << 3);
	if (clyde_pos.x  != 0xff) SpriteManagerAdd(SpriteClyde,  clyde_pos.x  << 3, clyde_pos.y  << 3);
	// initialize background with collisions (skip the very first tile (19), which is only for the player)
	InitScroll(current_level_bank, &current_level_desc, coll_tiles + 1, NULL);
	// player must not walk through the ghost room door (tile 19)
	ScrollInitCollisionGroup(COLL_GROUP_1, coll_tiles);
}

void START(void) {
	// initialization
	current_level = 0;
	game_score = 0;
	// load level
	load_level();
}

void UPDATE(void) {
	if (!dot_count) {
		if (levels[++current_level].map) {
			// switch to the next level
			FadeIn();
			load_level();
			FadeOut();
		} else {
			// todo: win state
			SetState(StateMenu);
		}
	}
}

void DESTROY(void) {
}
