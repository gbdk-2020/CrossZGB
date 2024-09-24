# Bresenham aiming example with the bullet logic which runs in coroutines
Shooting projectiles at player, each bullet has limited lifetime of 50 frames. Program may allocate up to 16 coroutine contexts, thus 16 bullets may be processed at once.

Stackful coroutines are implemented. Use MAX_CORO_STACK_SIZE definition to set the coroutine stack size (64 by default), and CORO_MAX_CONTEXTS to set the maximum allocated contexts (16 by default).