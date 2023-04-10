;
; collection of miscellaneous routines
;

; return (a == 127) || (a == 63) || (a == 31) || (a == 15) || (a == 7) || (a == 3) || (a == 1)
_routine_01::
    cpl
    ld  h, a
    dec h
    xor h
    add a, h
    rla
    and #1

    ret
