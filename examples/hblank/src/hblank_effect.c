#include "ZGBMain.h"

extern const INT8 scanline_offsets_tbl[];
const INT8 * scanline_offsets = scanline_offsets_tbl;
UINT8 scan_counter;

void effect_VBL_isr(void) NONBANKED {
        scanline_offsets = scanline_offsets_tbl + ((sys_time >> 2) & 0x07u);
	scan_counter = 0; 
}

#if defined(NINTENDO)
// declare the standard LCD handler
void LCD_isr(void);

const INT8 scanline_offsets_tbl[] = {0, 1, 2, 3, 3, 2, 1, 0, 0, 1, 2, 3, 3, 2, 1, 0};

void effect_LCD_isr(void) NONBANKED {
	SCX_REG = scanline_offsets[(LY_REG & 0x07u)];
}

void install_effect(void) {
	CRITICAL {
		remove_LCD(LCD_isr);
		add_LCD(effect_LCD_isr);
		add_LCD(nowait_int_handler);
		add_VBL(effect_VBL_isr);
        	STAT_REG = STATF_MODE00;
	}
}

void uninstall_effect(void) {
	CRITICAL {
		remove_LCD(nowait_int_handler);
		remove_LCD(effect_LCD_isr);
		remove_VBL(effect_VBL_isr);
		add_LCD(LCD_isr);
		STAT_REG = STATF_LYC;
	}
}

#else

const INT8 scanline_offsets_tbl[] = {0, -1, -2, -3, -3, -2, -1, 0, 0, -1, -2, -3, -3, -2, -1, 0};

void effect_LCD_isr(void) NONBANKED {
	__WRITE_VDP_REG_UNSAFE(VDP_RSCX, scanline_offsets[(scan_counter++ & 0x07u)]);
}

void install_effect(void) {
	CRITICAL {
		add_LCD(effect_LCD_isr);
		add_VBL(effect_VBL_isr);
		__WRITE_VDP_REG_UNSAFE(VDP_R10, 0x00);
	}
	set_interrupts(VBL_IFLAG | LCD_IFLAG);
}
void uninstall_effect(void) {
	CRITICAL {
		remove_LCD(effect_LCD_isr);
		remove_VBL(effect_VBL_isr);
		__WRITE_VDP_REG_UNSAFE(VDP_R10, 0xff);
	}
	set_interrupts(VBL_IFLAG);
}

#endif
