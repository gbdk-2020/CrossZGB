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
	void * destructor;
	uint8_t bank;
	void * data;
	struct coro_runner_context_t * next;
} coro_runner_context_t;

// initialize coroutine runner
void coro_runner_init(void);

// alias for the coro_runner_init()
inline void CoroRunnerReset(void) {
	coro_runner_init();
}

// start coroutine, assign destructor, return coroutine context, NULL if failed
void * coro_runner_alloc_ex(coro_t coro, uint8_t coro_bank, void * user_data, coro_t destr, uint8_t destr_bank);


// start coroutine, return coroutine context, NULL if failed
inline void * coro_runner_alloc(coro_t coro, uint8_t coro_bank, void * user_data) {
	return coro_runner_alloc_ex(coro, coro_bank, user_data, NULL, 0);
}

// process coroutine
inline bool coro_runner_process(void * ctx) {
	return coro_continue(&((coro_runner_context_t *)ctx)->coro_context);
}

// free coroutine context
void coro_runner_free(void * ctx);

// coroutine helper macros
#define INIT_CORO(BANK, CORO) if (!(THIS->ctx = coro_runner_alloc(CORO, BANK, THIS->custom_data))) SpriteManagerRemoveSprite(THIS)
#define INIT_CORO_EX(BANK, CORO, BANK_DESTR, DESTRUCTOR) if (!(THIS->ctx = coro_runner_alloc_ex(CORO, BANK, THIS->custom_data, DESTRUCTOR, BANK_DESTR))) SpriteManagerRemoveSprite(THIS)
#define INIT_CORO_WITH_DATA(BANK, CORO, DATA, BANK_DESTR, DESTRUCTOR) if (!(THIS->ctx = coro_runner_alloc_ex(CORO, BANK, DATA, DESTRUCTOR, BANK_DESTR))) SpriteManagerRemoveSprite(THIS)
#define ITER_CORO if (!coro_runner_process(THIS->ctx)) SpriteManagerRemoveSprite(THIS)
#define FREE_CORO coro_runner_free(THIS->ctx)

#define INIT_STATE_CORO(CTX, BANK, CORO, BANK_DESTR, DESTRUCTOR) if ((CTX = coro_runner_alloc_ex(CORO, BANK, NULL, DESTRUCTOR, BANK_DESTR)) && (!coro_runner_process(CTX))) coro_runner_free(CTX),CTX=NULL
#define ITER_STATE_CORO(CTX) if ((CTX) && (!coro_runner_process(CTX))) coro_runner_free((CTX)),CTX=NULL
#define FREE_STATE_CORO(CTX) coro_runner_free(CTX),CTX=NULL

#define YIELD coro_yield()

// empty handler
void NONE(void * data) BANKED;

#define HASH_LIT #
#define HASH() HASH_LIT

#if defined(NINTENDO)
#define __CALL_INIT_CORO__(LOGIC, DESTR, B_LOGIC, B_DESTR, STRAP) \
__asm \
	ld d,HASH()B_DESTR \
	ld e,HASH()B_LOGIC \
	push de \
	ld bc,HASH()DESTR \
	ld de,HASH()LOGIC \
        call STRAP \
	ret \
__endasm
#elif defined(SEGA)
#define __CALL_INIT_CORO__(LOGIC, DESTR, B_LOGIC, B_DESTR, STRAP) \
__asm \
	ld d,HASH()B_DESTR \
	ld e,HASH()B_LOGIC \
	push de \
	ld de,HASH()DESTR \
	ld hl,HASH()LOGIC \
        call STRAP \
	ret \
__endasm
#endif

// sprite definition helper macro
#define SPRITE_COROUTINE(LOGIC, DESTRUCTOR) \
static void __init_sprite_coro(coro_t coro, coro_t destr, uint8_t bank_coro, uint8_t bank_destr) { \
	if (!(THIS->ctx = coro_runner_alloc_ex(coro, bank_coro, THIS->custom_data, destr, bank_destr))) SpriteManagerRemoveSprite(THIS); \
} \
void START(void) NAKED { __CALL_INIT_CORO__(_ ## LOGIC, _ ## DESTRUCTOR, b_ ## LOGIC, b_ ## DESTRUCTOR, ___init_sprite_coro); } \
void UPDATE(void) { ITER_CORO; } \
void DESTROY(void) { FREE_CORO; }

// state definition helper macro
#define STATE_COROUTINE(LOGIC, DESTRUCTOR) \
static void * __state_context; \
static void __init_state_coro(coro_t coro, coro_t destr, uint8_t bank_coro, uint8_t bank_destr) { \
	if ((__state_context = coro_runner_alloc_ex(coro, bank_coro, NULL, destr, bank_destr)) && (!coro_runner_process(__state_context))) coro_runner_free(__state_context),__state_context=NULL; \
} \
void START(void) NAKED { __CALL_INIT_CORO__(_ ## LOGIC, _ ## DESTRUCTOR, b_ ## LOGIC, b_ ## DESTRUCTOR, ___init_state_coro); } \
void UPDATE(void) { ITER_STATE_CORO(__state_context); } \
void DESTROY(void) { FREE_STATE_CORO(__state_context); }

#endif
