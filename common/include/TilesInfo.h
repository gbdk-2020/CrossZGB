#ifndef TILES_INFO_H
#define TILES_INFO_H

typedef struct TilesInfo {
	unsigned char num_frames;
	unsigned char* data;
	unsigned char num_pals;
	unsigned char* pals;
	unsigned char* color_data;
} TilesInfo;

#endif