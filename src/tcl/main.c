#include <stdint.h>
#include "../printf/xxtoa.h"
#include "../printf/printf.h"

// Send command to Tcl engine
void tcl(char* s) {
    (void*)s; // hl <- s
    __asm
        ld a, l
        out (#0x2E), a
        ld a, h
        out (#0x2E), a
    __endasm;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("No arguments provided.\n");
        return 1;
    }

    // Concatenate all command line parameters
    for (int i = 1; i < argc; i++) {
        argv[i][-1] = ' ';
    }

    tcl(argv[1]);
    return 0;
}
