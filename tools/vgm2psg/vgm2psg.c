#include <stdio.h>
#include <stdlib.h>
#include <zlib.h>
#include <string.h>

#define     FALSE                   0
#define     TRUE                    1

#define     VGM_OLD_HEADERSIZE      64        // 'old' VGM header size
#define     VGM_BIG_HEADERSIZE      256       // 'big' VGM header size
#define     VGM_HEADER_LOOPPOINT    0x1C
#define     VGM_HEADER_FRAMERATE    0x24
#define     VGM_DATA_OFFSET         0x34

#define     VGM_GGSTEREO    0x4F
#define     VGM_PSGFOLLOWS  0x50
#define     VGM_FMFOLLOWS   0x51
#define     VGM_FRAMESKIP_NTSC   0x62
#define     VGM_FRAMESKIP_PAL    0x63
#define     VGM_SAMPLESKIP  0x61
#define     VGM_ENDOFDATA   0x66

#define     MAX_WAIT        7                 // fits in 3 bits only

#define     PSG_ENDOFDATA   0x00
#define     PSG_LOOPMARKER  0x01
#define     PSG_WAIT        0x38

#define     CHANNELS        4

unsigned int loop_offset;
unsigned int data_offset;
gzFile fIN;
FILE *fOUT;

unsigned char volume[CHANNELS]={0x0F,0x0F,0x0F,0x0F};  // starting volume = silence
unsigned short freq[CHANNELS]={0,0,0,0};
int volume_change[CHANNELS]={FALSE,FALSE,FALSE,FALSE};
int freq_change[CHANNELS]={FALSE,FALSE,FALSE,FALSE};
int hi_freq_change[CHANNELS]={FALSE,FALSE,FALSE,FALSE};
int frame_started=TRUE;
int pause_started=FALSE;
int pause_len=0;
unsigned char lastlatch=0x9F;   // latch volume silent on channel 0
int active[CHANNELS]={FALSE,FALSE,FALSE,FALSE};
int is_sfx=FALSE;
int warn_32=FALSE;

void decLoopOffset(int n) {
  loop_offset-=n;
}

void incLoopOffset(void) {
  loop_offset++;
}

int checkLoopOffset(void) {        // returns 1 when loop_offset becomes 0
  return (loop_offset==0);
}

void init_frame(int initial_state) {
  int i;
  for (i=0;i<CHANNELS;i++) {
    volume_change[i]=FALSE;
    freq_change[i]=FALSE;
    hi_freq_change[i]=FALSE;
  }
  frame_started=initial_state;
}

void add_command (unsigned char c) {
  int chn,typ;
  if (c&0x80) {           // it's a latch
    chn=(c&0x60)>>5;
    typ=(c&0x10)>>4;
    if (typ==1) {
      if (volume[chn]!=(c&0x0F)) {        // see if we're really changing the volume or not
        volume[chn]=c&0x0F;
        volume_change[chn]=TRUE;
      }
    } else {
      if ((chn==3) || ((freq[chn]&0x0F)!=(c&0x0F))) {   // see if we're really changing the low part of the frequency or not (saving noise channel retrigs!)
        freq[chn]=(freq[chn]&0xFFF0)|(c&0x0F);
        freq_change[chn]=TRUE;
      }

      if ((chn==3) && (is_sfx) && (active[3]) && (!active[2]) && ((c&0x3)==0x3) && (!warn_32)) {
        printf("Warning: channel 3 (the noise channel) is using channel 2 tone. You probably need to run vgm2psg including channel 2 too.\n");
        warn_32=TRUE;
      }

    }
  } else {                // it's a data (not a latch)
    chn=(lastlatch&0x60)>>5;
    typ=(lastlatch&0x10)>>4;
    if (typ==1) {
      if (volume[chn]!=(c&0x0F)) {        // see if we're really changing the volume or not
        volume[chn]=c&0x0F;
        volume_change[chn]=TRUE;
      }
    } else {
      if ((c&0x3F)!=(freq[chn]>>4)) {     // see if we're really changing the high part of the frequency or not
        freq[chn]=(freq[chn]&0x000F)|((c&0x3F)<<4);
        hi_freq_change[chn]=TRUE;
        freq_change[chn]=TRUE;            // to update the high part of the frequency we need to update the low part too, there's no other way
      }
    }
  }
}

void dump_frame(void) {
  int i;
  unsigned char c;
  for (i=0;i<CHANNELS-1;i++) {
    if (freq_change[i]) {
      c=(freq[i]&0x0F)|(i<<5)|0x80;          // latch channel 0-2 freq
      fputc(c,fOUT);
      if (hi_freq_change[i]) {               // DATA byte needed?
        c=(freq[i]>>4)|0x40;                 // make sure DATA bytes have 1 as 6th bit
        fputc(c,fOUT);
      }
    }

    if (volume_change[i]) {
      c=0x90|(i<<5)|(volume[i]&0x0F);        // latch channel 0-2 volume
      fputc(c,fOUT);
    }

  }

  if (freq_change[3]) {
    c=(freq[i]&0x07)|0xE0;                   // latch channel 3 (noise)
    fputc(c,fOUT);
  }

  if (volume_change[3]) {
    c=0x90|(i<<5)|(volume[3]&0x0F);          // latch channel 3 volume
    fputc(c,fOUT);
  }
}


void dump_pause(void) {
  if (pause_len>0) {
    while (pause_len>MAX_WAIT) {
          fputc(PSG_WAIT+MAX_WAIT,fOUT);   // write PSG_WAIT+7 to file
          pause_len-=MAX_WAIT+1;           // skip MAX_WAIT+1
    }
    if (pause_len>0)
      fputc(PSG_WAIT+(pause_len-1),fOUT);  // write PSG_WAIT+[0 to 7] to file, don't do it if 0
  }
}

void found_pause(void) {
  if (frame_started) {
    dump_frame();
    init_frame(FALSE);
  }
  pause_started=TRUE;
}

void found_frame(void) {
  if (pause_started)
    dump_pause();
  frame_started=TRUE;

  pause_started=FALSE;
  pause_len=0;
}

void empty_data(void) {
  if (pause_started) {
    dump_pause();
    pause_len=0;
    pause_started=FALSE;
  } else if (frame_started) {
    dump_frame();
    init_frame(FALSE);
  }
}


void writeLoopMarker(void) {
  empty_data();
  fputc(PSG_LOOPMARKER,fOUT);
}


int main (int argc, char *argv[]) {

  unsigned int i;
  int c;
  int leave=0;
  int fatal=0;
  int ss,fs;
  int latched_chn=0;
  int first_byte=TRUE;
  unsigned int file_signature,frame_rate;
  int sample_divider=735;                            // NTSC (default)

  printf ("*** sverx's VGM to PSG converter ***\n");

  if (argc>4)
    printf ("Fatal: too many parameters specified. Three parameters at max are allowed.\n");

  if (argc<3)
    printf ("Fatal: too few parameters specified. At least two parameters are required.\n");

  if ((argc<3) || (argc>4)) {
    printf ("Usage: vgm2psg inputfile.VGM outputfile.PSG [[0][1][2][3]]\n");
    printf (" [optional] when converting SFXs, the third parameter specifies which channel(s) should be active, examples:\n");
    printf ("   0 means the SFX is using channel 0 only\n");
    printf ("   1 means the SFX is using channel 1 only\n");
    printf ("   2 means the SFX is using channel 2 only\n");
    printf ("   3 means the SFX is using channel 3 (noise) only\n");
    printf ("  23 means the SFX is using both channel 2 and channel 3 (noise)\n");
    printf (" 123 means the SFX is using channels 1 and 2 and channel 3 (noise)\n");
    return (1);
  }

  if (argc==4) {
    for (i=0;i<CHANNELS;i++)
      active[i]=0;

    for (i=0;i<strlen(argv[3]);i++) {
      switch (argv[3][i]) {
        case '0':
          active[0]=1;
          break;
        case '1':
          active[1]=1;
          break;
        case '2':
          active[2]=1;
          break;
        case '3':
          active[3]=1;
          break;
        default:
         printf ("Fatal: the optional third parameter can only contains digits 0 to 3\n");
         return (1);
      }
    }

    if (!(active[0] && active[1] && active[2] && active[3])) {
      is_sfx=TRUE;
      printf ("Info: SFX conversion on channel(s): %s%s%s%s\n",active[0]?"0":"",active[1]?"1":"",active[2]?"2":"",active[3]?"3":"");
    }
  }

  init_frame(TRUE);

  fIN=gzopen(argv[1],"rb");
  if (!fIN) {
    printf("Fatal: can't open input VGM file\n");
    return(1);
  }

  gzread (fIN,&file_signature, 4);
  if (file_signature!=0x206d6756) {    // check for 'Vgm ' file signature
    printf("Fatal: input file doesn't seem a valid VGM/VGZ file\n");
    return(1);
  }

  gzseek(fIN,VGM_HEADER_FRAMERATE,SEEK_SET);  // seek to FRAMERATE in the VGM header
  gzread (fIN,&frame_rate, 4);                // read frame_rate

  if (frame_rate==60) {
    printf ("Info: NTSC (60Hz) VGM detected\n");
  } else if (frame_rate==50) {
    printf ("Info: PAL (50Hz) VGM detected\n");
    sample_divider=882;                           // PAL!
  } else {
    printf ("Warning: unknown frame rate, assuming NTSC (60Hz)\n");
  }

  gzseek(fIN,VGM_HEADER_LOOPPOINT,SEEK_SET);  // seek to LOOPPOINT in the VGM header
  gzread (fIN,&loop_offset, 4);               // read loop_offset

  gzseek(fIN,VGM_DATA_OFFSET,SEEK_SET);       // seek to DATAOFFSET in the VGM header
  gzread (fIN,&data_offset, 4);               // read data_offset

  if (data_offset!=0) {
    gzseek(fIN,VGM_DATA_OFFSET+data_offset,SEEK_SET);  // skip VGM header
    data_offset=VGM_DATA_OFFSET+data_offset;
  } else {
    gzseek(fIN,VGM_OLD_HEADERSIZE,SEEK_SET);           // skip 'old' VGM header
    // note: some VGMs can have zero in the data_offset field and have 256 bytes long header instead of 64, filled with zeroes. We do a quick check here.
    c=gzgetc(fIN);
    if (c==0) {
      printf ("Warning: malformed VGM, will try my best\n");
      gzseek(fIN,VGM_BIG_HEADERSIZE,SEEK_SET);         // skip 'big' VGM header
      data_offset=VGM_BIG_HEADERSIZE;
    } else {
      gzseek(fIN,VGM_OLD_HEADERSIZE,SEEK_SET);         // skip 'old' VGM header
      data_offset=VGM_OLD_HEADERSIZE;
    }
  }

  if (loop_offset!=0) {
    printf ("Info: loop point at 0x%08x\n",loop_offset);
    loop_offset=loop_offset+VGM_HEADER_LOOPPOINT-data_offset;
  } else {
    printf ("Info: no loop point defined\n");
    loop_offset=-1; // make it negative so that won't happen
  }

  fOUT=fopen(argv[2],"wb");
  if (!fOUT) {
    printf("Fatal: can't write to output PSG file\n");
    return(1);
  }

  while ((!leave) && (!gzeof(fIN))) {
    c=gzgetc(fIN);
    decLoopOffset(1);
    if (checkLoopOffset()) writeLoopMarker();

    switch (c) {

      case VGM_GGSTEREO:            // stereo data byte follows
        // BETA: this is simply DISCARDED atm
        c=gzgetc(fIN);
        printf("Warning: GameGear stereo info discarded\n");
        decLoopOffset(1);
        if (checkLoopOffset()) writeLoopMarker();
        break;

      case VGM_FMFOLLOWS:
        // discard this!
        c=gzgetc(fIN);
        c=gzgetc(fIN);
        printf("Warning: FM chip write discarded\n");
        decLoopOffset(2);
        if (checkLoopOffset()) writeLoopMarker();
        break;

      case VGM_PSGFOLLOWS:          // PSG byte follows

        c=gzgetc(fIN);

        if (c&0x80) {
          lastlatch=c;               // latch value
          latched_chn=(c&0x60)>>5;   // isolate chn number
        } else {
          c|=0x40;                   // make sure DATA bytes have 1 as 6th bit
        }

        if ((!is_sfx) || (active[latched_chn])) {   // output only if on an active channel

          found_frame();

          if ((first_byte) && ((c&0x80)==0)) {
            add_command(lastlatch);
            printf("Warning: added missing latch command in frame start\n");
          }
          add_command(c);
          first_byte=FALSE;
        }

        decLoopOffset(1);
        if (checkLoopOffset()) writeLoopMarker();
        break;

      case VGM_FRAMESKIP_NTSC:
      case VGM_FRAMESKIP_PAL:

        // frame skip, now count how many
        found_pause();

        fs=1;
        do {
          c=gzgetc(fIN);
          if ((c==VGM_FRAMESKIP_NTSC) || (c==VGM_FRAMESKIP_PAL)) fs++;
          decLoopOffset(1);
        } while ((fs<MAX_WAIT) && ((c==VGM_FRAMESKIP_NTSC) || (c==VGM_FRAMESKIP_PAL)) && (!checkLoopOffset()));

        if ((c!=VGM_FRAMESKIP_NTSC) && (c!=VGM_FRAMESKIP_PAL)) {
          gzungetc(c,fIN);
          incLoopOffset();
        }

        pause_len+=fs;
        if (checkLoopOffset()) writeLoopMarker();

        first_byte=TRUE;

        break;

      case VGM_SAMPLESKIP:         // sample skip, now count how many

        found_pause();

        c=gzgetc(fIN);
        ss=c;
        c=gzgetc(fIN);
        ss+=c*256;
        fs=ss/sample_divider;                           // samples to frames
        if ((ss%sample_divider)!=0) {
          printf("Warning: pause length isn't perfectly frame sync'd\n");
          if ((ss%sample_divider)>(sample_divider/2))   // round to closest int
            fs++;
        }

        pause_len+=fs;

        decLoopOffset(2);
        if (checkLoopOffset()) writeLoopMarker();

        first_byte=TRUE;

        break;


      case VGM_ENDOFDATA:         // end of data
        leave=1;
        decLoopOffset(1);
        if (checkLoopOffset()) writeLoopMarker();
        empty_data();
        fputc(PSG_ENDOFDATA,fOUT);
        break;

      default:
        printf("Fatal: found unknown char 0x%02x\n",c);
        leave=1;
        fatal=1;
        break;
    }

  }

  gzclose (fIN);
  fclose (fOUT);

  if (!fatal) {
    printf ("Info: conversion complete\n");
    return(0);
  } else {
    return(1);
  }
}
