#include <stdint.h>
#include "fusion-c.h"
#include "input.h"


uint8_t read_keyboard_row(uint8_t row) __z88dk_fastcall;


static uint8_t keyboard_read()
{
    uint8_t scan;
    /* previous row statuses */
    static uint8_t row4 = 0xff;
    static uint8_t row8 = 0xff;
    static uint8_t accum = 0xff;

    // row 4 changed?
    if ((scan = read_keyboard_row(4)) != row4) {
        // search for 'M' key
        if (!(scan & FIRE2)) accum &= ~FIRE2; else accum |= FIRE2;
    }
    row4 = scan;

    // row 8 changed?
    if ((scan = read_keyboard_row(8)) != row8) {
        // search for space and arrow keys
        if (!(scan & FIRE1)) accum &= ~FIRE1; else accum |= FIRE1;
        if (!(scan &  LEFT)) accum &= ~LEFT;  else accum |= LEFT;
        if (!(scan &    UP)) accum &= ~UP;    else accum |= UP;
        if (!(scan &  DOWN)) accum &= ~DOWN;  else accum |= DOWN;
        if (!(scan & RIGHT)) accum &= ~RIGHT; else accum |= RIGHT;
    }
    row8 = scan;

    return ~accum;
}


uint8_t read_input(uint8_t source)
{
    if (source == SOURCE_KEYB)
        return keyboard_read();

    // not implemented yet?
    return IGNORED;
}

