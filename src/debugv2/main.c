#include <stdint.h>
#ifdef __SDCC
#pragma less_pedantic
#include "common.h"
#endif

int main()
{
    int a = 0;
    debug_printf("a = %i\n", a);

finished:
    goto finished;
}
