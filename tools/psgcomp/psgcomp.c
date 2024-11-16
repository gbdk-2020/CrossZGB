#include <stdio.h>
#include <stdlib.h>
#include <string.h>

FILE *fIN;
FILE *fOUT;

#define BUF_SIZE        65536
#define MIN_LEN         4
#define MAX_LEN         51        // 47+4

#define PSG_SUBSTRING   0x08


unsigned char buf[BUF_SIZE];

int size;

int mem_compare (const char* p1, const char* p2, int max_len) {
  // compares buffer *p1 and buffer *p2 until they no longer match or if max_len is reached
  // returns lenght of the match ( 0 to max_len inclusive )
  int match_len=0;
  
  while ((match_len<max_len) && (*p1++==*p2++))
    match_len++;
    
  return (match_len);
}

int main (int argc, char *argv[]) {

  int m_len, haystack, consolidate;
  
  int skip, needle;
  
  int max_save, max_save_at_skip,max_save_at_len,max_save_at_haystack;

  if (argc!=3) {
    printf ("Usage: psgcomp inputfile.PSG outputfile.PSG\n");
    return (1);
  }
  
  fIN=fopen(argv[1],"rb");
  if (!fIN) {
    perror(argv[1]);
    return (1);
  }
  size=fread (&buf, 1, BUF_SIZE, fIN);      // read input file
  fclose (fIN);
  
  printf ("Info: input file size is %d bytes\n",size);
  
  if (size>=4*1024)                             // just for fun
    printf ("Info: compression started - this will take a while...\n");
  else
    printf ("Info: compression started.\n");
  
  // start 'consolidating' from beginning of buf[4], to end-4
  for (consolidate=MIN_LEN; consolidate<(size-MIN_LEN); consolidate++) {
  
    max_save=0;
    
    for (skip=0; skip<MAX_LEN; skip++) {
    
      // is it worth even trying?
      if (max_save<(MAX_LEN-3-skip)) {

        // haystack is from buf[0]
        for (haystack=0; haystack<=(consolidate+skip-MAX_LEN); haystack++) {   // less or equal, not only less than

          needle=consolidate+skip;

          m_len=mem_compare(&buf[needle],&buf[haystack],
                            (MAX_LEN<(size-haystack))?MAX_LEN:(size-haystack));  // do not overrun!

          // have we found a more interesting correspondance?
          if ((m_len-3-skip)>max_save) {
            max_save=(m_len-3-skip);
            max_save_at_skip=skip;
            max_save_at_len=m_len;
            max_save_at_haystack=haystack;
          }
        }
      }
    }

    if (max_save>0) {
      
      if (max_save_at_skip==0) {
      
        consolidate+=max_save_at_skip;   // consolidate 'skip' bytes
        m_len=max_save_at_len;
        haystack=max_save_at_haystack;
      
        //  printf ("Skip %2d bytes, compress string of len %d\n",max_save_at_skip,max_save_at_len);
      
        buf[consolidate]=(m_len-MIN_LEN)+PSG_SUBSTRING;
        buf[consolidate+1]=(haystack&0xFF);
        buf[consolidate+2]=(haystack>>8);
        memmove(&buf[consolidate+3],&buf[consolidate+m_len],size-(consolidate+3));
        size-=(m_len-3);
        consolidate+=2;                  // 'consolidate' two additional bytes
       } else {
        consolidate+=(max_save_at_skip-1);   // consolidate 'skip-1' bytes
       }
        
        
    } else {
    
      // printf ("Skip 1 byte\n");
    
    }
    
    if (consolidate%100==0)
      printf ("\rProgress: %d%% ...",(consolidate*100)/size);

  }

  fOUT=fopen(argv[2],"wb");
  fwrite (&buf, 1, size, fOUT); 
  fclose (fOUT);
  
  printf ("\rProgress: Done!         \n");
  printf ("Info: output file size is %d bytes\n",size);
  return(0);
}
