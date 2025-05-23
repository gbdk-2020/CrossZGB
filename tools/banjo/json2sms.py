#!/usr/bin/env python3

# json2sms
# Joe Kennedy 2024

import sys
import json
import math
from pathlib import Path
from optparse import OptionParser

MAGIC_BYTE          = 0xba

NOTE_ON             = 0x00
NOTE_OFF            = 0x01
INSTRUMENT_CHANGE   = 0x02
VOLUME_CHANGE       = 0x03
FM_DRUM             = 0x04
SN_NOISE_MODE       = 0x05
SLIDE_UP            = 0x06
SLIDE_DOWN          = 0x07
SLIDE_PORTA         = 0x08
SLIDE_OFF           = 0x09
ARPEGGIO            = 0x0a
ARPEGGIO_OFF        = 0x0b
VIBRATO             = 0x0c
VIBRATO_OFF         = 0x0d
LEGATO_ON           = 0x0e
LEGATO_OFF          = 0x0f
GAME_GEAR_PAN       = 0x10
AY_ENV_ON           = 0x11
AY_ENV_OFF          = 0x12
AY_ENV_SHAPE        = 0x13
AY_ENV_PERIOD_HI    = 0x14
AY_ENV_PERIOD_LO    = 0x15
AY_CHANNEL_MIX      = 0x16
AY_NOISE_PITCH      = 0x17
AY_ENV_PERIOD_WORD  = 0x18
ORDER_JUMP          = 0x19
SET_SPEED_1         = 0x1a
SET_SPEED_2         = 0x1b
ORDER_NEXT          = 0x1c
NOTE_DELAY          = 0x1d
END_LINE            = 0x80

INSTRUMENT_SIZE     = 16
FM_PATCH_SIZE       = 8

CHIP_SN76489        = 0x03      # 0x03: SMS (SN76489) - 4 channels
CHIP_SN76489_OPLL   = 0x43      # 0x43: SMS (SN76489) + OPLL (YM2413) - 13 channels (compound!)
CHIP_OPLL           = 0x89      # 0x89: OPLL (YM2413) - 9 channels
CHIP_OPLL_DRUMS     = 0xa7      # 0xa7: OPLL drums (YM2413) - 11 channels
CHIP_AY_3_8910      = 0x80      # AY-3-8910 - 3 channels

CHAN_SN76489        = 0x00
CHAN_OPLL           = 0x01
CHAN_OPLL_DRUMS     = 0x02
CHAN_AY_3_8910      = 0x03

BANJO_HAS_SN        = 0x01
BANJO_HAS_OPLL      = 0x02
BANJO_HAS_AY        = 0x04
BANJO_HAS_DUAL_SN   = 0x08
BANJO_HAS_GG        = 0x10

MACRO_TYPE_VOLUME   = 0
MACRO_TYPE_ARP      = 1
MACRO_TYPE_DUTY     = 2
MACRO_TYPE_WAVE     = 3
MACRO_TYPE_PITCH    = 4
MACRO_TYPE_EX1      = 5

macro_type_lookup = {
    0: "MACRO_TYPE_VOLUME",
    1: "MACRO_TYPE_ARP",
    2: "MACRO_TYPE_DUTY",
    3: "MACRO_TYPE_WAVE",
    4: "MACRO_TYPE_PITCH",
    5: "MACRO_TYPE_EX1",
}

# whether the drum channel needs its volume to be pre-shifted
pre_shift_fm_drum_value = [0,0,4,0,4]

def ay_pitch_fnum(note, octave):
    
    freq = pow(2.0, ((note + ((octave + 2) * 12) - 69.0)/12.0)) * 440.0
    fnum = 3579545.0 / (2 * 8 * freq)

    return int(fnum)

def ay_envelope_period(fnum, numerator, denominator):

    # calculate envelope period from the fnum
    period = ((fnum * denominator) / numerator) / 16
    period = int(period)

    # make sure period is a valid value
    if period > 0xffff:
        period = 0xffff
    elif period < 1:
        period = 1

    return period


def int_def(v, d):
    try:
        return int(v)
    except ValueError:
        return d

def main(argv=None):
    print("json2sms - Joe Kennedy 2024")

    parser = OptionParser("Usage: json2sms.py [options] INPUT_FILE_NAME.JSON")
    parser.add_option("-o", '--out',        dest='outfilename',                                      help='output file name')
    parser.add_option("-i", '--identifier', dest='identifier',                                       help='source identifier')
    parser.add_option("-s", "--sfx",        dest='sfx',                               default="-1",  help='SFX channel')

    parser.add_option("-b", '--bank',       dest='bank',                                             help='BANK number')
    parser.add_option("-a", '--area',       dest='area',                                             help='AREA name')

    parser.add_option("", '--sdas',       dest='sdas',                                action="store_true",    help='SDAS assembler compatible output')

    (options, args) = parser.parse_args()

    if (len(args) == 0):
        parser.print_help()
        parser.error("Input file name required\n")

    infilename = Path(args[0])

    sfx = (int_def(options.sfx, -1) != -1)
    sfx_channel = int_def(options.sfx, 0) if sfx else 0

    in_filename = Path(args[0])
    out_filename = Path(options.outfilename) if (options.outfilename) else infilename.with_suffix('.c')
    song_prefix = options.identifier if (options.identifier) else str(Path(infilename.name).with_suffix(''))

    # try to load json file
    try:
        infile = open(str(in_filename), "r")
        # read into variable
        json_text = infile.read()
        infile.close()
    except OSError:
        print("Error reading input file: " + str(in_filename), file=sys.stderr)
        sys.exit(1)

    # try to parse json
    try:
        song = json.loads(json_text)
    except ValueError:
        print("File is not a valid json file: " + in_filename, file=sys.stderr)
        sys.exit(1)

    # calls which will be run to init chips
    channel_init_calls = []

    # calls which will be run to mute chips
    channel_mute_calls = []

    # calls which will be run to update chips
    channel_update_calls = []

    # calls which will be run to mute the entire song
    song_mute_calls = []

    # flags to say which chips are required by the song
    song['has_chips'] = 0

    # channel types and channel numbers
    sn_count = 0
    channel_types = []

    for sound_chip in song['sound_chips']:

        if (sound_chip == CHIP_SN76489):

            # flag that we have an SN chip
            song['has_chips'] |= BANJO_HAS_SN

            # add init and mute functions to lists
            if sn_count == 0:
                channel_init_calls.append("banjo_init_sn")
                song_mute_calls.append("banjo_mute_all_sn")
            else:
                channel_init_calls.append("banjo_init_sn_" + str(sn_count+1))
                song_mute_calls.append("banjo_mute_all_sn_" + str(sn_count+1))
                # also flag that we have a second SN chip 
                song['has_chips'] |= BANJO_HAS_DUAL_SN

            for i in range(0, 4):

                channel_types.append({
                    'type': CHAN_SN76489,
                    'update': "banjo_update_channel_sn",
                    'subchannel': (i | (sn_count << 4))
                    })

                channel_mute_calls.append("banjo_mute_channel_sn")

            channel_update_calls.append("banjo_update_channels_sn")

            sn_count += 1

        elif (sound_chip == CHIP_OPLL):

            # flag that we have an OPLL chip
            song['has_chips'] |= BANJO_HAS_OPLL

            # add init and mute functions to lists
            channel_init_calls.append("banjo_init_opll")
            song_mute_calls.append("banjo_mute_all_opll")

            for i in range(0, 9):

                channel_types.append({
                    'type': CHAN_OPLL,
                    'update': "banjo_update_channel_opll",
                    'subchannel': i
                    })

                channel_mute_calls.append("banjo_mute_channel_opll")

            channel_update_calls.append("banjo_update_channels_opll")

        elif (sound_chip == CHIP_OPLL_DRUMS):

            # flag that we have an OPLL chip
            song['has_chips'] |= BANJO_HAS_OPLL

            # add init and mute functions to lists
            channel_init_calls.append("banjo_init_opll_drums")
            song_mute_calls.append("banjo_mute_all_opll_drums")

            for i in range(0, 6):

                channel_types.append({
                    'type': CHAN_OPLL,
                    'update': "banjo_update_channel_opll",
                    'subchannel': i
                    })
                
                channel_mute_calls.append("banjo_mute_channel_opll")

            for i in range(0, 5):

                channel_types.append({
                    'type': CHAN_OPLL_DRUMS,
                    'update': "banjo_update_channel_opll_drums",
                    'subchannel': i
                    })

                channel_mute_calls.append("banjo_mute_channel_opll_drums")

            channel_update_calls.append("banjo_update_channels_opll_drums")

        elif (sound_chip == CHIP_SN76489_OPLL):

            # flag that we have an SN and OPLL chip
            song['has_chips'] |= (BANJO_HAS_SN | BANJO_HAS_OPLL)

            # add init and mute functions to lists
            channel_init_calls.append("banjo_init_sn")
            channel_init_calls.append("banjo_init_opll")
            song_mute_calls.append("banjo_mute_all_sn")
            song_mute_calls.append("banjo_mute_all_opll")

            for i in range(0, 4):

                channel_types.append({
                    'type': CHAN_SN76489,
                    'update': "banjo_update_channel_sn",
                    'subchannel': (i | (sn_count << 4))
                    })

                channel_mute_calls.append("banjo_mute_channel_sn")

            for i in range(0, 9):

                channel_types.append({
                    'type': CHAN_OPLL,
                    'update': "banjo_update_channel_opll",
                    'subchannel': i
                    })

                channel_mute_calls.append("banjo_mute_channel_opll")

            channel_update_calls.append("banjo_update_channels_sn")
            channel_update_calls.append("banjo_update_channels_opll")

            sn_count += 1

        elif (sound_chip == CHIP_AY_3_8910):

            # flag that we have an AY chip
            song['has_chips'] |= BANJO_HAS_AY

            # add init and mute functions to lists
            channel_init_calls.append("banjo_init_ay")
            song_mute_calls.append("banjo_mute_all_ay")
            
            for i in range(0, 3):

                channel_types.append({
                    'type': CHAN_AY_3_8910,
                    'update': "banjo_update_channel_ay",
                    'subchannel': i
                    })

                channel_mute_calls.append("banjo_mute_channel_ay")

            channel_update_calls.append("banjo_update_channels_ay")

    # in sfx mode, only include channel_types for the specified sfx channel
    if (sfx):

        channel_types = [channel_types[sfx_channel]]
        sfx_subchannel = channel_types[0]["subchannel"]
        sfx_type = channel_types[0]["type"]

        channel_update_calls = [channel_types[0]["update"]]

        if sfx_type == CHAN_OPLL:
            channel_init_calls = ["banjo_init_sfx_channel_opll"]
            song_mute_calls = ["banjo_mute_all_opll"]

        elif sfx_type == CHAN_SN76489:
            channel_init_calls = ["banjo_init_sfx_channel_sn"]
            song_mute_calls = ["banjo_mute_all_sn"]

        elif sfx_type == CHAN_AY_3_8910:
            channel_init_calls = ["banjo_init_sfx_channel_ay"]
            song_mute_calls = ["banjo_mute_all_ay"]

    # process instruments
    instruments = []

    macros = {}
    fm_patches = {}

    for i in range(0, len(song['instruments'])):

        instrument = song['instruments'][i]

        new_instrument = {
            "volume_macro_len": 0,
            "volume_macro_loop": 0xff,
            "volume_macro_ptr": 0xffff,

            "ex_macro_type": 0,
            "ex_macro_len": 0,
            "ex_macro_loop": 0xff,
            "ex_macro_ptr": 0xffff,

            "fm_patch": 0xff,
            "fm_patch_ptr": 0xffff,
        }

        # macros
        for macro in instrument["macros"]:

            if macro["loop"] != 0xff and macro["loop"] >= macro["length"]:
                macro["loop"] = 0xff

            # volume macro
            if macro["code"] == MACRO_TYPE_VOLUME:

                new_instrument["volume_macro_len"] = macro["length"]
                new_instrument["volume_macro_loop"] = macro["loop"]

                for j in range(0, macro["length"]):                     
                    macro['data'][j] = 15 - macro['data'][j]

                macro_data_name = "macro_volume_" + str(i)
                macros[macro_data_name] = macro['data']
                new_instrument["volume_macro_ptr"] = macro_data_name

            # currently no ex macro for this instrument
            elif new_instrument["ex_macro_ptr"] == 0xffff:

                new_instrument["ex_macro_type"] = macro_type_lookup[macro["code"]] if macro["code"] in macro_type_lookup else macro["code"]
                new_instrument["ex_macro_len"] = macro["length"]
                new_instrument["ex_macro_loop"] = macro["loop"]

                if macro["code"] == MACRO_TYPE_ARP:
                    for j in range(0, macro["length"]):
                        val = macro['data'][j]

                        # get the absolute/relative flag in bit 7
                        absolute = ((abs(val) >> 23) & 0x80)
                        
                        # constrain the arp amount to 7 bits
                        macro['data'][j] = (macro['data'][j] & 0x7f) | absolute

                # need to invert pitch offset for certain chips
                if macro["code"] == MACRO_TYPE_PITCH and (instrument["type"] == 0 or instrument["type"] == 6):
                    for j in range(0, macro["length"]):
                        macro['data'][j] = -macro['data'][j]

                # need to pre-shift fm patch for opll patch macro
                if macro["code"] == MACRO_TYPE_WAVE and instrument["type"] == 13:
                    for j in range(0, macro["length"]):
                        macro['data'][j] = macro['data'][j] << 4

                # need to invert duty for ay noise freq macro
                if macro["code"] == MACRO_TYPE_DUTY and instrument["type"] == 6:
                    for j in range(0, macro["length"]):
                        macro['data'][j] = 0x1f - macro['data'][j]

                # special case for sn noise duty, similar to SN_NOISE_MODE command
                if macro["code"] == MACRO_TYPE_DUTY and instrument["type"] == 0:
                    for j in range(0, macro["length"]):
                        duty = macro['data'][j]
                        noise_mode = (duty & 0x1) << 2
                        noise_freq = 0x3 if (duty & 0x2) else 0x0
                        macro['data'][j] = 0x80 | (0x3 << 5) | noise_mode | noise_freq

                macro_data_name = "macro_ex_" + str(i)
                macros[macro_data_name] = macro['data']
                new_instrument["ex_macro_ptr"] = macro_data_name


        # FM preset change (only for OPLL FM instruments)
        if (instrument['type'] == 13 and "fm" in instrument):

            # custom fm patch data
            if (instrument['fm']['opll_patch'] == 0):

                # pre-shift the patch number into the upper nibble
                new_instrument["fm_patch"] = (instrument['fm']['opll_patch'] & 0xf) << 4

                fm_patch = [0, 0, 0, 0, 0, 0, 0, 0]
                operator = False

                for j in range(0, 2):

                    operator = instrument['fm']['operator_data'][j]

                    # operator sustain seems to be in top bit of ssg in furnace format
                    fm_patch[0 + j] =   operator['mult'] | \
                                        (operator['ksr'] << 4) | \
                                        (((operator['ssg'] >> 3) & 0x1) << 5) | \
                                        (operator['vib'] << 6) | \
                                        (operator['am'] << 7)


                operator = instrument['fm']['operator_data'][0]
                fm_patch[2] = operator['tl'] | (operator['ksl'] << 6)

                # op 1 and 2 half-sine modes seem to be in instrument's fms and ams in furnace format
                operator = instrument['fm']['operator_data'][1]
                fm_patch[3] =   (instrument['fm']['feedback']) | \
                                (instrument['fm']['ams'] << 3) | \
                                (instrument['fm']['fms'] << 4) | \
                                (operator['ksl'] << 6)

                for j in range(0, 2):

                    operator = instrument['fm']['operator_data'][j]
                    fm_patch[4 + j] = operator['dr'] | (operator['ar'] << 4)
                    fm_patch[6 + j] = operator['rr'] | (operator['sl'] << 4)


                fm_patch_name = "fm_patch_" + str(i)
                fm_patches[fm_patch_name] = fm_patch
                new_instrument["fm_patch_ptr"] = fm_patch_name


            # custom drum patch with fixed pitch
            elif (instrument['fm']['opll_patch'] == 16):

                # pre-shift the patch number into the upper nibble
                new_instrument["fm_patch"] = (0xff if instrument['opl_drums']['fixed_freq'] else 0)

                fm_patch = []

                fm_patch.append(instrument['opl_drums']['kick_freq'] & 0xff)
                fm_patch.append(instrument['opl_drums']['kick_freq'] >> 8)

                fm_patch.append(instrument['opl_drums']['snare_hat_freq'] & 0xff)
                fm_patch.append(instrument['opl_drums']['snare_hat_freq'] >> 8)

                fm_patch.append(instrument['opl_drums']['tom_top_freq'] & 0xff)
                fm_patch.append(instrument['opl_drums']['tom_top_freq'] >> 8)

                fm_patch_name = "fm_patch_" + str(i)
                fm_patches[fm_patch_name] = fm_patch
                new_instrument["fm_patch_ptr"] = fm_patch_name

            else:

                # pre-shift the patch number into the upper nibble
                new_instrument["fm_patch"] = (instrument['fm']['opll_patch'] & 0xf) << 4

        instruments.append(new_instrument)

    # process patterns
    patterns = {}

    # process patterns
    # go through each channel
    for i in range(0, song['channel_count']):

        # in sfx mode, only process sfx channel
        if (sfx and i != sfx_channel):

            continue

        # only one channel for sfx
        if (sfx):

            channel_type = channel_types[0]

        else:

            channel_type = channel_types[i]

        channel_subchannel = channel_type['subchannel']
        
        # separate pattern arrays per channel
        patterns[i] = {}

        # go through each pattern in the channel
        for j in range (0, len(song['patterns'][i])):

            # keep track of the current instrument
            # so we don't have multiple INSTRUMENT_CHANGEs per note
            # when the instrument is exactly the same
            # reset at a pattern level as patterns may appear out of order
            # or be jumped to
            current_inst = -1
            current_vol = -1

            pattern = song['patterns'][i][j]

            pattern_bin = []

            if (pattern):

                last_note = { "note": -1, "octave": -1 }
                ay_envelope_auto = False

                # go through every line
                for k in range (0, song['pattern_length']):

                    line = pattern['data'][k]

                    note = { "note": line['note'], "octave": line['octave'] }

                    # instrument has changed
                    if (current_inst != line['instrument'] and line['instrument'] != -1):

                        current_inst = line['instrument']

                        # ensure the instrument exists so we don't load
                        # other data as an instrument
                        if current_inst < len(instruments):

                            if (channel_type['type'] == CHAN_OPLL_DRUMS):

                                pattern_bin.append(FM_DRUM)
                                pattern_bin.append(current_inst)

                            else:

                                pattern_bin.append(INSTRUMENT_CHANGE)
                                pattern_bin.append(current_inst)

                    # check effects
                    for eff in range (0, len(line['effects']), 2):

                        # SN noise mode
                        if (line['effects'][eff] == 0x20 and channel_type['type'] == CHAN_SN76489):

                            pattern_bin.append(SN_NOISE_MODE)

                            noise_mode = (line['effects'][eff + 1] & 0x1) << 2
                            noise_freq = 0x3 if (line['effects'][eff + 1] & 0x10) else 0x0

                            pattern_bin.append(0x80 | (0x3 << 5) | noise_mode | noise_freq)

                        # Arpeggio
                        elif (line['effects'][eff] == 0x00):

                            if (line['effects'][eff + 1] != 0):
                                pattern_bin.append(ARPEGGIO)
                                pattern_bin.append(line['effects'][eff + 1])
                            else:
                                pattern_bin.append(ARPEGGIO_OFF)


                        # Pitch slide up
                        elif (line['effects'][eff] == 0x01):

                            if (line['effects'][eff + 1] != 0):
                                pattern_bin.append(SLIDE_UP)
                                pattern_bin.append(line['effects'][eff + 1])
                            else:
                                pattern_bin.append(SLIDE_OFF)


                        # Pitch slide down
                        elif (line['effects'][eff] == 0x02):

                            if (line['effects'][eff + 1] != 0):
                                pattern_bin.append(SLIDE_DOWN)
                                pattern_bin.append(line['effects'][eff + 1])
                            else:
                                pattern_bin.append(SLIDE_OFF)


                        # Portamento
                        elif (line['effects'][eff] == 0x03):

                            if (line['effects'][eff + 1] != 0):
                                pattern_bin.append(SLIDE_PORTA)
                                pattern_bin.append(line['effects'][eff + 1])
                            else:
                                pattern_bin.append(SLIDE_OFF)

                        # Vibrato
                        elif (line['effects'][eff] == 0x04):

                            vibrato_speed = line['effects'][eff + 1] >> 4
                            vibrato_amount = line['effects'][eff + 1] & 0xf

                            if (vibrato_speed == 0 and vibrato_amount == 0):
                                pattern_bin.append(VIBRATO_OFF)

                            else:
                                pattern_bin.append(VIBRATO)
                                pattern_bin.append(vibrato_speed)
                                pattern_bin.append(vibrato_amount << 4)

                        # legato
                        elif (line['effects'][eff] == 0xea):

                            if (line['effects'][eff + 1] > 0):

                                pattern_bin.append(LEGATO_ON)

                            else:

                                pattern_bin.append(LEGATO_OFF)

                        # order jump
                        elif (line['effects'][eff] == 0x0b):
                            
                            pattern_bin.append(ORDER_JUMP)
                            pattern_bin.append(line['effects'][eff + 1])

                        # order next
                        elif (line['effects'][eff] == 0x0d):
                            
                            pattern_bin.append(ORDER_NEXT)

                        # set speed 1
                        elif (line['effects'][eff] == 0x09):
                            
                            pattern_bin.append(SET_SPEED_1)
                            pattern_bin.append(line['effects'][eff + 1])

                        # set speed 2
                        elif (line['effects'][eff] == 0x0f):
                            
                            pattern_bin.append(SET_SPEED_2)
                            pattern_bin.append(line['effects'][eff + 1])

                        # note delay
                        elif (line['effects'][eff] == 0xed):

                            pattern_bin.append(NOTE_DELAY)
                            pattern_bin.append(line['effects'][eff + 1])

                        # sn modes
                        if channel_type['type'] == CHAN_SN76489:

                            # Panning
                            if (line['effects'][eff] == 0x80):

                                pattern_bin.append(GAME_GEAR_PAN)

                                pan_left = (0 if (line['effects'][eff + 1] > 0x80) else 1) << (channel_subchannel + 4)
                                pan_right = (0 if (line['effects'][eff + 1] < 0x80) else 1) << channel_subchannel

                                pan_mask = (~(1 << channel_subchannel)) & 0xf

                                pattern_bin.append(pan_left | pan_right)
                                pattern_bin.append(pan_mask | (pan_mask << 4))

                            # Panning
                            if (line['effects'][eff] == 0x08):

                                pattern_bin.append(GAME_GEAR_PAN)

                                pan_left = (1 if (line['effects'][eff + 1] & 0xf0) else 0) << (channel_subchannel + 4)
                                pan_right = (1 if (line['effects'][eff + 1] < 0x0f) else 0) << channel_subchannel

                                pan_mask = (~(1 << channel_subchannel)) & 0xf

                                pattern_bin.append(pan_left | pan_right)
                                pattern_bin.append(pan_mask | (pan_mask << 4))


                        # ay3 modes
                        if channel_type['type'] == CHAN_AY_3_8910:

                            if line['effects'][eff] == 0x20:

                                # by default both noise and square enabled
                                ay_bits = 0b00000000

                                ay_mode = line['effects'][eff + 1]

                                if ay_mode < 7:
                                    
                                    if ay_mode >= 3:
                                        pattern_bin.append(AY_ENV_ON)
                                    else:
                                        pattern_bin.append(AY_ENV_OFF)
                                    
                                    # square only
                                    if ay_mode == 0 or ay_mode == 4:
                                        ay_bits = 0b00001000

                                    # noise only
                                    elif ay_mode == 1 or ay_mode == 5:
                                        ay_bits = 0b00000001
                                        
                                    pattern_bin.append(AY_CHANNEL_MIX)
                                    pattern_bin.append(ay_bits << channel_subchannel)

                                # square, noise and env off
                                elif ay_mode == 7:

                                    ay_bits = 0b00001001

                                    pattern_bin.append(AY_ENV_OFF)
                                    pattern_bin.append(AY_CHANNEL_MIX)
                                    pattern_bin.append(ay_bits << channel_subchannel)

                            # ay envelope shape/enable for channel
                            elif line['effects'][eff] == 0x22:

                                pattern_bin.append(AY_ENV_SHAPE)
                                pattern_bin.append(line['effects'][eff + 1] >> 4)

                                # envelope on or off for this channel?
                                if (line['effects'][eff + 1] & 0xf) > 0:
                                    pattern_bin.append(AY_ENV_ON)
                                else:
                                    pattern_bin.append(AY_ENV_OFF)

                            # ay envelope hi period byte
                            elif line['effects'][eff] == 0x24:

                                pattern_bin.append(AY_ENV_PERIOD_HI)
                                pattern_bin.append(line['effects'][eff + 1])

                            # ay envelope lo period byte
                            elif line['effects'][eff] == 0x23:

                                pattern_bin.append(AY_ENV_PERIOD_LO)
                                pattern_bin.append(line['effects'][eff + 1])

                            # ay envelope lo period byte
                            elif line['effects'][eff] == 0x21:
                                
                                pattern_bin.append(AY_NOISE_PITCH)
                                pattern_bin.append(0x1f - line['effects'][eff + 1])

                            # ay envelope auto
                            elif line['effects'][eff] == 0x29:

                                # get effect numerator and denominator
                                numerator = (line['effects'][eff + 1] >> 4) & 0xf
                                denominator = line['effects'][eff + 1] & 0xf

                                # check if we're switching the effect off
                                if numerator == 0 or denominator == 0:

                                    ay_envelope_auto = False

                                # we're applying the effect
                                else:

                                    # keep the numerator and denominator around
                                    ay_envelope_auto = { "numerator": numerator, "denominator": denominator}

                                    # use fnum of new note
                                    if note["note"] != -1 and note["octave"] != -1:

                                        fnum = ay_pitch_fnum(note["note"], note["octave"])

                                    # use fnum of last note
                                    elif last_note["note"] != -1 and last_note["octave"] != -1:

                                        fnum = ay_pitch_fnum(last_note["note"], last_note["octave"])

                                    # get period
                                    period = ay_envelope_period(fnum, numerator, denominator)

                                    pattern_bin.append(AY_ENV_PERIOD_WORD)
                                    pattern_bin.append(period & 0xff)
                                    pattern_bin.append((period >> 8) & 0xff)

                    # volume
                    # if the volume has been specified on the line, or we haven't provided a volume command yet
                    if (line['volume'] != -1):

                        if (line['volume'] != -1):
                            volume = line['volume']

                        # volume has changed
                        if volume != current_vol:

                            pattern_bin.append(VOLUME_CHANGE)

                            # sn channel, this byte can be sent straight to the output to set the volume
                            if (channel_type['type'] == CHAN_SN76489):

                                # cap volume
                                if volume > 15:
                                    volume = 15

                                pattern_bin.append(15 - volume)

                            elif (channel_type['type'] == CHAN_OPLL):

                                # cap volume
                                if volume > 15:
                                    volume = 15
                                    
                                pattern_bin.append((15 - volume))

                            elif (channel_type['type'] == CHAN_OPLL_DRUMS):

                                # cap volume
                                if volume > 15:
                                    volume = 15

                                pattern_bin.append((15 - volume) << pre_shift_fm_drum_value[channel_subchannel])

                            elif (channel_type['type'] == CHAN_AY_3_8910):
                                
                                # cap volume
                                if volume > 15:
                                    volume = 15

                                pattern_bin.append(volume)

                        current_vol = volume

                    # empty
                    if (line['note'] == -1 and line['octave'] == -1):

                        False

                    # note on
                    elif (line['note'] >= 0 and line['note'] < 12):

                        pattern_bin.append(NOTE_ON)

                        # note number
                        pattern_bin.append((note['note'] + (note['octave'] * 12)) & 0x7f)

                        # store in last_note
                        last_note = note

                        # apply ay envelope auto if it's enabled
                        if ay_envelope_auto != False:

                            fnum = ay_pitch_fnum(last_note["note"], last_note["octave"])
                            
                            period = ay_envelope_period(fnum, ay_envelope_auto["numerator"], ay_envelope_auto["denominator"])

                            pattern_bin.append(AY_ENV_PERIOD_WORD)
                            pattern_bin.append(period & 0xff)
                            pattern_bin.append((period >> 8) & 0xff)


                    # note off
                    elif (line['note'] == 100 or line['note'] == 101):

                        pattern_bin.append(NOTE_OFF)


                    # end line marker
                    pattern_bin.append(END_LINE)


                # when a run of END_LINE ends, replace it if it's long enough
                def run_end(run_length, pattern_bin_rle):

                    if (run_length > 0):
                        pattern_bin_rle.append("(" + str(END_LINE) + " | " + str(run_length - 1) + ")")


                # run length encoding for END_LINE
                run_length = 0
                pattern_bin_rle = []


                # look through pattern_bin for runs of END_LINE
                for k in range(0, len(pattern_bin)):

                    if (pattern_bin[k] == END_LINE):

                        run_length += 1

                    else:

                        run_end(run_length, pattern_bin_rle)
                        run_length = 0
                        pattern_bin_rle.append(pattern_bin[k])

                run_end(run_length, pattern_bin_rle)

                pattern_bin = pattern_bin_rle

                # add to patterns array
                patterns[i][pattern["index"]] = pattern_bin

    # output asm/h file
    outfile = open(str(out_filename), "w")

    def writelabel(label):

        outfile.write(song_prefix + "_" + label + ":" + "\n")

    bank_number = str(options.bank) if (options.bank) else "0"

    # sdas/sdcc mode
    if (options.sdas):

        # prefix song name with an underscore so it's accessible in C
        song_prefix = "_" + song_prefix

        # prefix asm calls song name with an underscore
        channel_init_calls = list(map(lambda call: "_" + call, channel_init_calls))
        channel_update_calls = list(map(lambda call: "_" + call, channel_update_calls))
        channel_mute_calls = list(map(lambda call: "_" + call, channel_mute_calls))
        song_mute_calls = list(map(lambda call: "_" + call, song_mute_calls))

        # set the module for this file so it flags up the file name in any errors
        outfile.write(".module " + song_prefix + "\n")

        # export the song label so it's accessible in C
        outfile.write(".globl " + song_prefix + "\n")

        # area specified
        if (options.area):

            # postfix the area name with the bank number if available
            outfile.write(".area _" + options.area + (str(options.bank) if options.bank else "") + "\n")

        # bank specified
        if (options.bank):
            
            # used for gbdk autobanking
            outfile.write("___bank" + song_prefix + " .equ " + str(options.bank) + "\n")
            outfile.write(".globl ___bank" + song_prefix + "\n")
            bank_number = "___bank" + song_prefix

        outfile.write("\n")

    # initial label
    outfile.write(song_prefix + ":" + "\n")

    # basic song data
    writelabel("magic_byte")
    outfile.write(".db " + str(MAGIC_BYTE) + "\n")
    writelabel("bank")
    outfile.write(".db " + bank_number + "\n")
    writelabel("channel_count")
    outfile.write(".db " + str(1 if sfx else song['channel_count']) + "\n")
    writelabel("flags")
    outfile.write(".db " + str(0 if sfx else 2) + "\n")
    writelabel("master_volume")
    outfile.write(".db 0x80\n")
    writelabel("master_volume_fade")
    outfile.write(".db 0x00\n")
    writelabel("has_chips")
    outfile.write(".db " + str(song['has_chips']) + "\n")
    writelabel("sfx_channel")
    outfile.write(".db " + str(sfx_channel if sfx else 0xff) + "\n")
    writelabel("sfx_subchannel")
    outfile.write(".db " + str(sfx_subchannel if sfx else 0xff) + "\n")
    writelabel("time_base")
    outfile.write(".db " + str(song['time_base'] + 1) + "\n")
    writelabel("speed_1")
    outfile.write(".db " + str(song['speed_1']) + "\n")
    writelabel("speed_2")
    outfile.write(".db " + str(song['speed_2']) + "\n")
    writelabel("pattern_length")
    outfile.write(".db " + str(song['pattern_length']) + "\n")
    writelabel("orders_length")
    outfile.write(".db " + str(song['orders_length']) + "\n")
    writelabel("instrument_ptrs")
    outfile.write(".dw " + song_prefix + "_instrument_pointers" + "\n")
    writelabel("order_ptrs")
    outfile.write(".dw " + song_prefix + "_order_pointers" + "\n")
    writelabel("subtic")
    outfile.write(".db 0\n")
    writelabel("tic")
    outfile.write(".db 0\n")
    writelabel("line")
    outfile.write(".db 0\n")
    writelabel("order")
    outfile.write(".db 0\n")
    writelabel("order_jump")
    outfile.write(".db 0xff\n")
    writelabel("noise_mode")
    outfile.write(".db 0\n")
    writelabel("panning")
    outfile.write(".db 0xff\n")

    # when there's multiple channel init calls a function will be created
    # if there's only one we just point to that
    writelabel("channel_init")
    if len(channel_init_calls) > 1:
        outfile.write(".dw " + song_prefix + "_channel_init_call" + "\n")
    else:
        outfile.write(".dw " + channel_init_calls[0] + "\n")    
        
    # when there's multiple channel update calls a function will be created
    # if there's only one we just point to that
    writelabel("channel_update")
    if len(channel_update_calls) > 1:
        outfile.write(".dw " + song_prefix + "_channel_update_call\n")
    else:
        outfile.write(".dw " + channel_update_calls[0] + "\n")    

    # array of calls to the mute function for each channel
    writelabel("channel_mute")
    outfile.write(".dw " + song_prefix + "_channel_mute_calls_data\n")

    # when there's multiple song mute calls a function will be created
    # if there's only one we just point to that
    writelabel("song_mute")
    if len(song_mute_calls) > 1:
        outfile.write(".dw " + song_prefix + "_song_mute_call\n")
    else:
        outfile.write(".dw " + song_mute_calls[0] + "\n")    

    writelabel("song_stop")
    outfile.write(".dw " + ("_" if options.sdas else "") + ("banjo_sfx_stop" if sfx else "banjo_song_stop") + "\n")
    outfile.write("\n" + "\n")

    # channel init calls
    if len(channel_init_calls) > 1:
        writelabel("channel_init_call")
        for call in channel_init_calls:
            outfile.write("\tcall " + call + "\n")
        outfile.write("\tret\n")
        outfile.write("\n")

    # channel update calls
    if len(channel_update_calls) > 1:
        writelabel("channel_update_call")
        for call in channel_update_calls:
            outfile.write("\tcall " + call + "\n")
        outfile.write("\tret\n")
        outfile.write("\n")

    # channel mute calls
    writelabel("channel_mute_calls_data")
    outfile.write("\t.dw " + ", ".join(channel_mute_calls) + "\n")
    outfile.write("\n")

    # song mute calls
    if len(song_mute_calls) > 1:
        writelabel("song_mute_call")
        for call in song_mute_calls:
            outfile.write("\tcall " + call + "\n")
        outfile.write("\tret\n")
        outfile.write("\n")

    # macros
    writelabel("macros")

    for key in macros:

        writelabel(key)
        outfile.write(".db " + ", ".join(map(str, macros[key])) + "\n")

    outfile.write("\n" + "\n")

    # fm patches
    writelabel("fm_patches")

    print(fm_patches)

    for key in fm_patches:

        writelabel(key)
        outfile.write(".db " + ", ".join(map(str, fm_patches[key])) + "\n")

    outfile.write("\n" + "\n")

    # instruments
    writelabel("instrument_pointers")

    for i in range (0, len(instruments)):

        outfile.write(".dw " + song_prefix + "_instrument_" + str(i) + "\n")

    outfile.write("\n" + "\n")

    # instruments
    writelabel("instrument_data")

    for i in range (0, len(instruments)):

        instrument = instruments[i]

        writelabel("instrument_" + str(i))
        outfile.write("\t.db " + str(instrument["volume_macro_len"]) + " ; len\n")
        outfile.write("\t.db " + str(instrument["volume_macro_loop"]) + " ; loop\n")

        if (instrument["volume_macro_ptr"] == 0xffff):
            outfile.write("\t.dw 0xffff ;ptr\n")
        else:
            outfile.write("\t.dw " + song_prefix + "_" + str(instrument["volume_macro_ptr"]) + " ; ptr\n")

        # ex macro
        outfile.write("\t.db " + str(instrument["ex_macro_type"]) + " ; type\n")
        outfile.write("\t.db " + str(instrument["ex_macro_len"]) + " ; len \n")
        outfile.write("\t.db " + str(instrument["ex_macro_loop"]) + " ; loop\n")

        if (instrument["ex_macro_ptr"] == 0xffff):
            outfile.write("\t.dw 0xffff ;ptr\n")
        else:
            outfile.write("\t.dw " + song_prefix + "_" + str(instrument["ex_macro_ptr"]) + " ; ptr\n")

        # fm preset
        outfile.write("\t.db " + str(instrument["fm_patch"]) + "\n")

        # fm patch
        if (instrument["fm_patch_ptr"] == 0xffff):
            outfile.write("\t.dw 0xffff ;ptr\n")
        else:
            outfile.write("\t.dw " + song_prefix + "_" + str(instrument["fm_patch_ptr"]) + " ; ptr\n")



    outfile.write("\n" + "\n")


    # order pointers
    writelabel("order_pointers")

    for i in range (0, song['channel_count']):

        # in sfx mode, only process sfx channel
        if (sfx and i != sfx_channel):

            continue


        outfile.write("\t.dw " + song_prefix + "_orders_channel_" + str(i) + "\n")


    # orders
    writelabel("orders")

    for i in range (0, song['channel_count']):

        # in sfx mode, only process sfx channel
        if (sfx and i != sfx_channel):

            continue


        writelabel("orders_channel_" + str(i))

        for j in range (0, len(song['orders'][i])):

            if (j % 4 == 0):

                outfile.write("\n\t.dw ")


            outfile.write(song_prefix + "_patterns_" + str(i) + "_" + str(song['orders'][i][j]))

            if ((j % 4 != 3) and (j != len(song['orders'][i]) - 1)):

                outfile.write(", ")


        outfile.write("\n")


    outfile.write("\n" + "\n")

    # patterns
    writelabel("patterns")

    for i in range (0, song['channel_count']):

        # in sfx mode, only process sfx channelfs.
        if (sfx and i != sfx_channel):

            continue

        for j in range (0, len(song["patterns"][i])):

            pattern_index = song["patterns"][i][j]["index"]

            writelabel("patterns_" + str(i) + "_" + str(pattern_index))
            outfile.write(".db " + ", ".join(map(str, patterns[i][pattern_index])) + "\n")

    #outfile.write("\n\n.rept 10000\n.db 0xba\n.endm\n")

    # close file
    outfile.close()

if __name__=='__main__':
    main()
