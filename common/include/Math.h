#ifndef MATH_H
#define MATH_H

#include <gbdk/platform.h>

#define U_LESS_THAN(A, B) (((INT16)((A) - (B)) < 0) ? 1 : 0)

#define DISTANCE(A, B) ((A < B) ? (B - A) : (A - B))

#define SET_BIT(N, POS)   ((N) |=  (1 << (POS)))
#define UNSET_BIT(N, POS) ((N) &= ~(1 << (POS)))
#define GET_BIT(N, POS)   (1 & DespRight(N, POS))

#define SET_BIT_MASK(N, MASK) ((N) |= (MASK))
#define UNSET_BIT_MASK(N, MASK) ((N) &= ~(MASK))
#define GET_BIT_MASK(N, MASK) ((N) & (MASK))

inline INT16 DespRight(INT16 a, UINT8 b) {
	return a >> b;
}

#endif