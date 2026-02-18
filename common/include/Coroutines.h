#ifndef __COROUTINES_H_INCLUDE__
#define __COROUTINES_H_INCLUDE__

#include <gbdk/platform.h>
#include <stdint.h>
#include <stdbool.h>

#include "SpriteManager.h"

#ifndef CORO_STACK_SIZE
	#define CORO_STACK_SIZE 96
#endif
#define ALLOW_RUN_CORO_AS_FUNCTION

typedef struct coro_context_t {
	uint16_t * SP;
	uint16_t stack[CORO_STACK_SIZE >> 1];
} coro_context_t;
typedef void (* coro_t)(void * user_data) BANKED;

extern coro_context_t * coro_current_context;

// suspend coroutine execution and pass into main
bool coro_yield(void);

// initialize coroutine, but don't start
void coro_init(coro_context_t * context, coro_t coro, uint8_t coro_bank, void * user_data, uint16_t stack_size);

// continue coroutine execution
bool coro_continue(coro_context_t * context);

// initialize and start coroutine
inline bool coro_start(coro_context_t * context, coro_t coro, uint8_t coro_bank, void * user_data, uint16_t stack_size) {
	coro_init(context, coro, coro_bank, user_data, stack_size);
	return coro_continue(context);
}

#ifndef CORO_MAX_CONTEXTS
	#define CORO_MAX_CONTEXTS 16
#endif

typedef struct coro_runner_context_t {
	struct coro_context_t coro_context;
	struct coro_runner_context_t * next;
} coro_runner_context_t;

// initialize coroutine runner
void coro_runner_init(void);

// alias for the coro_runner_init()
inline void CoroRunnerReset(void) {
	coro_runner_init();
}

// start coroutine, return coroutine context, NULL if failed
void * coro_runner_alloc(coro_t coro, uint8_t coro_bank, void * user_data);

// process coroutine
inline bool coro_runner_process(void * ctx) {
	return coro_continue(&((coro_runner_context_t *)ctx)->coro_context);
}

// free coroutine context
void coro_runner_free(void * ctx);

// coroutine helper macros
#define INIT_CORO(BANK, CORO) if (!(THIS->ctx = coro_runner_alloc(CORO, BANK, THIS->custom_data))) SpriteManagerRemoveSprite(THIS)
#define INIT_CORO_WITH_DATA(BANK, CORO, DATA) if (!(THIS->ctx = coro_runner_alloc(CORO, BANK, DATA))) SpriteManagerRemoveSprite(THIS)
#define INIT_AND_ITER_CORO(BANK, CORO, DATA) \
	if (THIS->ctx = coro_runner_alloc(CORO, BANK, DATA)) { \
		if (!coro_runner_process(THIS->ctx)) SpriteManagerRemoveSprite(THIS); \
	} else SpriteManagerRemoveSprite(THIS)
#define ITER_CORO if (!coro_runner_process(THIS->ctx)) SpriteManagerRemoveSprite(THIS)
#define FREE_CORO coro_runner_free(THIS->ctx)

#define INIT_STATE_CORO(CTX, BANK, CORO) if ((CTX = coro_runner_alloc(CORO, BANK, NULL)) && (!coro_runner_process(CTX))) coro_runner_free(CTX),CTX=NULL
#define ITER_STATE_CORO(CTX) if ((CTX) && (!coro_runner_process(CTX))) coro_runner_free((CTX)),CTX=NULL
#define FREE_STATE_CORO(CTX) if (CTX) coro_runner_free(CTX)

#define YIELD coro_yield()

#endif                    