#include <gbdk/platform.h>
#include <stdint.h>
#include <stdbool.h>

#include "coroutines_runner.h"

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
		coro_init(&tmp->coro_context, coro, coro_bank, user_data);
		return tmp;
	}
	return NULL;
}

void coro_runner_free(void * ctx) {
	if (!ctx) return;
	((coro_runner_context_t *)ctx)->next = coro_free_ctx;
	coro_free_ctx = (coro_runner_context_t *)ctx;
}
