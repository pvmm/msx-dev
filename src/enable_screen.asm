; void enable_screen()

.globl _enable_screen

ENASCR = 0x0044

_enable_screen::
        jp ENASCR

