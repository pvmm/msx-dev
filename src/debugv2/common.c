#ifndef DEBUG_H
#define DEBUG_H

#ifdef USE_DEBUG_MODE

#include "common.h"
#include <stdint.h>


// send value to debug device (control)
void _out2e(uint8_t value) __z88dk_fastcall
{
    value;
    __asm
        ld a, l
        out (#0x2e), a
    __endasm;
}


// send value to debug device (data)
void _out2f(uint8_t value) __z88dk_fastcall
{
    UNUSED(value);
    __asm
        ld a, l
        out (#0x2f), a
    __endasm;
}


void debug_mode(uint8_t mode)
{
    _out2e(mode);
}


// pause debug device (by tcl script)
void debug_break()
{
    __asm
        ld a, #0xff
        out (#0x2e), a
    __endasm;
}


// send printf like structure to tcl script
inline void _debug_printf(struct debug_printf_data* data)
{
    UNUSED(data);
    __asm
        ld c, #0x2f
        out (c), l
        out (c), h
    __endasm;
}

#else

typedef int make_iso_compilers_happy;

#endif // USE_DEBUG_MODE

#endif // DEBUG_H
