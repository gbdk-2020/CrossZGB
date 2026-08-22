#include <string.h>

#include "main.h"

#include "Vector.h"
#include "SpriteManager.h"
#include "Math.h"
#include "Scroll.h"
#include "ZGBMain.h"
#include "Flip.h"
#include "Palette.h"

#ifndef SPRITE_LIMIT_X
	#define SPRITE_LIMIT_X 32
#endif
#ifndef SPRITE_LIMIT_Y
	#define SPRITE_LIMIT_Y 32
#endif

//Pool
Sprite sprite_manager_sprites_mem[N_SPRITE_MANAGER_SPRITES];
Sprite* sprite_manager_sprites[N_SPRITE_MANAGER_SPRITES];
DECLARE_STACK(sprite_manager_sprites_pool, N_SPRITE_MANAGER_SPRITES);

//Current sprites
VECTOR_DECLARE(sprite_manager_updatables, N_SPRITE_MANAGER_SPRITES);

UINT8 sprite_manager_purge;

INT16 sprite_tile_allocator_top = SPRITE_TILE_ALLOC_TOP;
INT16 last_sprite_loaded;
UINT8 last_sprite_pal_loaded;

void SpriteManagerReset(void) {
	last_sprite_loaded = sprite_tile_allocator_top;
	last_sprite_pal_loaded = 0;

	UINT8 __save = CURRENT_BANK;
	//Call Destroy on all sprites still on the list
	for(UINT8 i = 0u; i != VECTOR_LEN(sprite_manager_updatables); ++ i) {
		THIS = sprite_manager_sprites[VECTOR_GET(sprite_manager_updatables, i)];
		SWITCH_ROM(spriteBanks[THIS->type]);
		spriteDestroyFuncs[THIS->type]();
	}
	SWITCH_ROM(__save);

	//place all sprites on the pool
	StackClear(sprite_manager_sprites_pool);
	for(UINT8 i = 0; i != N_SPRITE_MANAGER_SPRITES; ++i) {
		sprite_manager_sprites[i] = &sprite_manager_sprites_mem[i];
		StackPush(sprite_manager_sprites_pool, i);
	}
	ClearOAMs();

	memset(spriteIdxs,   (UINT8)SPRITE_TILE_ALLOC_BOTTOM, SPRITES_ARRAY_LEN);
	memset(spriteIdxsH,  (UINT8)SPRITE_TILE_ALLOC_BOTTOM, SPRITES_ARRAY_LEN);
	memset(spriteIdxsV,  (UINT8)SPRITE_TILE_ALLOC_BOTTOM, SPRITES_ARRAY_LEN);
	memset(spriteIdxsHV, (UINT8)SPRITE_TILE_ALLOC_BOTTOM, SPRITES_ARRAY_LEN);

	//Clear the list of updatable sprites
	VECTOR_CLEAR(sprite_manager_updatables);
	sprite_manager_purge = FALSE;
}

#if defined(NINTENDO)
void set_sprite_native_data_wrap(uint8_t first_tile, uint8_t nb_tiles, const uint8_t *data);
#endif

UINT8 SpriteManagerLoad(UINT8 sprite_type) {
	// check if already loaded
	if (((INT8)spriteIdxs[sprite_type]) >= last_sprite_loaded)
		return TRUE;

	UINT8 __save = CURRENT_BANK;
	SWITCH_ROM(spriteDataBanks[sprite_type]);

	const struct MetaSpriteInfo* data = spriteDatas[sprite_type];
	UINT8 n_tiles = data->num_tiles;

	// check if no room left
	if ((last_sprite_loaded - n_tiles) < SPRITE_TILE_ALLOC_BOTTOM) {
		SWITCH_ROM(__save);
		return FALSE;
	}

	// calculate the new last_sprite_loaded and set it for the current sprite
	last_sprite_loaded -= n_tiles;

	spriteIdxs[sprite_type] = last_sprite_loaded;
	spriteIdxsH[sprite_type] = last_sprite_loaded;
	spriteIdxsV[sprite_type] = last_sprite_loaded;
	spriteIdxsHV[sprite_type] = last_sprite_loaded;

	if (n_tiles) {
#if defined(NINTENDO)
		// load tiles wrapping around 0x9000
		set_sprite_native_data_wrap(last_sprite_loaded, n_tiles, data->data);
#elif defined(SEGA)
		// load normal tiles
		#if DEFAULT_COLOR_DEPTH == 4
		set_sprite_native_data(last_sprite_loaded, n_tiles, data->data);
		#else
		set_sprite_data(last_sprite_loaded, n_tiles, data->data);
		#endif
		// load X-flipped tiles if required and there is enough space
		if (spriteFlips[sprite_type] & FLIP_X) {
			if ((last_sprite_loaded - n_tiles) >= SPRITE_TILE_ALLOC_BOTTOM) {
				last_sprite_loaded -= n_tiles;
				spriteIdxsV[sprite_type] = last_sprite_loaded;
				set_sprite_data_flip(last_sprite_loaded, n_tiles, data->data, FLIP_X);
			}
		}
		// load Y-flipped tiles if required and there is enough space
		if (spriteFlips[sprite_type] & FLIP_Y) {
			if ((last_sprite_loaded - n_tiles) >= SPRITE_TILE_ALLOC_BOTTOM) {
				last_sprite_loaded -= n_tiles;
				spriteIdxsH[sprite_type] = last_sprite_loaded;
				set_sprite_data_flip(last_sprite_loaded, n_tiles, data->data, FLIP_Y);
			}
		}
		// load XY-flipped tiles if required and there is enough space
		if (spriteFlips[sprite_type] & FLIP_XY) {
			if ((last_sprite_loaded - n_tiles) >= SPRITE_TILE_ALLOC_BOTTOM) {
				last_sprite_loaded -= n_tiles;
				spriteIdxsHV[sprite_type] = last_sprite_loaded;
				set_sprite_data_flip(last_sprite_loaded, n_tiles, data->data, FLIP_X | FLIP_Y);
			}
		}
#endif
	}

#if defined(SEGA) || (defined(NINTENDO) && defined(CGB))
	UINT8 n_pals;
	if (n_pals = data->num_palettes) {
	#if defined(CGB)
		// search if palette combination already exist
		UINT8 search_index;
		for (search_index = 0; search_index != last_sprite_pal_loaded; ++search_index) {
			if (memcmp(ZGB_Fading_SPal + (search_index * N_PALETTE_COLORS), data->palettes, n_pals * PALETTE_SIZE) == 0)
				break;
		}

		// clamp palette offset. that produce wrong palettes in case of overflow, but not make sprite disappear on CGB
		spritePalsOffset[sprite_type] = ((search_index > (MAX_PALETTES - n_pals)) ? (MAX_PALETTES - n_pals) : search_index);

		// load palettes when it is required
		if (search_index == last_sprite_pal_loaded) {
			last_sprite_pal_loaded += SetPalette(SPRITES_PALETTE, last_sprite_pal_loaded, n_pals, data->palettes, CURRENT_BANK);
		}
	#else
		spritePalsOffset[sprite_type] = 0;
		// load sprite palette if it was not previously loaded, also there is only one sprite palette on the SMS/GG
		if (!last_sprite_pal_loaded) {
			last_sprite_pal_loaded += SetPalette(SPRITES_PALETTE, 0, n_pals, data->palettes, CURRENT_BANK);
		}
	#endif
	}
#endif

	SWITCH_ROM(__save);

	return TRUE;
}

Sprite* SpriteManagerAddEx(UINT8 sprite_type, UINT16 x, UINT16 y, void* data) {
	static Sprite *sprite, *cached_sprite;
	static UINT8 sprite_idx, cached_sprite_idx;
	static UINT8 __save;

	if (VECTOR_LEN(sprite_manager_updatables) > (N_SPRITE_MANAGER_SPRITES - 1)) return NULL;

	if (!SpriteManagerLoad(sprite_type)) return NULL;

	sprite_idx = StackPop(sprite_manager_sprites_pool);
	sprite = sprite_manager_sprites[sprite_idx];
	sprite->type = sprite_type;
	sprite->marked_for_removal = FALSE;
	sprite->lim_x = SPRITE_LIMIT_X;
	sprite->lim_y = SPRITE_LIMIT_Y;
	sprite->mirror = NO_MIRROR;

	VectorAdd(sprite_manager_updatables, sprite_idx);

	InitSprite(sprite, sprite_type);
	sprite->x = x;
	sprite->y = y;
	sprite->unique_id = (UINT16)data;

	//Before calling start THIS and THIS_IDX must be set, preserve the old values
	cached_sprite = THIS;
	cached_sprite_idx = THIS_IDX;
	THIS = sprite;
	THIS_IDX = VECTOR_LEN(sprite_manager_updatables) - 1;

	__save = CURRENT_BANK;
	SWITCH_ROM(spriteBanks[sprite->type]);
	spriteStartFuncs[sprite->type](data);
	SWITCH_ROM(__save);

	//And now they must be restored
	THIS = cached_sprite;
	THIS_IDX = cached_sprite_idx;
	return sprite;
}

void SpriteManagerRemove(UINT8 idx) {
	sprite_manager_sprites[VECTOR_GET(sprite_manager_updatables, idx)]->marked_for_removal = TRUE;
	sprite_manager_purge = TRUE;
}

void SpriteManagerRemoveSprite(Sprite* sprite) {
	sprite->marked_for_removal = TRUE; 
	sprite_manager_purge = TRUE;
}

UINT8 enable_flickering = TRUE;
UINT8 THIS_IDX;
Sprite* THIS = NULL;

void SpriteManagerUpdate(void) {
	static UINT8 __save;
	// preserve current bank
	__save = CURRENT_BANK;
	// iterate updatables, call the update function of the each sprite and render it
	SPRITEMANAGER_ITERATE(THIS_IDX, THIS)  {
		// if marked for removal then skip it
		if (THIS->marked_for_removal) continue;
		// switch rom bank of the sprite
		SWITCH_ROM(spriteBanks[THIS->type]);
		// call the sprite update function
		spriteUpdateFuncs[THIS->type]();
		// if sprite is a scroll target then update the scroll position
		if (THIS == scroll_target) {
			RefreshScroll();
			if ((THIS_IDX) && (enable_flickering)) {
				BufferExchange(sprite_manager_updatables + 1, THIS_IDX);
			}
		}
	}
	// restore bank
	SWITCH_ROM(__save);
	// if flickering enabled cycle the updateables array so sprites are drawn roundrobin, except the scroll target
	if (enable_flickering) {
		if (scroll_target) {
			BufferRotate(sprite_manager_updatables + 2, VECTOR_LEN(sprite_manager_updatables) - 1);
		} else {
			BufferRotate(sprite_manager_updatables + 1, VECTOR_LEN(sprite_manager_updatables));
		} 
	}
}

void SpriteManagerRenderOnly(void) {
	static UINT8 __save;
	__save = CURRENT_BANK;
	// iterate updatable sprites and render
	SPRITEMANAGER_ITERATE(THIS_IDX, THIS) {
		// switch ROM bank and call the sprite destroy handler
		SWITCH_ROM(spriteBanks[THIS->type]);
		// render sprite into the OAM
		DrawSprite();
	}
	SWITCH_ROM(__save);
}

void SpriteManagerRenderPurge(void) {
	static UINT8 __save, current, sprite_idx;
	__save = CURRENT_BANK;
	// iterate updatable sprites and render or remove marked
	current = 0;
	SPRITEMANAGER_ITERATE(THIS_IDX, THIS) {
		// switch ROM bank and call the sprite destroy handler
		SWITCH_ROM(spriteBanks[THIS->type]);
		// get global sprite index
		sprite_idx = VECTOR_GET(sprite_manager_updatables, THIS_IDX);
		// remove sprite or render to screen
		if (THIS->marked_for_removal) {
			spriteDestroyFuncs[THIS->type]();
			// return sprite to sprite pool
			StackPush(sprite_manager_sprites_pool, sprite_idx);
		} else {
			// render sprite into the OAM
			DrawSprite();
			// shrink the vector
			VECTOR_SET_DIRECT(sprite_manager_updatables, current++, sprite_idx);
		}

	}
	// update the updateables vector length
	VECTOR_LEN(sprite_manager_updatables) = current;
	SWITCH_ROM(__save);
}
