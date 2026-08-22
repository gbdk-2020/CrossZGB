#include "Sprite.h"

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

const void AT(offsetof(Sprite, unique_id)+sizeof(UINT16)) __offset_Sprite__flags;

const void AT(offsetof(Sprite, coll_group))               __offset_Sprite__coll_group;
const void AT(offsetof(Sprite, coll_group_down))          __offset_Sprite__coll_group_down;

const void AT(offsetof(Sprite, custom_data))              __offset_Sprite__custom_data;
