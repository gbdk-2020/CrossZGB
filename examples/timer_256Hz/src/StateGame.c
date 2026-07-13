#include "Banks/SetAutoBank.h"

#include <stdbool.h>

#include "ZGBMain.h"
#include "Scroll.h"
#include "SpriteManager.h"
#include "Music.h"
#include "Keys.h"

#define PLAYER_WIDTH  16
#define PLAYER_HEIGHT 16

DECLARE_MUSIC(ingamemusic);

bool music_playing = true;


extern UINT16 sfx_wav_freq;
extern UINT8 sfx_wav_len;

inline void music_setup_timer_256hz(void) {
#if defined(NINTENDO)
	TMA_REG = ((*(uint8_t *)0x0143 & 0x80) && (KEY1_REG & KEY1F_DBLSPEED)) ? 0x80u : 0xC0u;
	TAC_REG = 0x07u;
	sfx_wav_len = SFX_WAV_LENGTH_8192;
	sfx_wav_freq = SFX_WAV_FREQUENCY_8192;
	music_skip_interrupts = (256 / 64) - 1;
#else
	is_fast;
#endif
}

void START(void) {
	// reconfigure timer for 256Hz
	music_setup_timer_256hz();
	// spawn the player sprite at the center of the screen
	SpriteManagerAdd(SpritePlayer, ((SCREEN_WIDTH - PLAYER_WIDTH) / 2), ((SCREEN_HEIGHT - PLAYER_HEIGHT)/ 2));
	PlayMusic(ingamemusic, 1);
}

void UPDATE(void) {
	if (KEY_TICKED(J_START)) {
		if (music_playing = !music_playing) PlayMusic(ingamemusic, 1); else StopMusic;
	}
}

void DESTROY(void) {
}
