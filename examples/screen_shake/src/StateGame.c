#include "Banks/SetAutoBank.h"

#include "ZGBMain.h"
#include "Keys.h"

#include "Shake.h"

IMPORT_MAP(map);

void START(void) {
	InitScroll(BANK(map), &map, NULL, NULL);
}

void UPDATE(void) {
	if KEY_PRESSED(J_A) Shake(10, SHAKE_X, 5);
	if KEY_PRESSED(J_B) Shake(10, SHAKE_Y, 5);
}

void DESTROY(void) {
}
