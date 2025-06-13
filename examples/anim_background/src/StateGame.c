#include "Banks/SetAutoBank.h"

#include "ZGBMain.h"
#include "Scroll.h"
#include "SpriteManager.h"
#include "Vector.h"

IMPORT_MAP(map);

IMPORT_TILES(anim_tiles);

const UINT8 flower_anim[] = VECTOR(0, 1, 2, 2, 1, 0);

void UpdateTilesFromTileset(UINT8 tiles_bank, const struct TilesInfo* tiles, UINT8 dest_tile, UINT8 sour_tile, UINT8 n_tiles) NONBANKED {
	UINT8 __save = CURRENT_BANK;
	SWITCH_ROM(tiles_bank);
	#if DEFAULT_COLOR_DEPTH == 4
	set_bkg_native_data(dest_tile, n_tiles, tiles->data + ((UINT16)sour_tile << 5));
	#else
	set_bkg_data(dest_tile, n_tiles, tiles->data + ((UINT16)sour_tile << 4));
	#endif
	SWITCH_ROM(__save);
}


void START(void) {
	InitScroll(BANK(map), &map, NULL, NULL);
}

void UPDATE(void) {
	static UINT8 timer = 0, idx = 0;
        if (++timer == 12) {
		timer = 0;
		if (++idx == VECTOR_LEN(flower_anim)) idx = 0;
		UpdateTilesFromTileset(BANK(anim_tiles), &anim_tiles, 2, VECTOR_GET(flower_anim, idx), 1);
	}
}

void DESTROY(void) {
}
