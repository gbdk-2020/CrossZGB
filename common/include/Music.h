#ifndef MUSIC_H
#define MUSIC_H

#include <gbdk/platform.h>

#include "Sound.h"

extern void* last_music;
extern UINT8 last_music_bank;
extern UINT8 stop_music_on_new_state;
extern volatile UINT8 music_paused;

void MUSIC_isr(void) NONBANKED;
void __PlayMusic(void* music, UINT8 bank, UINT8 loop);

#ifdef MUSIC_DRIVER_HUGE
#undef MUSIC_DRIVER_GBT
#endif

#define PauseMusic music_paused = 1, sfx_sound_cut_mask(~music_mute_mask)
#define ResumeMusic if (last_music) music_paused = 0

#if defined(MUSIC_DRIVER_HUGE)
	#include "hUGEDriver.h"

	#define INIT_MUSIC_DRIVER()
	#define DECLARE_MUSIC(SONG) extern const void __bank_ ## SONG ## _uge; extern const hUGESong_t SONG ## _uge
	#define PlayMusic(SONG, LOOP) __PlayMusic(&SONG ## _uge, (uint8_t)&__bank_ ## SONG ## _uge, 0)
	#define StopMusic (sfx_sound_cut(), last_music_bank = SFX_STOP_BANK, last_music = NULL)

	#define MuteMusicChannels(CHANNELS) (music_mute_mask = (CHANNELS))
#elif defined(MUSIC_DRIVER_GBT)
	#include "gbt_player.h"

	#define INIT_MUSIC_DRIVER() gbt_stop()
	#define DECLARE_MUSIC(SONG) extern const void __bank_ ## SONG ## _mod_Data; extern const unsigned char * SONG ## _mod_Data[]
	#define PlayMusic(SONG, LOOP) __PlayMusic(SONG ## _mod_Data, (uint8_t)&__bank_ ## SONG ## _mod_Data, LOOP)
	#define StopMusic (sfx_sound_cut(), last_music_bank = SFX_STOP_BANK, last_music = NULL)

	#define MuteMusicChannels(CHANNELS) (music_mute_mask = (CHANNELS))
#elif defined(MUSIC_DRIVER_PSGLIB)
	#include "PSGlib.h"

	void __InitMusicDriver(void);
	void __StopMusic(void);
	#define INIT_MUSIC_DRIVER()
	#define DECLARE_MUSIC(SONG) extern const void __bank_ ## SONG ## _psg; extern const void SONG ## _psg
	#define PlayMusic(SONG, LOOP) __PlayMusic(&SONG ## _psg, (uint8_t)&__bank_ ## SONG ## _psg, LOOP)
	#define StopMusic (sfx_sound_cut(), last_music_bank = SFX_STOP_BANK, last_music = NULL)

	#define MuteMusicChannels(CHANNELS) PSGMuteChannels(CHANNELS)
#elif defined(MUSIC_DRIVER_BANJO)
	#include "banjo.h"

	#define INIT_MUSIC_DRIVER() __InitMusicDriver()
	#define DECLARE_MUSIC(SONG) extern const void __bank_ ## SONG ## _fur; extern const song_data_t SONG ## _fur
	#define PlayMusic(SONG, LOOP) __PlayMusic(&SONG ## _fur, (uint8_t)&__bank_ ## SONG ## _fur, LOOP)
	#define StopMusic __StopMusic()

	#define MuteMusicChannels(CHANNELS) (music_mute_mask = (CHANNELS))
#else
	#define INIT_MUSIC_DRIVER()
	#define DECLARE_MUSIC(SONG) extern void SONG ## _undetected
	#define PlayMusic(SONG, LOOP)
	#define StopMusic

	#define MuteMusicChannels(CHANNELS)
#endif

inline void INIT_MUSIC(void) {
	sfx_sound_init();
	sfx_sound_cut();
	CRITICAL {
	#if defined(NINTENDO)
		// Music is playing on timer on the Game Boy targets
		#ifdef CGB
			TMA_REG = (_cpu == CGB_TYPE) ? 0x78u : 0xBCu;
		#else
			TMA_REG = 0xBCu;
		#endif
		TAC_REG = 0x04u;
		add_low_priority_TIM(MUSIC_isr);
	#elif defined(SEGA)
		// Music is playing on VBlank on the SMS/GG targets
		add_VBL(MUSIC_isr);
	#endif
	}
	#if defined(NINTENDO)
	set_interrupts(IE_REG | TIM_IFLAG);
	#endif
	INIT_MUSIC_DRIVER();
}

#endif