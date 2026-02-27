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
#include "Vector.h"

#ifdef USE_SAVEGAME
	#include "savegame.h"
#endif

// reference font_recode_table so that allow passing -Wl-g_font_recode_table=ADDR when not using Print()
#if defined(__SDCC)
static void __force_use_font(void) NAKED { __asm__(".globl _font_recode_table"); }
#endif

extern UINT8 next_state;

UINT8 _is_SGB = FALSE;

UINT8 delta_time;
UINT8 current_state;
UINT8 state_running = FALSE;
FadeMode fade_mode = FADE_DEFAULT;

void SetState(UINT8 state) {
	state_running = FALSE;
	next_state = state;
}

UINT8 vbl_count = 0;
void VBL_isr(void);

void InitWindow(void);

void InitStates(void) BANKED;
void InitSprites(void) BANKED;

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

	INIT_MUSIC();

	InitStates();
	InitSprites();

	// Standard VBlank handler handles scroll
	CRITICAL {
		add_VBL(VBL_isr);
	}

	InitWindow();

#if DEFAULT_SPRITES_SIZE == 8
	SPRITES_8x8;
#else
	SPRITES_8x16;
#endif
	SHOW_SPRITES; SHOW_BKG;
#if defined(NINTENDO)
	WY_REG = (UINT8)(DEVICE_WINDOW_PX_OFFSET_Y + DEVICE_SCREEN_PX_HEIGHT);
#elif defined(SEGA)
	HIDE_LEFT_COLUMN;
#endif

#if defined(SEGA) || (defined(NINTENDO) && defined(CGB))
	SetDefaultColorPalettes();
#endif

	DISPLAY_OFF;
	while(TRUE) {
#if defined(NINTENDO)
		if (fade_mode != FADE_MANUAL) {
			BGP_REG = OBP0_REG = DMG_PALETTE(DMG_WHITE, DMG_LITE_GRAY, DMG_DARK_GRAY, DMG_BLACK);
			OBP1_REG = DMG_PALETTE(DMG_WHITE, DMG_WHITE, DMG_DARK_GRAY, DMG_BLACK);
		}
#endif
		if (stop_music_on_new_state) 
			StopMusic;

		SpriteManagerReset();                           // reset sprite manager and remove all sprites
		scroll_target = NULL;                           // reset scroll target

		state_running = TRUE;
		current_state = next_state;                     // switch to the next scene

		last_tile_loaded = 0;                           // reset tile allocator
#if defined(SEGA) || (defined(NINTENDO) && defined(CGB))
		last_bg_pal_loaded = 0;                         // reset palette allocator for CGB/SMS/GG
#endif
		scroll_x_vblank = scroll_offset_x = 0;          // reset the scroll position;
		scroll_y_vblank = scroll_offset_y = 0;

		HIDE_HUD;                                       // force hiding HUD

		SWITCH_ROM(stateBanks[current_state]);          // switch to the current state bank and stay

		startFuncs[current_state]();                    // initialize current state

		scroll_x_vblank = scroll_x, scroll_y_vblank = scroll_y;

		if (VECTOR_LEN(sprite_manager_updatables)) {
			SpriteManagerUpdate();                  // render sprites on screen if START() of the state spawns any
			SyncVBlank();                           // wait until sprites are actually rendered to OAM
		}

		if (state_running) {                            // initialization function may change state in START()

			switch (fade_mode) {                    // show screen content
				case FADE_ON : FadeOut(); break;
				default: DISPLAY_ON; break;
			}

			Void_Func_Void current_update = updateFuncs[current_state];

			while (state_running) {
				if (!vbl_count) SyncVBlank();        // wait VBlank if not slowdown

				delta_time = (vbl_count < 2u) ? 0u : 1u;
				vbl_count = 0;

				UPDATE_KEYS();                  // read joypad input

				SpriteManagerUpdate();          // render sprites on screen

				current_update();               // update current state
			}

			switch (fade_mode) {                    // hide screen content
				case FADE_ON : FadeIn(); break;
				case FADE_OFF: DISPLAY_OFF; break;
			}
		}

		destroyFuncs[current_state]();                  // destroy current state
	}
}

