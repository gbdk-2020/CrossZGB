#include <gbdk/platform.h>
#include <stdint.h>
#include <string.h>

static uint8_t _save;

void * memcpy_banked(void *dest, const void *src, size_t len, uint8_t bank) NONBANKED {
    _save = _current_bank;
    SWITCH_ROM(bank);
    void * res = memcpy(dest, src, len);
    SWITCH_ROM(_save);
    return res;
}
