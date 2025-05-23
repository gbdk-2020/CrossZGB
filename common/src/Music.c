#include "Music.h"
#include "Sound.h"

#define FORCE_CUT_SFX

#ifdef MUSIC_DRIVER_HUGE
#undef MUSIC_DRIVER_GBT
#endif

volatile UINT8 music_paused = 1;
volatile UINT8 music_mute_mask = MUTE_MASK_NONE;
volatile UINT8 sfx_priority = SFX_PRIORITY_MINIMAL;

void* last_music = NULL;
UINT8 last_music_bank = SFX_STOP_BANK;

UINT8 stop_music_on_new_state = 1;


#if defined(MUSIC_DRIVER_HUGE)
inline void __SetMusicMuteMask(UINT8 mask) {
	hUGE_mute_mask = mask;
	hUGE_reset_wave();
}
#elif defined(MUSIC_DRIVER_GBT)
inline void __SetMusicMuteMask(UINT8 mask) {
	gbt_enable_channels(0xF & ~(mask));
	gbt_reset_ch3_instrument();
}
#elif defined(MUSIC_DRIVER_PSGLIB)
inline void __SetMusicMuteMask(UINT8 mask) {
	PSGMuteMask = mask;
}
#elif defined(MUSIC_DRIVER_BANJO)

BANJO_SONG_CHANNELS(CHAN_COUNT_SN)

void __InitMusicDriver(void) NONBANKED {
	UBYTE __save = CURRENT_BANK;
	SWITCH_ROM(1);
	banjo_check_hardware();
	banjo_init(CHAN_COUNT_SN, BANJO_HAS_SN);
	SWITCH_ROM(__save);
}
void __StopMusic(void) NONBANKED {
	if (last_music_bank == SFX_STOP_BANK)
		return;

	UBYTE music_bank = last_music_bank; // save current music bank
	last_music_bank = SFX_STOP_BANK;    // prevent running music updates from the interrupt
	last_music = NULL;                  // clear the last music pointer

	UBYTE __save = CURRENT_BANK, __save2 = MAP_FRAME2;
	SWITCH_ROM(1);
	SWITCH_ROM2(music_bank);
	banjo_song_stop();
	SWITCH_ROM2(__save2);
	SWITCH_ROM(__save);
}
void __SetMusicMuteMask(UINT8 mask) NONBANKED {
	if (last_music_bank == SFX_STOP_BANK)
		return;
	UBYTE __save = CURRENT_BANK, __save2 = MAP_FRAME2;
	SWITCH_ROM(1);
	SWITCH_ROM2(last_music_bank);
	if (mask & CHANNEL_1) banjo_mute_song_channel(0); else banjo_unmute_song_channel(0);
	if (mask & CHANNEL_2) banjo_mute_song_channel(1); else banjo_unmute_song_channel(1);
	if (mask & CHANNEL_3) banjo_mute_song_channel(2); else banjo_unmute_song_channel(2);
	if (mask & CHANNEL_4) banjo_mute_song_channel(3); else banjo_unmute_song_channel(3);
	SWITCH_ROM2(__save2);
	SWITCH_ROM(__save);
}
#else
#define __SetMusicMuteMask(A)
#endif


void MUSIC_isr(void) NONBANKED {
	static UINT8 old_mute_mask = ~MUTE_MASK_NONE;
	if (old_mute_mask != music_mute_mask) {
		__SetMusicMuteMask(music_mute_mask); 
		old_mute_mask = music_mute_mask;
	}

	if (sfx_play_bank != SFX_STOP_BANK) {
		if (!sfx_play_isr()) {
#ifdef FORCE_CUT_SFX
			sfx_sound_cut_mask(music_mute_mask);
#endif
			__SetMusicMuteMask(MUTE_MASK_NONE);
			music_mute_mask = old_mute_mask = MUTE_MASK_NONE;

			sfx_priority = SFX_PRIORITY_MINIMAL;
			sfx_play_bank = SFX_STOP_BANK;
		}
	}

	if (music_paused)
		return;
	if (last_music_bank == SFX_STOP_BANK)
		return;

#if defined(MUSIC_DRIVER_HUGE)
	UBYTE __save = CURRENT_BANK;
	SWITCH_ROM(last_music_bank);
	hUGE_dosound();
	SWITCH_ROM(__save);
#elif defined(MUSIC_DRIVER_GBT)
	UBYTE __save = CURRENT_BANK;
	gbt_update();
	SWITCH_ROM(__save);
#elif defined(MUSIC_DRIVER_PSGLIB)
	UBYTE __save = CURRENT_BANK;
	SWITCH_ROM(last_music_bank);
	PSGFrame();
	SWITCH_ROM(__save);
#elif defined(MUSIC_DRIVER_BANJO)
	UBYTE __save = CURRENT_BANK, __save2 = MAP_FRAME2;
	SWITCH_ROM(1);
	SWITCH_ROM2(last_music_bank);
	banjo_update_song();
	SWITCH_ROM2(__save2);
	SWITCH_ROM(__save);
#endif
}

void __PlayMusic(void* music, UINT8 bank, UINT8 loop) NONBANKED {
	bank; loop;
	if(music != last_music) {
		last_music_bank = SFX_STOP_BANK;
		UBYTE __save = CURRENT_BANK;
#if defined(MUSIC_DRIVER_GBT)
		gbt_play(music, bank, 7);
		gbt_loop(loop);
#elif defined(MUSIC_DRIVER_HUGE)
		INIT_SOUND();
		SWITCH_ROM(bank);
		hUGE_init(music);
#elif defined(MUSIC_DRIVER_PSGLIB)
		SWITCH_ROM(bank);
		PSGPlay(music, loop);
#elif defined(MUSIC_DRIVER_BANJO)
		UBYTE __save2 = MAP_FRAME2;
		SWITCH_ROM(1);
		SWITCH_ROM2(bank);
		banjo_play_song(music);
		banjo_set_song_loop_mode(loop);
		SWITCH_ROM2(__save2);
#endif
		SWITCH_ROM(__save);
		last_music = music;
		last_music_bank = bank;
	}
	music_paused = 0;
}
