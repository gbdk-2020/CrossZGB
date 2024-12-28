#include <gbdk/platform.h>
#include <stdint.h>
#include <stdbool.h>

#include "Coroutines.h"

uint16_t * coro_main_context;
coro_context_t * coro_current_context = NULL;

bool coro_yield(void) NONBANKED NAKED {
	__asm
#if defined(__TARGET_gb) || defined(__TARGET_ap) || defined(__TARGET_duck)
		ldhl sp, #-2
		ld d, h
		ld e, l
		ld hl, #_coro_current_context
		ld a, (hl+)
		ld h, (hl)
		ld l, a
#if defined(ALLOW_RUN_CORO_AS_FUNCTION)
		or h
		ret z
#endif
		ld a, e
		ld (hl+), a
		ld (hl), d

		ldh a, (__current_bank)
		push af

		ld hl, #_coro_main_context
		ld a, (hl+)
		ld h, (hl)
		ld l, a
		ld sp, hl

		pop af
		ldh (__current_bank), a
		ld (_rROMB0), a

#if defined(ALLOW_RUN_CORO_AS_FUNCTION)
		xor a
		ld hl, #_coro_current_context
		ld (hl+), a
		ld (hl), a
		inc a
#else
		ld a, #1
#endif
		ret
#elif defined(__TARGET_sms) || defined(__TARGET_gg)
		ld de, (_coro_current_context)
#if defined(ALLOW_RUN_CORO_AS_FUNCTION)
		ld a, d
		or e
		ret z
#endif

		ld hl, #-4
		add hl, sp
		ex de, hl

		ld (hl), e
		inc hl
		ld (hl), d

		push ix
		ld a, (_MAP_FRAME1)
		push af

		ld sp, (_coro_main_context)

		pop af
		ld (_MAP_FRAME1), a
		pop ix

#if defined(ALLOW_RUN_CORO_AS_FUNCTION)
		ld hl, #0
		ld (_coro_current_context), hl
#endif
		ld a, #1
		ret
#else
	#error Unrecognized port
#endif
	__endasm;
}

bool coro_finalize(void) NONBANKED NAKED {
	__asm
#if defined(__TARGET_gb) || defined(__TARGET_ap) || defined(__TARGET_duck)
		ld hl, #_coro_main_context
		ld a, (hl+)
		ld h, (hl)
		ld l, a
		ld sp, hl

		pop af
		ldh (__current_bank), a
		ld (_rROMB0), a

		xor a
#if defined(ALLOW_RUN_CORO_AS_FUNCTION)
		ld hl, #_coro_current_context
		ld (hl+), a
		ld (hl), a
#endif
		ret
#elif defined(__TARGET_sms) || defined(__TARGET_gg)
		ld sp, (_coro_main_context)

		pop af
		ld (_MAP_FRAME1), a
		pop ix

#if defined(ALLOW_RUN_CORO_AS_FUNCTION)
		ld hl, #0
		ld (_coro_current_context), hl
#endif
		xor a
		ret
#else
	#error Unrecognized port
#endif
	__endasm;
}

void coro_init(coro_context_t * context, coro_t coro, uint8_t coro_bank, void * user_data) NONBANKED {
	uint16_t * stack = context->stack + ((CORO_STACK_SIZE >> 1) - 1);
	*stack = (uint16_t)user_data;
#if defined(__TARGET_gb) || defined(__TARGET_ap) || defined(__TARGET_duck)
	stack = (uint16_t *)((uint8_t *)stack - 6); // match SM83 banked call convention
#elif defined(__TARGET_sms) || defined(__TARGET_gg)
	stack = (uint16_t *)((uint8_t *)stack - 5); // match Z80 banked call convention
#else
	#error Unrecognized port
#endif
	*stack-- = (uint16_t *)coro_finalize;
	*stack = (uint16_t *)coro;
#if defined(__TARGET_gb) || defined(__TARGET_ap) || defined(__TARGET_duck)
	*--stack = coro_bank << 8;                  // coroutine bank
#elif defined(__TARGET_sms) || defined(__TARGET_gg)
	stack -= 2;                                 // dummy IX value
	*stack = coro_bank << 8;                    // coroutine bank
#else
	#error Unrecognized port
#endif
	context->SP = stack;
}

bool coro_continue(coro_context_t * context) NONBANKED NAKED {
	context;
	__asm
#if defined(__TARGET_gb) || defined(__TARGET_ap) || defined(__TARGET_duck)
		ldh a, (__current_bank)
		push af
		ld (_coro_main_context), sp

		ld hl, #_coro_current_context
		ld (hl), e
		inc hl
		ld (hl), d

		ld h, d
		ld l, e
		ld a, (hl+)
		ld h, (hl)
		ld l, a
		ld sp, hl

		pop af
		ldh (__current_bank), a
		ld (_rROMB0), a

		ret
#elif defined(__TARGET_sms) || defined(__TARGET_gg)

		push ix
		ld a, (_MAP_FRAME1)
		push af
		ld (_coro_main_context), sp

		ld (_coro_current_context), hl

		ld a, (hl)
		inc hl
		ld h, (hl)
		ld l, a
		ld sp, hl

		pop af
		ld (_MAP_FRAME1), a
		pop ix

		ret
#else
	#error Unrecognized port
#endif
	__endasm;
}
