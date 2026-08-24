#include "Sprite.h"
#include "TilesInfo.h"
#include "MapInfo.h"
#include "MetaSpriteInfo.h"

#include <stddef.h>

// Sprite
const void AT(offsetof(Sprite, x))                        __offset_Sprite__x;
const void AT(offsetof(Sprite, y))                        __offset_Sprite__y;
                                                          
const void AT(offsetof(Sprite, coll_w))                   __offset_Sprite__coll_w;
const void AT(offsetof(Sprite, coll_h))                   __offset_Sprite__coll_h;
                                                          
const void AT(offsetof(Sprite, mt_sprite_bank))           __offset_Sprite__mt_sprite_bank;
const void AT(offsetof(Sprite, mt_sprite_info))           __offset_Sprite__mt_sprite_info;
                                                          
const void AT(offsetof(Sprite, flips))                    __offset_Sprite__flips;
const void AT(offsetof(Sprite, first_tile))               __offset_Sprite__first_tile;
const void AT(offsetof(Sprite, first_tile_H))             __offset_Sprite__first_tile_H;
const void AT(offsetof(Sprite, first_tile_V))             __offset_Sprite__first_tile_V;
const void AT(offsetof(Sprite, first_tile_HV))            __offset_Sprite__first_tile_HV;
const void AT(offsetof(Sprite, attr_add))                 __offset_Sprite__attr_add;
                                                          
const void AT(offsetof(Sprite, anim_data))                __offset_Sprite__anim_data;
const void AT(offsetof(Sprite, anim_accum_ticks))         __offset_Sprite__anim_accum_ticks;
const void AT(offsetof(Sprite, anim_speed))               __offset_Sprite__anim_speed;
const void AT(offsetof(Sprite, anim_frame))               __offset_Sprite__anim_frame;
const void AT(offsetof(Sprite, mt_sprite))                __offset_Sprite__mt_sprite;
                                                          
const void AT(offsetof(Sprite, mirror))                   __offset_Sprite__mirror;
                                                          
const void AT(offsetof(Sprite, ctx))                      __offset_Sprite__ctx;
                                                          
const void AT(offsetof(Sprite, lim_x))                    __offset_Sprite__lim_x;
const void AT(offsetof(Sprite, lim_y))                    __offset_Sprite__lim_y;
const void AT(offsetof(Sprite, type))                     __offset_Sprite__type;
const void AT(offsetof(Sprite, marked_for_removal))       __offset_Sprite__marked_for_removal;
const void AT(offsetof(Sprite, unique_id))                __offset_Sprite__unique_id;

const void AT(offsetof(Sprite, unique_id)+sizeof(UINT16)) __offset_Sprite__flags;           // struct bitfields become flags!

const void AT(offsetof(Sprite, coll_group))               __offset_Sprite__coll_group;
const void AT(offsetof(Sprite, coll_group_down))          __offset_Sprite__coll_group_down;

const void AT(offsetof(Sprite, custom_data))              __offset_Sprite__custom_data;

// TilesInfo
const void AT(offsetof(TilesInfo, num_frames))            __offset_TilesInfo__num_frames;
const void AT(offsetof(TilesInfo, data))                  __offset_TilesInfo__data;
const void AT(offsetof(TilesInfo, num_pals))              __offset_TilesInfo__num_pals;
const void AT(offsetof(TilesInfo, pals))                  __offset_TilesInfo__pals;
const void AT(offsetof(TilesInfo, color_data))            __offset_TilesInfo__color_data;

// MapInfo
const void AT(offsetof(MapInfo, data))                    __offset_MapInfo__data;
const void AT(offsetof(MapInfo, width))                   __offset_MapInfo__width;
const void AT(offsetof(MapInfo, height))                  __offset_MapInfo__height;
const void AT(offsetof(MapInfo, attributes))              __offset_MapInfo__attributes;

const void AT(offsetof(MapInfo, tiles_bank))              __offset_MapInfo__tiles_bank;
const void AT(offsetof(MapInfo, tiles))                   __offset_MapInfo__tiles;

const void AT(offsetof(MapInfo, extra_tiles_bank))        __offset_MapInfo__extra_tiles_bank;
const void AT(offsetof(MapInfo, extra_tiles))             __offset_MapInfo__extra_tiles;

// MetaSpriteInfo
const void AT(offsetof(MetaSpriteInfo, width))            __offset_MetaSpriteInfo__width;
const void AT(offsetof(MetaSpriteInfo, height))           __offset_MetaSpriteInfo__height;
const void AT(offsetof(MetaSpriteInfo, num_tiles))        __offset_MetaSpriteInfo__num_tiles;
const void AT(offsetof(MetaSpriteInfo, data))             __offset_MetaSpriteInfo__data;
const void AT(offsetof(MetaSpriteInfo, num_palettes))     __offset_MetaSpriteInfo__num_palettes;
const void AT(offsetof(MetaSpriteInfo, palettes))         __offset_MetaSpriteInfo__palettes;
const void AT(offsetof(MetaSpriteInfo, num_sprites))      __offset_MetaSpriteInfo__num_sprites;
const void AT(offsetof(MetaSpriteInfo, metasprites))      __offset_MetaSpriteInfo__metasprites;
