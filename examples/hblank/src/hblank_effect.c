#include "ZGBMain.h"

extern const INT8 scanline_offsets_tbl[];
const INT8 * scanline_offsets = scanline_offsets_tbl;
UINT8 isr_scan_counter;

// Additional VBlank handler
void effect_VBL_isr(void) NONBANKED {
	// change offset for the wobbling table each 4 frames
        scanline_offsets = scanline_offsets_tbl + ((sys_time >> 2) & 0x07u);
	// reset scanline counter for SMS/GG (polling VCOUNTER is too slow)
	isr_scan_counter = 0; 
}

#if defined(NINTENDO)
// declare the standard LCD handler
void LCD_isr(void);
// the prepared SCX value table in ROM
const INT8 scanline_offsets_tbl[] = {0, 1, 2, 3, 3, 2, 1, 0, 0, 1, 2, 3, 3, 2, 1, 0};

// HBlank ISR function, must be EXTREMELY fast to be executed each line and fit the timing!!!
void effect_LCD_isr(void) NONBANKED {
	SCX_REG = scanline_offsets[(LY_REG & 0x07u)];
}
// Game Boy ISR install/uninstall functions
void install_effect(void) {
	// with disabled interrupts
	CRITICAL {
		// remove the standard CrossZGB handler
		remove_LCD(LCD_isr);
		// add required handlers
		add_LCD(effect_LCD_isr);
		add_LCD(nowait_int_handler);   // cancel waiting before returning from the interrupt
		add_VBL(effect_VBL_isr);
		// setup HBlank interrupts fire every line
        	STAT_REG = STATF_MODE00;       // fire each line
	}
}

void uninstall_effect(void) {
	// with disabled interrupts
	CRITICAL {
		// remove previously installed handlers
		remove_LCD(nowait_int_handler);
		remove_LCD(effect_LCD_isr);
		remove_VBL(effect_VBL_isr);
		// restore CrossZGB default handler
		add_LCD(LCD_isr);
		// restore default CrossZGB HBlank behavior
		STAT_REG = STATF_LYC;          // fire when LY == LYC (CrossZGB behavior)
	}
}

#elif defined(SEGA)

// the prepared RSCX (VDP R8) value table in ROM
const INT8 scanline_offsets_tbl[] = {0, -1, -2, -3, -3, -2, -1, 0, 0, -1, -2, -3, -3, -2, -1, 0};

// HBlank ISR function, must be EXTREMELY fast to be executed each line and fit the timing!!!
void effect_LCD_isr(void) NONBANKED {
	__WRITE_VDP_REG_UNSAFE(VDP_RSCX, scanline_offsets[(isr_scan_counter++ & 0x07u)]);
}
// Master System and Game Gear ISR install/uninstall functions
void install_effect(void) {
	// with disabled interrupts
	CRITICAL {
		// add required handlers
		add_LCD(effect_LCD_isr);
		add_VBL(effect_VBL_isr);
		// set up HBlank interrupt parameters
		__WRITE_VDP_REG_UNSAFE(VDP_R10, 0x00);
	}
	// enable VBlank and HBlank interrupts
	set_interrupts(VBL_IFLAG | LCD_IFLAG);
}
void uninstall_effect(void) {
	// with disabled interrupts
	CRITICAL {
		// remove previously installed handlers
		remove_LCD(effect_LCD_isr);
		remove_VBL(effect_VBL_isr);
		// make HBlank never fire
		__WRITE_VDP_REG_UNSAFE(VDP_R10, 0xff);
	}
	// enable VBlank interrupt only
	set_interrupts(VBL_IFLAG);
}

#endif
