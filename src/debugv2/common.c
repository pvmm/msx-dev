#ifndef DEBUG_H
#define DEBUG_H

#ifdef USE_DEBUG_MODE

#include "common.h"
#include <stdint.h>


uint16_t _stored_numeric_mode = DEBUG_INT;


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
    _stored_numeric_mode = mode;
}


inline void _debug_str(const char* msg)
{
    _out2e(DEBUG_CHAR);
    for (int i = 0; msg[i] != 0; ++i) {
        _out2f(msg[i]);
    }
}


void debug_msg(char* msg)
{
    _debug_str(msg);
    _out2f('\n');
}


void _debug_num(uint16_t value)
{
    _out2e(_stored_numeric_mode);
    _out2f(value & 0xff);               // LSB
    _out2f((value >> 8) & 0xff);        // MSB
}


void debug(char* msg, int value)
{
    _debug_str(msg);
    _debug_num(value);
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
        ld a, #DEBUG_PRINTF
        out (#0x2e), a
        ld c, #0x2f
        out (c), l
        out (c), h
    __endasm;
}

#else

typedef int make_iso_compilers_happy;

#endif // USE_DEBUG_MODE

#endif // DEBUG_H
