#ifndef __INCLUDE_BANKUTILS_H__
#define __INCLUDE_BANKUTILS_H__

#include <gbdk/platform.h>
#include <stdint.h>

void * memcpy_banked(void *dest, const void *src, size_t len, uint8_t bank) NONBANKED;

#endif
