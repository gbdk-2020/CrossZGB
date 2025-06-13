#pragma bank 255

#include "ZGBMain.h"

#define _STATE_(STATE_ID, DESTROY) DECLARE_STATE(STATE_ID); extern const void __bank_##STATE_ID;
STATES
#undef _STATE_

UINT8 stateBanks[N_STATES];
Void_Func_Void startFuncs[N_STATES];
Void_Func_Void updateFuncs[N_STATES];
Void_Func_Void destroyFuncs[N_STATES];

void StateDestroyDefault(void) NONBANKED {
}

#define _STATE_(STATE_ID, DESTROY) [STATE_ID] = {BANK(STATE_ID)},
static const UINT8 __stateBanks[N_STATES] = {
	STATES
};
#undef _STATE_
#define _STATE_(STATE_ID, DESTROY) [STATE_ID] = Start_##STATE_ID,
static const Void_Func_Void const __startFuncs[N_STATES] = {
	STATES
};
#undef _STATE_
#define _STATE_(STATE_ID, DESTROY) [STATE_ID] = Update_##STATE_ID,
static const Void_Func_Void const __updateFuncs[N_STATES] = {
	STATES
};
#undef _STATE_
#define _STATE_(STATE_ID, DESTROY) [STATE_ID] = DESTROY,
static const Void_Func_Void const __destroyFuncs[N_STATES] = {
	STATES
};
#undef _STATE_
void InitStates(void) BANKED {
	memcpy(stateBanks, __stateBanks, sizeof(stateBanks));
	memcpy(startFuncs, __startFuncs, sizeof(startFuncs));
	memcpy(updateFuncs, __updateFuncs, sizeof(updateFuncs));
	memcpy(destroyFuncs, __destroyFuncs, sizeof(destroyFuncs));
}

//-------------------------------------------------------------------------------------------------
#include "MetaSpriteInfo.h"
#include "Flip.h"
#define _SPRITE_(SPRITE_ID, DATA, FLIP) DECLARE_SPRITE(SPRITE_ID); extern const void __bank_##SPRITE_ID; extern const void __bank_##DATA;
SPRITES
#undef _SPRITE_

UINT8 spriteBanks[SPRITES_ARRAY_LEN];
UINT8 spriteDataBanks[SPRITES_ARRAY_LEN];

Void_Func_Void spriteStartFuncs[SPRITES_ARRAY_LEN];
Void_Func_Void spriteUpdateFuncs[SPRITES_ARRAY_LEN];
Void_Func_Void spriteDestroyFuncs[SPRITES_ARRAY_LEN];

#define _SPRITE_(SPRITE_ID, DATA, FLIP) extern const void __bank_##DATA; extern const struct MetaSpriteInfo DATA;
	SPRITES
#undef _SPRITE_

const struct MetaSpriteInfo* spriteDatas[SPRITES_ARRAY_LEN];
UINT8 spriteFlips[SPRITES_ARRAY_LEN];

UINT8 spriteIdxs[SPRITES_ARRAY_LEN];
UINT8 spriteIdxsH[SPRITES_ARRAY_LEN];
UINT8 spriteIdxsV[SPRITES_ARRAY_LEN];
UINT8 spriteIdxsHV[SPRITES_ARRAY_LEN];
UINT8 spritePalsOffset[SPRITES_ARRAY_LEN];

#define _SPRITE_(SPRITE_ID, DATA, FLIP) [SPRITE_ID] = {BANK(SPRITE_ID)},
static const UINT8 __spriteBanks[SPRITES_ARRAY_LEN] = {
	SPRITES
};
#undef _SPRITE_
#define _SPRITE_(SPRITE_ID, DATA, FLIP) [SPRITE_ID] = {BANK(DATA)},
static const UINT8 __spriteDataBanks[SPRITES_ARRAY_LEN] = {
	SPRITES
};
#undef _SPRITE_
#define _SPRITE_(SPRITE_ID, DATA, FLIP) [SPRITE_ID] = Start_##SPRITE_ID,
static const Void_Func_Void const __spriteStartFuncs[SPRITES_ARRAY_LEN] = {
	SPRITES
};
#undef _SPRITE_
#define _SPRITE_(SPRITE_ID, DATA, FLIP) [SPRITE_ID] = Update_##SPRITE_ID,
static const Void_Func_Void const __spriteUpdateFuncs[SPRITES_ARRAY_LEN] = {
	SPRITES
};
#undef _SPRITE_
#define _SPRITE_(SPRITE_ID, DATA, FLIP) [SPRITE_ID] = Destroy_##SPRITE_ID,
static const Void_Func_Void const __spriteDestroyFuncs[SPRITES_ARRAY_LEN] = {
	SPRITES
};
#undef _SPRITE_
#define _SPRITE_(SPRITE_ID, DATA, FLIP) [SPRITE_ID] = &DATA,
static const struct MetaSpriteInfo* const __spriteDatas[SPRITES_ARRAY_LEN] = {
	SPRITES
};
#undef _SPRITE_
#define _SPRITE_(SPRITE_ID, DATA, FLIP) [SPRITE_ID] = FLIP,
static const UINT8 __spriteFlips[SPRITES_ARRAY_LEN] = {
	SPRITES
};
#undef _SPRITE_
void InitSprites(void) BANKED {
	memcpy(spriteBanks, __spriteBanks, sizeof(spriteBanks));
	memcpy(spriteDataBanks, __spriteDataBanks, sizeof(spriteDataBanks));
	memcpy(spriteStartFuncs, __spriteStartFuncs, sizeof(spriteStartFuncs));
	memcpy(spriteUpdateFuncs, __spriteUpdateFuncs, sizeof(spriteUpdateFuncs));
	memcpy(spriteDestroyFuncs, __spriteDestroyFuncs, sizeof(spriteDestroyFuncs));
	memcpy(spriteDatas, __spriteDatas, sizeof(spriteDatas));
	memcpy(spriteFlips, __spriteFlips, sizeof(spriteFlips));
}
