#include <gbdk/platform.h>

#if defined(NINTENDO)

#define LYC_SYNC_VALUE 150u
#define LYC_NEVER_FIRE 160u
UINT8 win_start = DEVICE_SCREEN_PX_HEIGHT + 1u, win_stop = LYC_SYNC_VALUE, win_x = DEVICE_WINDOW_PX_OFFSET_X;
void LCD_isr(void) NONBANKED {
	if (LYC_REG == win_start) {
		WX_REG = win_x;
		SHOW_WIN; HIDE_SPRITES;
		LYC_REG = win_stop;
	} else {
		WX_REG = 0;
		HIDE_WIN; SHOW_SPRITES;
		LYC_REG = win_start;
	}
}

void SetWindowPos(UINT8 x, UINT8 y, UINT8 h) {
	if ((h) && (y < DEVICE_SCREEN_PX_HEIGHT)) {
		win_x = WX_REG = x;
		WY_REG = y;
		win_stop = (((UINT16)(y) + h) < LYC_SYNC_VALUE) ? (y + h) : LYC_SYNC_VALUE;
		LYC_REG = win_start = (y) ? (y - 1u) : (LYC_SYNC_VALUE + 1u);
	} else {
		LYC_REG = LYC_NEVER_FIRE;
		HIDE_WIN; SHOW_SPRITES;		
	}
}

#endif

void InitWindow(void) {
#if defined(NINTENDO)
	CRITICAL {
		// HBlank interrupt is used for the window manipulations on the Game Boy targets 		
		STAT_REG |= STATF_LYC;
		add_LCD(LCD_isr);
	}
	set_interrupts(IE_REG | LCD_IFLAG);
#endif
}
