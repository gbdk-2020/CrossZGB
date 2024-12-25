/* **************************************************
   Minimal PSGlib - C library for the SEGA PSG
   ************************************************** */

#ifndef __PSGLIB_H
#define __PSGLIB_H

#include <stdint.h>

#define PSG_STOPPED     0
#define PSG_PLAYING     1

#define PSG_NO_LOOP     0
#define PSG_LOOP        1

#define PSG_CHANNEL0    0b00000001
#define PSG_CHANNEL1    0b00000010
#define PSG_CHANNEL2    0b00000100
#define PSG_CHANNEL3    0b00001000

#define PSG_MUTE_ALL    (PSG_CHANNEL0 | PSG_CHANNEL1 | PSG_CHANNEL2 | PSG_CHANNEL3)
#define PSG_MUTE_NONE   0

extern uint8_t PSGStatus;                    // playback status
extern uint8_t PSGMuteMask;                  // channel mute mask
extern uint8_t PSGLoopFlag;                  // music loop flag

extern void *PSGStart;                       // the pointer to the beginning of music

void PSGPlay (void *song, uint8_t loop);     // initialize the player

void PSGRetriggerChannels(uint8_t mask);     // retrigger channels
void PSGCutChannels(uint8_t mask);           // cut sound on channels

inline void PSGMuteChannels(uint8_t mask) {  // mute channels by mask and cut sound
    if (PSGMuteMask) PSGRetriggerChannels(PSGMuteMask & ~mask);
    if (PSGMuteMask = mask) PSGCutChannels(mask);
}

inline void PSGPause (void) {               // pause music playback
    PSGStatus = PSG_STOPPED;
    PSGCutChannels(~PSGMuteMask);
}

inline void PSGResume (void) {              // resume music playback
    if ((PSGStart) && (PSGStatus == PSG_STOPPED)) {
        PSGRetriggerChannels(~PSGMuteMask);
        PSGStatus = PSG_PLAYING;
    }
}

void PSGFrame (void);                        // process music data (must be called once per frame)

#endif