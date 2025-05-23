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

void * coro_runner_alloc(coro_t coro, uint8_t coro_bank, void * user_data) {
	if (coro_free_ctx) {
		coro_runner_context_t * tmp = coro_free_ctx;
		coro_free_ctx = tmp->next;
		coro_init(&tmp->coro_context, coro, coro_bank, user_data, CORO_STACK_SIZE);
		return tmp;
	}
	return NULL;
}

void coro_runner_free(void * ctx) {
	if (!ctx) return;
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
