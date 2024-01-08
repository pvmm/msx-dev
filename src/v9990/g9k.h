#ifndef _G9K_H
#define _G9K_H


#include <stdint.h>
#include <stdbool.h>


// sprite stuff
#define MAX_SPRITES         125
#define SPRITE_VRAM_ADDRESS 0x10000
#define SPRITE_DISABLE_BIT  (1<<4)

// port defines
#define G9K_RAM_DATA                    0x60 // RW
#define G9K_PAL_DATA                    0x61 // RW
#define G9K_CMD_DATA                    0x62 // RW
#define G9K_REG_DATA                    0x63 // RW
#define G9K_REG_SEL                     0x64 // W
#define G9K_STA_DATA                    0x65 // R
#define G9K_INT_FLAG                    0x66 // RW
#define G9K_SYS_CTRL                    0x67 // W
#define G9K_SUP_CTRL                    0x6F // RW

// bit flags
#define G9K_LAYER_B_BIT                 0x40
#define G9K_LAYER_A_BIT                 0x80

// GFX9000 registers
#define G9K_WRI_MODE                    0x0
#define G9K_RED_MODE                    0x3

// extra GFX9000 stuff
#define MAX_PAL_LEN                     (64 * 3) // 64 colours * 3 components (RGB)

// v9000 control port presets
#define SUPERIMPOSE_OFF                 0x00
#define SUPERIMPOSE_TRANSPARENT         0x10
#define SUPERIMPOSE_GENLOCK             0x18


void set_p1_mode(bool ntsc60hz);
void set_ctrl_port(uint8_t byte) __z88dk_fastcall;
void enable_fg_layer(void);
void disable_fg_layer(void);
void enable_bg_layer(void);
void disable__bg_layer(void);
void scroll_fg_x(uint16_t value);
void scroll_bg_x(uint16_t value);

// register functions
void set_register(uint8_t reg, uint8_t bits);
void set_next_register(uint8_t bits);
uint8_t get_register(uint8_t reg);
void enable_register(uint8_t reg, uint8_t bits);
void _disable_register(uint8_t reg, uint8_t bits);
void enable_interrupts(void);
void disable_interrupts(void);
void enable_interrupt_line(uint16_t line);

#define disable_register(reg, bits)         _disable_register(reg, (uint8_t) ((~(bits)) & 0xff))

// palette functions
#define set_banks_palettes(b1, b2)          _set_banks_palettes(((b2) << 2) | (b1))
void _set_banks_palettes(uint8_t palettes);

// priority functions
#define set_banks_priorities(prx, pry)      _set_banks_priorities(((pry) << 2) | (prx))
void _set_banks_priorities(uint8_t priority);


//
// VRAM
//
  
void copy_ram_to_vram(void* address, uint16_t size, uint32_t vram);

uint8_t vpeek(uint32_t address);

uint8_t vpeek_next(void);

void vpoke(uint32_t address, uint8_t value);

void vpoke_next(uint8_t value);


//
// Tiles
//

#define put_pattern_a(tile, dst) do { _put_pattern_a((tile), (dst) << 1); } while(0)
void _put_pattern_a(uint16_t tile, uint16_t dst);

#define put_pattern_b(tile, dst) do { _put_pattern_b((tile), (dst) << 1); } while(0)
void _put_pattern_b(uint16_t tile, uint16_t dst);


#endif // _G9K_H
