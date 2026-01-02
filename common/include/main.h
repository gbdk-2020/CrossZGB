#ifndef MAIN_H
#define MAIN_H

#include <gbdk/platform.h>

#include "Sprite.h"
#include "Scroll.h"
#include "TilesInfo.h"
#include "gbc_hicolor.h"

#define SRAM(VARNAME) ( (uint8_t) & __sram_ ## VARNAME )
#define SRAMREF_EXTERN(VARNAME) extern const void __sram_ ## VARNAME;

typedef void (*Void_Func_Void)(void);
typedef void (*Void_Func_SpritePtr)(Sprite*);

#define DECLARE_STATE(STATE_IDX)   extern UINT8 bank_##STATE_IDX; void Start_##STATE_IDX(void); void Update_##STATE_IDX(void); void Destroy_##STATE_IDX(void)
extern UINT8 stateBanks[];
extern Void_Func_Void startFuncs[];
extern Void_Func_Void updateFuncs[];
extern Void_Func_Void destroyFuncs[];

#define DECLARE_SPRITE(SPRITE_IDX) extern UINT8 bank_##SPRITE_IDX; void Start_##SPRITE_IDX(void); void Update_##SPRITE_IDX(void); void Destroy_##SPRITE_IDX(void)
extern UINT8 spriteBanks[];
extern Void_Func_Void spriteStartFuncs[];
extern Void_Func_Void spriteUpdateFuncs[];
extern Void_Func_Void spriteDestroyFuncs[];
extern const struct MetaSpriteInfo* spriteDatas[];
extern UINT8 spriteDataBanks[];
extern UINT8 spriteIdxs[];
extern UINT8 spriteIdxsH[];
extern UINT8 spriteIdxsV[];
extern UINT8 spriteIdxsHV[];
extern UINT8 spriteFlips[];
extern UINT8 spritePalsOffset[];

extern UINT8 _is_SGB;

extern UINT8 current_state;

void SetState(UINT8 state);

extern UINT8 delta_time;

typedef enum {
	FADE_OFF        = 0,
	FADE_ON         = 1,
	FADE_DEFAULT    = 1,
	FADE_MANUAL     = 2
} FadeMode;

extern FadeMode fade_mode;

#if defined(NINTENDO)
void LCD_isr(void) NONBANKED;
inline void LCD_uninstall(void) { 
	CRITICAL {
		remove_LCD(LCD_isr);
	}
	set_interrupts(IE_REG & ~LCD_IFLAG);
}
inline void LCD_install(void) { 
	CRITICAL {
		remove_LCD(LCD_isr);
		add_LCD(LCD_isr);
	}
	set_interrupts(IE_REG | LCD_IFLAG);
}
#define SetWindowY(A) SetWindowPos(DEVICE_WINDOW_PX_OFFSET_X, (A), DEVICE_SCREEN_PX_HEIGHT) 
#elif defined(SEGA)
#define SetWindowY(A)
#define LCD_uninstall(A)
#define LCD_install(A)
#endif

#define IMPORT_MAP(MAP) extern struct MapInfo MAP; extern const void __bank_##MAP
#define IMPORT_TILES(TILES) extern struct TilesInfo TILES; extern const void __bank_##TILES
#define IMPORT_FONT(FONT) extern const UINT8 FONT##_table[]; extern const UINT8 FONT##_tiles[]; IMPORT_TILES(FONT)

#ifdef USE_SGB_BORDER
#define IMPORT_BORDER(MAP) extern struct MapInfo MAP; extern const void __bank_##MAP
#else
#define IMPORT_BORDER(MAP) extern void MAP
#endif

#if defined(NINTENDO) && defined(CGB)
#define IMPORT_HICOLOR(IMAGE) extern const struct hicolor_data HICOLOR_VAR(IMAGE); extern const void __bank_##IMAGE
#define HICOLOR_START(IMAGE) hicolor_start(&HICOLOR_VAR(IMAGE), BANK(IMAGE))
#define HICOLOR_STOP hicolor_stop()
#else
#define IMPORT_HICOLOR(IMAGE) extern void IMAGE
#define HICOLOR_START(IMAGE) (0)
#define HICOLOR_STOP
#endif

#ifdef SEGA
#define MAP_OVERLAP_SPR __WRITE_VDP_REG(VDP_R2, R2_MAP_0x3800); __WRITE_VDP_REG(VDP_R5, R5_SAT_0x3F00)
#define MAP_OVERLAP_BKG __WRITE_VDP_REG(VDP_R2, R2_MAP_0x1800); __WRITE_VDP_REG(VDP_R5, R5_SAT_0x1F00)
#else
#define MAP_OVERLAP_SPR
#define MAP_OVERLAP_BKG
#endif


#endif