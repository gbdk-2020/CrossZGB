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

#define PSG_CHANNEL0 0b00000001
#define PSG_CHANNEL1 0b00000010
#define PSG_CHANNEL2 0b00000100
#define PSG_CHANNEL3 0b00001000

extern uint8_t PSGStatus;                    // playback status
extern uint8_t PSGMuteMask;                  // channel mute mask
extern uint8_t PSGLoopFlag;                  // music loop flag

void PSGPlay (void *song, uint8_t loop);     // initialize the player

void PSGRetriggerChannels(uint8_t mask);     // retrigger channels
void PSGCutChannels(uint8_t mask);           // cut sound on channels

inline void PSGMuteChannels(uint8_t mask) {  // mute channels by mask and cut sound
   if (PSGMuteMask) PSGRetriggerChannels(PSGMuteMask & ~mask);
   if (PSGMuteMask = mask) PSGCutChannels(mask);
}

void PSGFrame (void);                        // process music data (must be called once per frame)

#endif