#ifndef _INPUT_H
#define _INPUT_H


#include <stdint.h>


// input source
#define SOURCE_KEYB             0
#define SOURCE_JOY1             1
#define SOURCE_JOY2             2
#define SOURCE_INVALID          3

// generic input mapping
#define IGNORED                 0
#define FIRE1                   (1 << 0)
#define FIRE2                   (1 << 2)
#define LEFT                    (1 << 4)
#define UP                      (1 << 5)
#define DOWN                    (1 << 6)
#define RIGHT                   (1 << 7)
#define UP_LEFT                 (UP + LEFT)
#define UP_RIGHT                (UP + RIGHT)
#define DOWN_LEFT               (DOWN + LEFT)
#define DOWN_RIGHT              (DOWN + RIGHT)


void disable_click(void);
uint8_t read_input(uint8_t source);


#endif // _INPUT_H
