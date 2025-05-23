/* **************************************************
   Minimal PSGlib - C library for the SEGA PSG
   ************************************************** */

#include <gbdk/platform.h>
#include <stdint.h>

#include "PSGlib.h"

#define PSGData             #0x40

#define PSGWait             #0x38
#define PSGSubString        #0x08
#define PSGLoop             #0x01
#define PSGEnd              #0x00

uint8_t PSGStatus = PSG_STOPPED;        // playback status
uint8_t PSGMuteMask = PSG_MUTE_NONE;    // channel mute mask
uint8_t PSGLoopFlag;                    // the tune should loop or not (flag)

void *PSGStart;                         // the pointer to the beginning of music
static void *PSGPointer;                // the pointer to the current
static void *PSGLoopPoint;              // the pointer to the loop begin
static uint8_t PSGSkipFrames;           // the frames we need to skip

static uint8_t PSGLastChannel;          // last channel

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

    PSGShadow[PSG_CH0 >> 5].volume = PSG_LATCH | PSG_CH0 | PSG_VOLUME | 0x0f;
    PSGShadow[PSG_CH1 >> 5].volume = PSG_LATCH | PSG_CH1 | PSG_VOLUME | 0x0f;
    PSGShadow[PSG_CH2 >> 5].volume = PSG_LATCH | PSG_CH2 | PSG_VOLUME | 0x0f;
    PSGShadow[PSG_CH3 >> 5].volume = PSG_LATCH | PSG_CH3 | PSG_VOLUME | 0x0f;

    PSGStatus = PSG_PLAYING;            // start playback
}

void PSGRetriggerChannels(uint8_t mask) NAKED {
    mask;
    __asm
        and #(PSG_CHANNEL0 | PSG_CHANNEL1 | PSG_CHANNEL2 | PSG_CHANNEL3)
        ret z                           ; if nothing to retrigger then return
        ld c, #_PSG                     ; c points to the PSG port
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
        ld c, #_PSG                     ; c points to the PSG port
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
        .irp ch,PSG_CH0,PSG_CH1,PSG_CH2,PSG_CH3
            .db #(PSG_LATCH | ch | PSG_VOLUME | 0x0f)
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
        cp #PSG_LATCH                   ; is it a latch?
        jp c, 7$                        ; if < $80 then it is NOT a latch

; --- latch --------------------

        rlca
        rlca
        rlca
        and #0x03
        ld (_PSGLastChannel), a         ; store last latched channel
        jp 12$                          ; save to shadow copy

; --- no latch -----------------
7$:
        cp PSGData
        jr c, 8$                        ; if < $40 then it is a command

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
        jp 17$
14$:
        ld a, (_PSGLastChannel)
        cp #(PSG_CH3 >> 5)
        jp z, 16$                       ; special case for the noise

        bit 7, b
        jp z, 13$
        ld 0(iy), b                     ; tone byte
        jp 17$
13$:
        ld 1(iy), b                     ; tone data byte
        jp 17$

16$:
        ld a, b
        and #0b00000111
        or #(PSG_LATCH | PSG_CH3)
        ld b, a
        ld 0(iy), b                     ; tone byte
        ld 1(iy), b                     ; tone data byte

; --- check mute and write -------
17$:
        ld a, (_PSGLastChannel)

        ld hl, #15$                     ; point to the mute bits array
        add l
        ld l, a
        adc h
        sub l
        ld h, a

        ld a, (_PSGMuteMask)
        and (hl)
        jp nz, 11$                      ; dont write to PSG if muted

        ld a, b
        out (_PSG), a                   ; write to PSG
        jp 11$                          ; loop
15$:
        .db PSG_CHANNEL0, PSG_CHANNEL1, PSG_CHANNEL2, PSG_CHANNEL3

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
