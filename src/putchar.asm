; void putchar(uint8_t c) __z88dk_fast_call

.globl _putchar

CHPUT = 0x00A2

_putchar::
	ld a, l
	jp CHPUT        ;BIOS call for display the caracter
