#ifndef ZCONSOLE_H
#define ZCONSOLE_H

#include <gb/gb.h> 

extern UINT8 last_tile_loaded;
#define INIT_FONT(FONT, TARGET) font_idx = last_tile_loaded; print_target = TARGET; ScrollSetTiles(last_tile_loaded, BANK(FONT), &FONT);
#define INIT_FONT_NUMBERS(FONT, TARGET) font_idx = last_tile_loaded - 27; print_target = TARGET; ScrollSetTiles(last_tile_loaded, BANK(FONT), &FONT);

extern UINT8 print_x, print_y, font_idx, print_target;
extern INT8 scroll_h_border;

typedef enum {
	PRINT_BKG,
	PRINT_WIN
} PRINT_TARGET;

void UIntToString(UINT16 n, unsigned char* str);
void IntToString (INT16  n, unsigned char* str);

void Printf(const char* txt, ...);
#define PRINT_POS(X, Y) print_x = X; print_y  = Y
#define PRINT(X, Y, TXT) PRINT_POS(X,Y); Printf(TXT)

#ifdef NDEBUG 
#define INIT_CONSOLE
#define DPrintf
#define DPRINT_POS
#define DPRINT
#else
#define INIT_CONSOLE(FONT, FONT_BANK, NLINES) \
	INIT_FONT(FONT, PRINT_WIN); \
	print_x = 0;\
	print_y = 0;\
	WX_REG = 7;\
	SetWindowY(144 - (NLINES << 3));\
	scroll_h_border = NLINES << 3;\
	SHOW_WIN;

#define DPrintf Printf
#define DPRINT_POS PRINT_POS
#define DPRINT PRINT

#endif


#endif
