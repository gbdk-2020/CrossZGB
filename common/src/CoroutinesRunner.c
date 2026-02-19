#include <gbdk/platform.h>
#include <stdint.h>
#include <stdbool.h>

#include "Coroutines.h"

coro_runner_context_t coro_contexts[CORO_MAX_CONTEXTS];
coro_runner_context_t * coro_free_ctx = NULL;

void coro_runner_init(void) {
	coro_free_ctx = NULL;
	for (coro_runner_context_t * i = coro_contexts; i != (coro_contexts + CORO_MAX_CONTEXTS); i++) {
		i->next = coro_free_ctx;
		coro_free_ctx = i;
	}
}

void * coro_runner_alloc_ex(coro_t coro, uint8_t coro_bank, void * user_data, coro_t destr) {
	if (coro_free_ctx) {
		coro_runner_context_t * tmp = coro_free_ctx;
		coro_free_ctx = tmp->next;
		coro_init(&tmp->coro_context, coro, coro_bank, user_data, CORO_STACK_SIZE);
		tmp->destructor = (uint16_t *)destr;
		tmp->bank = coro_bank;
		tmp->data = user_data;
		return tmp;
	}
	return NULL;
}

static void coro_call_handler(void * handler, uint16_t bank, void * data) NONBANKED NAKED {
	handler; bank; data;
	__asm
#if defined(__TARGET_gb) || defined(__TARGET_ap) || defined(__TARGET_duck)
                ld a, d
		or e
		jr z, 1$		

		ldhl sp, #2
		ld a, (hl+)
		ld h, (hl)
		ld l, a

		push hl
                ld h, d
                ld l, e
		ld e, c
		call ___sdcc_bcall_ehl
		pop hl
		pop hl
		pop bc
		jp (hl)
1$:
		pop hl
                pop bc
		jp (hl)
#elif defined(__TARGET_sms) || defined(__TARGET_gg)
		pop iy
		pop bc
		
		ld a, h
		or l
		jp z, 1$

		push iy
		push bc
		call ___sdcc_bcall_ehl
		pop bc
		ret
1$:
		jp (iy)		
#else
	#error Unrecognized port
#endif
	__endasm;
}

void coro_runner_free(void * ctx) {
	if (!ctx) return;
	coro_call_handler(((coro_runner_context_t *)ctx)->destructor, ((coro_runner_context_t *)ctx)->bank, ((coro_runner_context_t *)ctx)->data);
	((coro_runner_context_t *)ctx)->next = coro_free_ctx;
	coro_free_ctx = (coro_runner_context_t *)ctx;
}

static void __initializer__(void) NONBANKED NAKED {
	__asm
#if defined(__TARGET_gb) || defined(__TARGET_ap) || defined(__TARGET_duck) || defined(__TARGET_sms) || defined(__TARGET_gg)
		; we inject the call to the coro_runner_init() in the _GSINIT section, 
		; so there will be no need to call it explicitly in the game code

		.AREA _GSINIT

		call _coro_runner_init
#else
	#error Unrecognized port
#endif
	__endasm;
}
