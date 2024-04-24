#include <stdint.h>
#include "common.h"

char* arg1 = "Pedro de Medeiros";
int arg2 = 0x1bb7;
int arg3 = -27;

#define n "" // "\n"

int main()
{
    debug_printf("Hello, I am %.5s (%S) and it's nice to meet you!"n, arg1, arg1);
    debug_printf("Let's talk about numbers! This is a left padded number with 10 spaces: %10i..."n, &arg2);
    debug_printf("Right padded number with 10 spaces: %-10i..."n, &arg2);
    debug_printf("Treat a 16-bit number like an 8-bit unsigned int: %hhu"n, &arg2);
    debug_printf("Now signed: %hi"n, &arg3);
    debug_printf("Uppercase hexadecimal: %hX"n, &arg2);
    debug_printf("16-bit binary: %hb"n, &arg2);
    debug_printf("Left padded zeros: %08u"n, &arg2);
    debug_printf("Goodbye!\n", NULL);

finished:
    goto finished;
}
