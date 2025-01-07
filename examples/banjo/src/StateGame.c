#include "Banks/SetAutoBank.h"

#include "ZGBMain.h"
#include "Scroll.h"
#include "Music.h"

DECLARE_MUSIC(cmajor_sn);

void START(void) {
	PlayMusic(cmajor_sn, 1);
}

void UPDATE(void) {
}

void DESTROY(void) {
}
