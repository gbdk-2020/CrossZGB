#include "Banks/SetAutoBank.h"

#include "ZGBMain.h"
#include "Scroll.h"

#include "vwf.h"

IMPORT_MAP(map);
IMPORT_VWF_FONT(font);
IMPORT_VWF_FONT(font_fancy);

void START(void) {
	InitScroll(BANK(map), &map, 0, 0);


	vwf_set_destination(VWF_RENDER_BKG);
	vwf_load_font(0, FONT(font), BANK(font));
	vwf_load_font(1, FONT(font_fancy), BANK(font_fancy));
	vwf_activate_font(0);

//	vwf_set_mode(VWF_MODE_RENDER);
//	vwf_set_colors(2, 1);
	UINT8 tiles_used = vwf_draw_text(3, 3, last_tile_loaded, "Hello, world!\n\x01\x01\Fancy font\x01\x00\n\x03\xffthis is a test\x03\x00\nof the new line\ncontrol symbol");

	#ifdef NINTENDO
	INIT_HUD_EX(map, SCREEN_HEIGHT - 8, 8);
	vwf_set_destination(VWF_RENDER_WIN);
	vwf_draw_text(0, 0, last_tile_loaded + tiles_used, "OVERLAY WINDOW");
	#endif	
}

void UPDATE(void) {
}

void DESTROY(void) {
}