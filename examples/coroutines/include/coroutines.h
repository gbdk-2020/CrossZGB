#ifndef __COROUTINES_H_INCLUDE__
#define __COROUTINES_H_INCLUDE__

#include <gbdk/platform.h>
#include <stdint.h>
#include <stdbool.h>

#define MAX_CORO_STACK_SIZE 64
#define ALLOW_RUN_CORO_AS_FUNCTION

typedef struct coro_context_t {
	uint16_t * SP;
	uint16_t stack[MAX_CORO_STACK_SIZE >> 1];
} coro_context_t;
typedef void (* coro_t)(void * user_data) BANKED;

extern coro_context_t * coro_current_context;

// suspend coroutine execution and pass into main
bool coro_yield(void);

// initialize coroutine, but don't start
void coro_init(coro_context_t * context, coro_t coro, uint8_t coro_bank, void * user_data);

// continue coroutine execution
bool coro_continue(coro_context_t * context);

// initialize and start coroutine
inline bool coro_start(coro_context_t * context, coro_t coro, uint8_t coro_bank, void * user_data) {
	coro_init(context, coro, coro_bank, user_data);
	return coro_continue(context);
}

#endif