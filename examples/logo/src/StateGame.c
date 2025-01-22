#include "Banks/SetAutoBank.h"

#include "ZGBMain.h"
#include "Scroll.h"

IMPORT_MAP(logo);

void START(void) {
	InitScroll(BANK(logo), &logo, 0, 0);
}

void UPDATE(void) {
}

void DESTROY(void) {
}
