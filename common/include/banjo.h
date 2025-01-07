
// banjo sound driver
// Joe Kennedy 2024

#ifndef __BANJO_H
#define __BANJO_H

// instructions
#define NOTE_ON             0x00
#define NOTE_OFF            0x01
#define INSTRUMENT_CHANGE   0x02
#define VOLUME_CHANGE       0x03
#define FM_DRUM             0x04
#define SN_NOISE_MODE       0x05
#define SLIDE_UP            0x06
#define SLIDE_DOWN          0x07
#define SLIDE_PORTA         0x08
#define SLIDE_OFF           0x09
#define ARPEGGIO            0x0a
#define ARPEGGIO_OFF        0x0b
#define VIBRATO             0x0c
#define VIBRATO_OFF         0x0d
#define LEGATO_ON           0x0e
#define LEGATO_OFF          0x0f
#define GAME_GEAR_PAN       0x10
#define AY_ENV_ON           0x11
#define AY_ENV_OFF          0x12
#define AY_ENV_SHAPE        0x13
#define AY_ENV_PERIOD_HI    0x14
#define AY_ENV_PERIOD_LO    0x15
#define AY_CHANNEL_MIX      0x16
#define AY_NOISE_PITCH      0x17
#define AY_ENV_PERIOD_WORD  0x18
#define ORDER_JUMP          0x19
#define SET_SPEED_1         0x1a
#define SET_SPEED_2         0x1b
#define ORDER_NEXT          0x1c
#define NOTE_DELAY          0x1d
#define END_LINE            0x80

#define CHAN_COUNT_SN		0x4
#define CHAN_COUNT_OPLL		0x9
#define CHAN_COUNT_OPLL_DRUMS   0xb
#define CHAN_COUNT_AY		0x3

#define BANJO_HAS_SN		    0x1
#define BANJO_HAS_OPLL		    0x2
#define BANJO_HAS_AY		    0x4
#define BANJO_HAS_DUAL_SN		0x8

#define BANJO_LOOP_OFF          0
#define BANJO_LOOP_ON           1

#define CHAN_FLAG_MUTED     	0x01
#define CHAN_FLAG_NOTE_ON       0x02
#define CHAN_FLAG_LEGATO        0x04
#define CHAN_FLAG_VOLUME_MACRO  0x08
#define CHAN_FLAG_VIBRATO	    0x10
#define CHAN_FLAG_ARPEGGIO	    0x20
#define CHAN_FLAG_SLIDE	        0x40
#define CHAN_FLAG_EX_MACRO	    0x80

#define SYS_FLAG_GG             1
#define SYS_FLAG_MARK_III       2
#define SYS_FLAG_SYSTEM_E       4

// declares enough space for x song channels and pointers to them
#define BANJO_SONG_CHANNELS(x) \
    channel_t song_channels[x]; \
    channel_t *song_channel_ptrs[x]; 

typedef void (*banjo_func_ptr_t)(void);

typedef struct instrument_s {

    unsigned char volume_macro_len;
    unsigned char volume_macro_loop;
    const unsigned char * volume_macro_ptr;

    unsigned char ex_macro_len;
    unsigned char ex_macro_type;
    unsigned char ex_macro_loop;
    const unsigned char * ex_macro_ptr;

    unsigned char fm_preset;
    const unsigned char * fm_patch;

} instrument_t;

typedef struct song_data_s {

    unsigned char magic_byte;
    unsigned char bank;
    unsigned char channel_count;

    unsigned char flags;
    unsigned char master_volume;
    unsigned char master_volume_fade;

    unsigned char has_chips;

    unsigned char sfx_channel;
    unsigned char sfx_subchannel;

    unsigned char time_base;
    unsigned char speed_1;
    unsigned char speed_2;

    unsigned char pattern_length;
    unsigned char orders_length;

    const instrument_t * const * instrument_pointers;
    const unsigned char * const * const * order_pointers;

    unsigned char subtic;
    unsigned char tic;
    unsigned char line;
    unsigned char order;

    unsigned char order_jump;

    unsigned char noise_mode;
    unsigned char panning;

    const banjo_func_ptr_t *channel_init_call;
    const banjo_func_ptr_t *channel_update_call;
    const banjo_func_ptr_t *channel_mute_calls;
    const banjo_func_ptr_t *song_mute;
    const banjo_func_ptr_t *song_stop;

} song_data_t;

typedef struct channel_s {

    unsigned char flags;
    unsigned char subchannel;
    unsigned char port;
    unsigned char events;

    unsigned int freq;
    unsigned int target_freq;

    unsigned char volume;
    unsigned char midi_note;
    unsigned char instrument_num;
            
    unsigned char slide_amount;
    unsigned char slide_type;
    
    unsigned char vibrato_depth;
    unsigned char vibrato_counter;
    unsigned char vibrato_counter_add;

    unsigned char arpeggio_pos;
    unsigned char arpeggio;

    unsigned char * order_table_ptr;
    unsigned char * pattern_ptr;
    unsigned char line_wait;
    unsigned char tic_wait;

    unsigned char volume_macro_len;
    unsigned char volume_macro_vol;
    unsigned char volume_macro_pos;
    unsigned char volume_macro_loop;
    const unsigned char * volume_macro_ptr;

    unsigned char ex_macro_type;
    unsigned char ex_macro_len;
    unsigned char ex_macro_val;    
    unsigned char ex_macro_pos;
    unsigned char ex_macro_loop;
    const unsigned char * ex_macro_ptr;

    unsigned char patch;
    
} channel_t;

extern unsigned char banjo_has_chips;
extern unsigned char banjo_system_flags;
extern unsigned char banjo_max_channels;

extern unsigned char song_playing;
extern song_data_t song_state;

// checks the hardware to see which chips are available
// updates banjo_has_chips and banjo_system_flags
void banjo_check_hardware(void);

// initialise banjo
// sets pointers to channels, updates banjo_max_channels
// and sets up the required chips if necessary
void banjo_init(unsigned char channel_count, unsigned char chips);

// start playing song
// you should change to the song's bank before calling this
void banjo_play_song(const song_data_t *song_ptr);

// update song if one is playing
// change to the song's bank before calling this
void banjo_update_song(void);

// stop the currently playing song
// change to the song's bank before calling this
void banjo_song_stop(void);

// resume playback of a stopped song
// change to the song's bank before calling this
void banjo_song_resume(void);

// set song loop mode
// 	   0 for looping off
// 	>= 1 for looping on
void banjo_set_song_loop_mode(unsigned char mode);

// mute the given song channel
// change to the song's bank before calling this
void banjo_mute_song_channel(unsigned char chan);

// unmute the given song channel
// change to the song's bank before calling this
void banjo_unmute_song_channel(unsigned char chan);

// set master volume for song playback
// values >= 0x80 are full volume
// lower values attenuate the volume
void banjo_set_song_master_volume(unsigned char volume);

// subtracts amount from master volume every frame
void banjo_song_fade_out(unsigned char amount);

// adds amount to master volume every frame
void banjo_song_fade_in(unsigned char amount);

#endif