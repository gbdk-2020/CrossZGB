#ifndef SPRITE_MANAGER_H
#define SPRITE_MANAGER_H

#include <gbdk/platform.h>

#include "Vector.h"
#include "Stack.h"
#include "Sprite.h"

#if defined(NINTENDO)
	#define SPRITE_TILE_ALLOC_TOP 128
	#define SPRITE_TILE_ALLOC_BOTTOM -128
#elif defined(SEGA)
	#define SPRITE_TILE_ALLOC_TOP 256
	#define SPRITE_TILE_ALLOC_FULL SPRITE_TILE_ALLOC_TOP
	#define SPRITE_TILE_ALLOC_PART 192
	#define SPRITE_TILE_ALLOC_BOTTOM 0
#endif

#define N_SPRITE_MANAGER_SPRITES 20

#define SPRITEMANAGER_ITERATE(i, spr) for(i = 0u, spr = sprite_manager_sprites[VECTOR_GET(sprite_manager_updatables, 0)]; i != VECTOR_LEN(sprite_manager_updatables); spr = sprite_manager_sprites[VECTOR_GET(sprite_manager_updatables, ++i)])

#define ENABLE_SPRITE_FLICKERING (enable_flickering=TRUE)
#define DISABLE_SPRITE_FLICKERING (enable_flickering=FALSE)

//Pool
extern Sprite* sprite_manager_sprites[];
extern UINT8* sprite_manager_sprites_pool;

//Current sprites
extern UINT8 sprite_manager_updatables[];

extern UINT8 enable_flickering;

extern UINT8 THIS_IDX;
extern Sprite* THIS;

void SpriteManagerReset(void);

UINT8 SpriteManagerLoad(UINT8 sprite_type);

Sprite* SpriteManagerAddEx(UINT8 sprite_type, UINT16 x, UINT16 y, void* data);
inline Sprite* SpriteManagerAdd(UINT8 sprite_type, UINT16 x, UINT16 y) {
	return SpriteManagerAddEx(sprite_type, x, y, NULL);
}
void SpriteManagerRemove(UINT8 idx);
void SpriteManagerRemoveSprite(Sprite* sprite);

void SpriteManagerFlushRemove(void);
void SpriteManagerUpdate(void);

inline void SpriteManagerBringToFront(Sprite* sprite) {
	if (!sprite) return;
	for (UINT8 i = 0; i != VECTOR_LEN(sprite_manager_updatables); ++i) {
		if (sprite_manager_sprites[VECTOR_GET(sprite_manager_updatables, i)] == sprite) {
			VectorExchange(sprite_manager_updatables, 0, i);
			return;
		}
	}
}

#endif