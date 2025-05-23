#include <string.h>

#include "main.h"

#include "OAMManager.h"
#include "Scroll.h"
#include "Keys.h"
#include "Music.h"
#include "SpriteManager.h"
#include "Fade.h"
#include "Palette.h"
#include "Keys.h"

#ifdef USE_SAVEGAME
	#include "savegame.h"
#endif

// reference font_recode_table so that allow passing -Wl-g_font_recode_table=ADDR when not using Print()
#if defined(__SDCC)
static void __force_use_font(void) NAKED { __asm__(".globl _font_recode_table"); }
#endif

extern UINT8 next_state;

UINT8 _is_SGB = 0;

UINT8 delta_time;
UINT8 current_state;
UINT8 state_running = 0;
UINT8 fade_enabled = TRUE;

void SetState(UINT8 state) {
	state_running = 0;
	next_state = state;
}

UINT8 vbl_count = 0;
void VBL_isr(void) {
	vbl_count ++;
#if defined(NINTENDO)
	move_bkg(scroll_x_vblank + (scroll_offset_x << 3), scroll_y_vblank + (scroll_offset_y << 3));
#elif defined(SEGA)
	if (_shadow_OAM_OFF == 0) {
		__WRITE_VDP_REG_UNSAFE(VDP_RSCX, -(scroll_x_vblank + (scroll_offset_x << 3)));
		__WRITE_VDP_REG_UNSAFE(VDP_RSCY, (((UINT16)(scroll_y_vblank + (scroll_offset_y << 3))) % (DEVICE_SCREEN_BUFFER_HEIGHT << 3)));
	}
#endif
}

void InitStates(void) BANKED;
void InitSprites(void) BANKED;

#if defined(NINTENDO)
#define LYC_SYNC_VALUE 150u
#define LYC_NEVER_FIRE 160u
UINT8 win_start = DEVICE_SCREEN_PX_HEIGHT + 1u, win_stop = LYC_SYNC_VALUE, win_x = DEVICE_WINDOW_PX_OFFSET_X;
void LCD_isr(void) NONBANKED {
	if (LYC_REG == win_start) {
		WX_REG = win_x;
		SHOW_WIN; HIDE_SPRITES;
		LYC_REG = win_stop;
	} else {
		WX_REG = 0;
		HIDE_WIN; SHOW_SPRITES;
		LYC_REG = win_start;
	}
}

void SetWindowPos(UINT8 x, UINT8 y, UINT8 h) {
	if ((h) && (y < DEVICE_SCREEN_PX_HEIGHT)) {
		win_x = WX_REG = x;
		WY_REG = y;
		win_stop = (((UINT16)(y) + h) < LYC_SYNC_VALUE) ? (y + h) : LYC_SYNC_VALUE;
		LYC_REG = win_start = (y) ? (y - 1u) : (LYC_SYNC_VALUE + 1u);
	} else {
		LYC_REG = LYC_NEVER_FIRE;
		HIDE_WIN; SHOW_SPRITES;		
	}
}
#endif

void main(void) {
#if defined(NINTENDO)
	// this delay is required for PAL SNES SGB border commands to work
	for (UINT8 i = 4; i != 0; i--) vsync();
	// set global SGB detection variable
	_is_SGB = sgb_check();
	#ifdef CGB
		cpu_fast();
	#endif
#endif

#ifdef USE_SAVEGAME
	CheckSRAMIntegrity((UINT8*)&savegame, sizeof(Savegame));
#endif

	INIT_KEYS();

	InitOAMs();

	INIT_MUSIC;

	InitStates();
	InitSprites();

	CRITICAL {
#if defined(NINTENDO)
	#ifdef CGB
		TMA_REG = (_cpu == CGB_TYPE) ? 0x78u : 0xBCu;
	#else
		TMA_REG = 0xBCu;
	#endif
		TAC_REG = 0x04u;
		//Instead of calling add_TIM add_low_priority_TIM is used because it can be interrupted. This fixes a random
		//bug hiding sprites under the window (some frames the call is delayed and you can see sprites flickering under the window)
		add_low_priority_TIM(MUSIC_isr);

		add_VBL(VBL_isr);

		STAT_REG |= STATF_LYC;
		add_LCD(LCD_isr);
#elif defined(SEGA)
		add_VBL(VBL_isr);
		add_VBL(MUSIC_isr);
#endif
	}

#if DEFAULT_SPRITES_SIZE == 8
	SPRITES_8x8;
#else
	SPRITES_8x16;
#endif

#if defined(NINTENDO)
	set_interrupts(VBL_IFLAG | TIM_IFLAG | LCD_IFLAG);
	LCDC_REG |= LCDCF_OBJON | LCDCF_BGON;
	WY_REG = (UINT8)(DEVICE_WINDOW_PX_OFFSET_Y + DEVICE_SCREEN_PX_HEIGHT);
#elif defined(SEGA)
	set_interrupts(VBL_IFLAG);

	HIDE_LEFT_COLUMN;
#endif

#if defined(SEGA) || (defined(NINTENDO) && defined(CGB))
	SetDefaultColorPalettes();
#endif

	DISPLAY_OFF;
	while(1) {
		if (stop_music_on_new_state) 
			StopMusic;

		SpriteManagerReset();

		state_running = 1;
		current_state = next_state;
		scroll_target = 0;
		last_tile_loaded = 0;
#if defined(SEGA) || (defined(NINTENDO) && defined(CGB))
		last_bg_pal_loaded = 0;
#endif

#if defined(NINTENDO)
		BGP_REG = OBP0_REG = DMG_PALETTE(DMG_WHITE, DMG_LITE_GRAY, DMG_DARK_GRAY, DMG_BLACK);
		OBP1_REG = DMG_PALETTE(DMG_WHITE, DMG_WHITE, DMG_DARK_GRAY, DMG_BLACK);
#endif

		SWITCH_ROM(stateBanks[current_state]);		// switch to the current state bank and stay

		startFuncs[current_state]();			// initialize current state

		scroll_x_vblank = scroll_x, scroll_y_vblank = scroll_y;

		if (state_running) {				// initialization function may change state immediately
			if (fade_enabled) FadeOut(); else DISPLAY_ON;

			Void_Func_Void current_update = updateFuncs[current_state];

			while (state_running) {
				if (!vbl_count)
					vsync();

				delta_time = (vbl_count < 2u) ? 0u : 1u;
				vbl_count = 0;

				UPDATE_KEYS();

				SpriteManagerUpdate();

				current_update();		// update current state
			}

			if (fade_enabled) FadeIn(); else DISPLAY_OFF;
		}

		destroyFuncs[current_state]();			// destroy current state
	}
}

