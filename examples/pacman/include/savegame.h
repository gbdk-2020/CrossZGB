#ifndef SAVEGAME_H
#define SAVEGAME_H

#include "SRAM.h"

typedef struct {
	SAVEGAME_HEADER;

	UINT16 best_score;
} Savegame;

extern Savegame savegame;

#endif
