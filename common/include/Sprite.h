#ifndef SPRITE_H
#define SPRITE_H

#include <gbdk/platform.h>

#include "vector.h"
#include "OAMManager.h"
#include "MetaSpriteInfo.h"
#include "Flip.h"

typedef struct {
	// Position
	UINT16 x, y;

	// Collision box
	UINT8 coll_w, coll_h;

	// Meta sprite info
	UINT8 mt_sprite_bank;
	const struct MetaSpriteInfo* mt_sprite_info;

	// Frame info
	UINT8 flips;                    // supported flips/mirror modes
	UINT8 first_tile;               // tile offset, for animation indices
	UINT8 first_tile_H;             // tile H mirrored offset, for the animation indices
	UINT8 first_tile_V;             // tile V mirrored offset, for the animation indices
	UINT8 first_tile_HV;            // tile HV mirrored offset, for the animation indices
	UINT8 attr_add;                 // metasprite attributes, used to set the DMG palette (bit 4) or the priority over Bg (bit 7) 
	                                // engine internally sets the palette

	// Animation data
	UINT8* anim_data;
	UINT8 anim_accum_ticks;
	UINT8 anim_speed;
	UINT8 anim_frame;
	struct metasprite_t* mt_sprite;

	// Flags, currently used for mirroring
	MirroMode mirror;

	// For the sprite manager
	INT16 lim_x, lim_y;             // limits offscren where the sprite will be deleted (0 means immediatelly)
	UINT8 type;                     // sprite type (enemy, bullet, etc.)
	UINT8 marked_for_removal;
	UINT16 unique_id;               // unique ID, based on the tile, sprite spawned at

	UINT8 visible;                  // visibility (sprite is not rendered if zero)

	UINT8 custom_data[CUSTOM_DATA_SIZE];
} Sprite;

// Palette flag
#if defined(NINTENDO)
#define SPRITE_SET_CGB_PALETTE(SPRITE, PALETTE) SPRITE->attr_add = ((SPRITE->attr_add & 0xF8u) | ((PALETTE) & 0x07u))
#define SPRITE_SET_DMG_PALETTE(SPRITE, PALETTE) SPRITE->attr_add = ((SPRITE->attr_add & 0xEFu) | (((PALETTE) & 0x01u) << 4))
#ifdef CGB
#define SPRITE_SET_PALETTE(SPRITE, PALETTE) if(_cpu == CGB_TYPE) SPRITE_SET_CGB_PALETTE(SPRITE, PALETTE); else SPRITE_SET_DMG_PALETTE(SPRITE, PALETTE)
#define SPRITE_SET_DEFAULT_PALETTE(SPRITE)  if(_cpu == CGB_TYPE) SPRITE->attr_add = SPRITE->attr_add & 0xEFu
#else
#define SPRITE_SET_PALETTE(SPRITE, PALETTE) SPRITE_SET_DMG_PALETTE(SPRITE, PALETTE)
#endif
#elif defined(SEGA)
#define SPRITE_SET_PALETTE(SPRITE, PALETTE)
#define SPRITE_SET_DEFAULT_PALETTE(SPRITE)
#endif

void SetFrame(Sprite* sprite, UINT8 frame);
void InitSprite(Sprite* sprite, UINT8 sprite_type);
void SetSpriteAnim(Sprite* sprite, const UINT8* data, UINT8 speed);

inline void SetSpriteAnimFrame(Sprite* sprite, UINT8 frame) {
	if (sprite->anim_data) {
		SetFrame(sprite, VECTOR_GET(sprite->anim_data, frame));
		sprite->anim_frame = frame;
		sprite->anim_accum_ticks = 0;
	}
}
inline void SetVisible(Sprite* sprite, UINT8 visible) {
	sprite->visible = visible;
}

void DrawSprite(void);

UINT8 TranslateSprite(Sprite* sprite, INT8 x, INT8 y);

UINT8 CheckCollision(Sprite* sprite1, Sprite* sprite2);

#endif