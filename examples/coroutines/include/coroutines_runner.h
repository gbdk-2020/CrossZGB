#ifndef __COROUTINES_RUNNER_H_INCLUDE__
#define __COROUTINES_RUNNER_H_INCLUDE__

#include <gbdk/platform.h>
#include <stdint.h>
#include <stdbool.h>

#include "coroutines.h"

#define CORO_MAX_CONTEXTS 16

typedef struct coro_runner_context_t {
	struct coro_context_t coro_context;
	struct coro_runner_context_t * next;
} coro_runner_context_t;

// initialize coroutine runner
void coro_runner_init(void);

// start coroutine, return coroutine id
void * coro_runner_alloc(coro_t coro, uint8_t coro_bank, void * user_data);

// process coroutines (run each until yield)
inline bool coro_runner_process(void * ctx) {
    return coro_continue(&((coro_runner_context_t *)ctx)->coro_context);
}

// kill running coroutine and free context
void coro_runner_free(void * ctx);


#endif