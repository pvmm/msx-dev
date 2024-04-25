#include <stdint.h>
#include "common.h"

int arg2 = 0x1bb7;

#define n "\n" // ""

//#pragma less_pedantic

int main()
{
    int arg3 = -27;
    debug_msg("Begin testing...");
    char msg2[] = "String variable test...";
    debug_msg(msg2);
    debug_mode(1);
    int debug_value = 123;
    debug("value = ", debug_value);
    char H = 'H';
    char arg1[] = "Pedro de Medeiros";
    debug_printf("%cello, I am %.5s (%S) and it's nice to meet you!"n, &H, arg1, arg1);
    debug_printf("Let's talk about numbers! This is a left padded number (7095) with 10 spaces: %10i..."n, &arg2);
    debug_printf("Right padded number (7095) with 10 spaces: %-10i..."n, &arg2);
    debug_printf("Treat a 16-bit number (7095) like an 8-bit unsigned int: %hhu"n, &arg2);
    debug_printf("Now signed number -27: %hi"n, &arg3);
    debug_printf("Uppercase hexadecimal 7095: %hX\n7095 in binary: %hb\n7095 with left padded zeros: %08u"n, &arg2, &arg2, &arg2);
    debug_printf("Goodbye!\n", NULL);

finished:
    goto finished;
}
