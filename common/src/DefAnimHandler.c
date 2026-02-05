#include <gbdk/platform.h>

#include "MetaSpriteInfo.h"
#include "Sprite.h"

// default animation change handler

static UINT8 __save;
struct metasprite_t * DefAnimHandler(Sprite * sprite, UINT8 anim_idx) {
	__save = CURRENT_BANK;
	SWITCH_ROM(sprite->mt_sprite_bank);
	struct metasprite_t * res = sprite->mt_sprite_info->metasprites[anim_idx];
	SWITCH_ROM(__save);
	return res;
}
