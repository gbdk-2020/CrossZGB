/* **************************************************
   Minimal PSGlib - C library for the SEGA PSG
   ************************************************** */

#ifndef __PSGLIB_H
#define __PSGLIB_H

#include <stdint.h>

#define PSG_STOPPED  0
#define PSG_PLAYING  1

#define PSG_NO_LOOP  0
#define PSG_LOOP     1

extern uint8_t PSGStatus;        // playback status
extern uint8_t PSGMuteMask;      // channel mute mask
extern uint8_t PSGLoopFlag;      // music loop flag

void PSGPlay (void *song, uint8_t loop);
void PSGFrame (void);

#endif