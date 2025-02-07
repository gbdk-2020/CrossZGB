/* **************************************************
   Minimal PSGlib - C library for the SEGA PSG
   ************************************************** */

#include <gbdk/platform.h>
#include <stdint.h>

#include "PSGlib.h"

#define PSGData             #0x40

#define PSGLatch            #0b10000000
#define PSGChannel0         #0b00000000
#define PSGChannel1         #0b00100000
#define PSGChannel2         #0b01000000
#define PSGChannel3         #0b01100000
#define PSGVolumeData       #0b00010000

#define PSGWait             #0x38
#define PSGSubString        #0x08
#define PSGLoop             #0x01
#define PSGEnd              #0x00

static SFR AT(0x7F) PSGPort;

uint8_t PSGStatus = PSG_STOPPED;        // playback status
uint8_t PSGMuteMask = PSG_MUTE_NONE;    // channel mute mask
uint8_t PSGLoopFlag;                    // the tune should loop or not (flag)

void *PSGStart;                         // the pointer to the beginning of music
static void *PSGPointer;                // the pointer to the current
static void *PSGLoopPoint;              // the pointer to the loop begin
static uint8_t PSGSkipFrames;           // the frames we need to skip

static uint8_t PSGLastChannel;          // last channel
static uint8_t PSGLastMuted;            // current latched channel is muted

typedef struct shadow_reg_t {
    uint8_t tone;
    uint8_t data;
    uint8_t volume;
} shadow_reg_t;

static shadow_reg_t PSGShadow[4];       // shadow registers to retrigger channels

static uint8_t PSGSubLen;               // length of the substring we are playing
static void *PSGSubRetAddr = 0;         // return to this address when substring is over

void PSGPlay (void *song, uint8_t loop) {
    PSGStatus = PSG_STOPPED;            // prevent running PSGFrame

    PSGLoopFlag = loop;
    PSGStart = song;                    // store the beginning point of music
    PSGPointer = song;                  // set music pointer to the beginning
    PSGLoopPoint = song;                // loop pointer points to the beginning

    PSGSkipFrames = 0;                  // reset the skip frames
    PSGSubLen = 0;                      // reset the substring len (for compression)
    PSGLastChannel = 0;                 // latch channel 0

    PSGShadow[PSGChannel0 >> 5].volume = PSGLatch | PSGChannel0 | PSGVolumeData | 0x0f;
    PSGShadow[PSGChannel1 >> 5].volume = PSGLatch | PSGChannel1 | PSGVolumeData | 0x0f;
    PSGShadow[PSGChannel2 >> 5].volume = PSGLatch | PSGChannel2 | PSGVolumeData | 0x0f;
    PSGShadow[PSGChannel3 >> 5].volume = PSGLatch | PSGChannel3 | PSGVolumeData | 0x0f;

    PSGStatus = PSG_PLAYING;            // start playback
}

void PSGRetriggerChannels(uint8_t mask) NAKED {
    mask;
    __asm
        and #(PSG_CHANNEL0 | PSG_CHANNEL1 | PSG_CHANNEL2 | PSG_CHANNEL3)
        ret z                           ; if nothing to retrigger then return
        ld c, #_PSGPort                 ; c points to the PSG port
        ld hl, #_PSGShadow              ; hl points to shadow regs copy
        ld de, #3
0$:
        srl a                           ; check bits one by one
        jp nc, 1$                       ; if zero then advance to next channel
        outi                            ; write tone to PSG
        outi                            ; write data to PSG
        outi                            ; write volume to PSG

        or a                            ; check if anything else to retrigger
        ret z                           ; return if done
        jr  0$                          ; loop
1$:
        add hl, de                      ; advance pointer to the next channel
        jr  0$                          ; loop
    __endasm;
}

void PSGCutChannels(uint8_t mask) NAKED {
    mask;
    __asm
        and #(PSG_CHANNEL0 | PSG_CHANNEL1 | PSG_CHANNEL2 | PSG_CHANNEL3)
        ret z                           ; if nothing to retrigger then return
        ld c, #_PSGPort                 ; c points to the PSG port
        ld hl, #2$                      ; hl points to the muting data
0$:
        srl a
        jr nc, 1$
        outi

        or a
        ret z
        jr 0$
1$:
        inc hl
        jr 0$
2$:
        .irp ch,PSGChannel0,PSGChannel1,PSGChannel2,PSGChannel3
            .db #(PSGLatch | ch | PSGVolumeData | 0x0f)
        .endm
    __endasm;
}

void PSGFrame (void) NAKED {
    __asm
        .ez80

        ld a, (_PSGStatus)              ; check if we have got to play a tune
        or a
        ret z

        xor a
        ld hl, #_PSGSkipFrames
        or (hl)
        jp z, 0$
        dec (hl)
        ret
0$:
        ld de, (_PSGPointer)            ; read current address

11$:
        ld a, (de)
        ld b, a                         ; load PSG byte (in B)
        inc de                          ; point to next byte
        ld a, (_PSGSubLen)              ; read substring len
        or a
        jr z, 1$                        ; check if it is 0 (we are not in a substring)
        dec a                           ; decrease len
        ld (_PSGSubLen), a              ; save len
        jr nz, 1$
        ld de, (_PSGSubRetAddr)         ; substring is over, retrieve return address

1$:
        ld a, b                         ; copy PSG byte into A
        cp PSGLatch                     ; is it a latch?
        jp c, 7$                        ; if < $80 then it is NOT a latch

; --- latch --------------------

        rlca
        rlca
        rlca
        and #0x03

        ld (_PSGLastChannel), a         ; store last latched channel

        ld hl, #15$                     ; point to the mute bits array
        add l
        ld l, a
        adc h
        sub l
        ld h, a

        ld a, (_PSGMuteMask)
        and (hl)
        ld (_PSGLastMuted), a           ; do not write data to PSG flag
        jp nz, 12$                      ; save to shadow copy if muted
        jp 2$                           ; else write to PSG
15$:
        .db PSG_CHANNEL0, PSG_CHANNEL1, PSG_CHANNEL2, PSG_CHANNEL3

; --- no latch -----------------
7$:
        cp PSGData
        jr c, 8$                        ; if < $40 then it is a command

        ld a, (_PSGLastMuted)           ; do not write data to PSG flag
        or a
        jr nz, 12$                      ; save to shadow copy if muted, else fall through write to PSG

; --- write to PSG --------------

2$:
        ld a, b
        out (_PSGPort), a               ; write to PSG, fall through save to shadow copy

; --- save shadow registers -----

12$:
        ld iy, #_PSGShadow
        ld a, (_PSGLastChannel)
        ld c, a
        add a
        add c
        add iyl
        ld iyl, a
        adc iyh
        sub iyl
        ld iyh, a

        bit 4, b
        jp z, 14$
        ld 2(iy), b                     ; volume byte
        jp 11$
14$:
        ld a, (_PSGLastChannel)
        cp #(PSGChannel3 >> 5)
        jp z, 16$                       ; special case for the noise

        bit 7, b
        jp z, 13$
        ld 0(iy), b                     ; tone byte
        jp 11$
13$:
        ld 1(iy), b                     ; tone data byte
        jp 11$

16$:
        ld a, b
        and #0b00000111
        or #(PSGLatch | PSGChannel3)
        ld 0(iy), a                     ; tone byte
        ld 1(iy), a                     ; tone data byte
        jp 11$

; --- commands -------------------
8$:
        cp PSGWait
        jr z, 4$                        ; no additional frames
        jr c, 5$                        ; other commands?
        and #0x07                       ; take only the last 3 bits for skip frames
        ld (_PSGSkipFrames), a          ; we got additional frames
4$:
        ld (_PSGPointer),de             ; save current address
        ret                             ; frame done

5$:
        cp PSGSubString
        jr nc, 10$
        cp PSGEnd
        jr z, 9$
        cp PSGLoop
        ret nz

; --- set loop point -------------
        ld (_PSGLoopPoint), de
        jp 11$

; --- set loop -------------------
9$:
        ld a, (_PSGLoopFlag)            ; looping requested?
        or a
        jp nz, 6$                       ; No - stop
        ld a, #PSG_STOPPED
        ld (_PSGStatus), a
        ret

6$:
        ld de, (_PSGLoopPoint)
        jp 11$

; --- substring ------------------
10$:
        sub #(PSGSubString - 4)         ; len is value - $08 + 4
        ld (_PSGSubLen), a              ; save len
        ld a, (de)                      ; load substring address (offset)
        ld c, a
        inc de
        ld a, (de)
        ld b, a
        inc de
        ld (_PSGSubRetAddr), de         ; save return address
        ld hl, (_PSGStart)
        add hl, bc                      ; make substring current
        ld d, h
        ld e, l
        jp 11$
    __endasm;
}
