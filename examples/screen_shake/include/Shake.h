#ifndef __SHAKEVBLHANDLER_INCLUDE__
#define __SHAKEVBLHANDLER_INCLUDE__

#define SHAKE_X 0x01
#define SHAKE_Y 0x02

extern UINT8 shake_duration, shake_type, shake_intensivity; 

inline void Shake(UINT8 duration, UINT8 type, UINT8 intensivity) {
	shake_duration = duration; shake_type = type; shake_intensivity = intensivity;
}

#endif