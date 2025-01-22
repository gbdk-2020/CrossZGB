#ifndef SPRITE_MANAGER_H
#define SPRITE_MANAGER_H

#include <gbdk/platform.h>

#include "Vector.h"
#include "Stack.h"
#include "Sprite.h"

#define N_SPRITE_MANAGER_SPRITES 20

#define SPRITEMANAGER_ITERATE(i, spr) for(i = 0u, spr = sprite_manager_sprites[VECTOR_GET(sprite_manager_updatables, 0)]; i != VECTOR_LEN(sprite_manager_updatables); spr = sprite_manager_sprites[VECTOR_GET(sprite_manager_updatables, ++i)])

//Pool
extern Sprite* sprite_manager_sprites[];
extern UINT8* sprite_manager_sprites_pool;

//Current sprites
extern UINT8 sprite_manager_updatables[];

extern UINT8 enable_flickering;

extern UINT8 THIS_IDX;
extern Sprite* THIS;

void SpriteManagerReset(void);

void SpriteManagerLoad(UINT8 sprite_type);

Sprite* SpriteManagerAdd(UINT8 sprite_type, UINT16 x, UINT16 y);
void SpriteManagerRemove(UINT8 idx);
void SpriteManagerRemoveSprite(Sprite* sprite);

void SpriteManagerFlushRemove(void);
void SpriteManagerUpdate(void);

#endif