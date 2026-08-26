	.include        "global.s"

	.title  "VRAM utilities"
	.module VRAMUtils

	.area   _HOME

; void set_vram_word(uint8_t * addr, uint16_t v) __preserves_regs(iyh, iyl);
_set_vram_word::
	ld b, h
	ld c, l

	DISABLE_VBLANK_COPY

	VDP_WRITE_CMD b, c

	VDP_WRITE_DATA d, e

	ENABLE_VBLANK_COPY

	ret
