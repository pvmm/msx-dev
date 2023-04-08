; void disable_click(void);

CLIKSW = 0xf3db

_disable_click::
    ; disable key click sound
    xor a
    ld (CLIKSW), a
    ret
