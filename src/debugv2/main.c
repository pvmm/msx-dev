#include <stdint.h>
#ifdef __SDCC
#pragma less_pedantic
#include "common.h"
#endif

#if defined(__GNUC__) || defined(__clang__)
#define debug_printf(...) printf(__VA_ARGS__)
#define debug_mode(x)
#define debug_msg(x)
#define debug(x, y)
#include <stdio.h>
#endif

#define n "\n" // ""
	       //
int arg2 = 0x8001;

int main()
{
    const char* msg2 = "String variable test...";
    int16_t arg3 = -27;
    debug_mode(1);
    debug_msg("Begin testing...");
    debug_msg(msg2);
    const char arg4[] = {64, 18, 52, 86}; // 0.123456e+0
    float arg5 = 123456.0f;
    debug("float size: ", sizeof(arg5));
    uint32_t arg6 = 4294967294;
    char H = 'H';
    char arg1[] = "Pedro de Medeiros";
    int arg7 = 1000;

    debug_printf("%cello, I am %.5s (%S) and it's nice to meet you!"n, H, arg1, arg1);
    debug_printf("Let's talk about numbers! This is a left padded number (-32767) with 10 spaces: [%10i]..."n, arg2);
    debug_printf("Right padded number (-32767) with 10 spaces: [%-10i]..."n, arg2);
    debug_printf("Treat a 16-bit number (-32767) like an 8-bit unsigned int: %hhu"n, arg2);
    debug_printf("Now signed number -27: %hi"n, arg3);
    debug_printf("The float number 123.0 in BASIC (%f) and SDCC float format (%hf)"n, arg4, arg5);
    debug_printf("-32767 in hexadecimal: %hx\n-32767 in binary: %hb\n-32767 with "
                 "left padded zeros: %08d"n, arg2, arg2, arg2);
    debug_printf("1000 as positive integer with signal: %+i"n, arg7);
    debug_printf("The max uint32 - 1 as unsigned: %lu"n, arg6);
    debug_printf("The max uint32 - 1 as hexadecimal: %lx"n, arg6);
    debug_printf("The max uint32 - 1 as signed: %li"n, arg6);
    debug_printf("The function main pointer at %p"n, main);
    debug_printf("Goodbye!\n");

finished:
    goto finished;
}
