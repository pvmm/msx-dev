#include "put_pattern.h"
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>


// port numbers
static __sfr __at(G9K_RAM_DATA) port_p0;
static __sfr __at(G9K_CMD_DATA) port_p2;
static __sfr __at(G9K_REG_DATA) port_p3;
static __sfr __at(G9K_REG_SEL)  port_p4;
static __sfr __at(G9K_SYS_CTRL) port_p7;


void set_p1_mode(bool ntsc60hz)
{
    port_p7 = 0;
    set_register(6, 5);
    set_next_register(ntsc60hz ? 0 : 0b1000); // write to R#7
    set_register(13, 0);

    // 512kb VRAM
    set_register(8, 0b00000010);
}


// use Video9000's superimpose control support
void set_ctrl_port(uint8_t byte) __z88dk_fastcall
{
    byte;
    __asm
        ld a, l
        out (#0x6f), a
    __endasm;
}


void copy_ram_to_vram(void* address, const uint16_t size, uint32_t vram)
{
    set_register(0, vram & 0xff);                   // R#0
    set_next_register((vram >>  8) & 0xff);         // R#1
    set_next_register((vram >> 16) & 0b111);        // R#2, autoincrement

    for (uint16_t pos = 0; pos < size; ++pos) {
        port_p0 = ((uint8_t*)address)[pos];
    } 
}


/* you need to check if the register is readable */
void enable_register(uint8_t reg, uint8_t bits)
{
    port_p4 = reg;                // select register reg
    // you need to know first if the register is readable
    uint8_t tmp = port_p3 | bits; // read value and change bits
    port_p3 = tmp;                // ... and write it back
}


// use disable_registers instead of this function directly
void _disable_register(uint8_t reg, uint8_t bits)
{
    port_p4 = reg;                // select register reg
    // you need to know first if the register is readable
    uint8_t tmp = port_p3 & bits; // read value and change bits
    port_p3 = tmp;                // ... and write it back
}


void set_register(uint8_t reg, uint8_t bits)
{
    port_p4 = reg;  // select register reg
    port_p3 = bits; // ... and write data to P#3
}


inline void set_next_register(uint8_t bits)
{
    port_p3 = bits; // ... and write data to P#3
}


// use set_banks_palettes instead of this function directly
void _set_banks_palettes(uint8_t palettes)
{
    set_register(13, palettes);
}


// use set_banks_priorities instead of this function directly
void _set_banks_priorities(uint8_t priority)
{
    set_register(27, priority);
}


uint8_t vpeek(uint32_t address)
{
    set_register(3, address & 0xff);                     // R#3
    set_next_register((address >> 8) & 0xff);            // R#4
    set_next_register(0x00 | ((address >> 16) & 0b111)); // R#5, autoincrement
    return port_p0;
}


// vpeek next address used in vpeek
uint8_t vpeek_next(void)
{
    return port_p0;
}


void vpoke(uint32_t address, uint8_t value)
{
    set_register(0, address & 0xff);                     // R#0
    set_next_register((address >> 8) & 0xff);            // R#1
    set_next_register(0x00 | ((address >> 16) & 0b111)); // R#2, autoincrement
    port_p0 = value; // write value
}


// vpoke next address used in vpoke
void vpoke_next(uint8_t value)
{
    port_p0 = value;
}


void _put_pattern_a(uint16_t tile, uint16_t dst)
{
    port_p4 = G9K_WRI_MODE;
    // dst base address: 0x7C000-0x7DFFF (8191 bytes)
    port_p3 = dst & 0xff;
    port_p3 = 0xc0 + (dst >> 8);
    port_p3 = 0x07; // AII (address increment inhibit): advance to next address
    port_p0 = tile & 0xff;
    port_p0 = tile >> 8;
}


void _put_pattern_b(uint16_t tile, uint16_t dst)
{
    port_p4 = G9K_WRI_MODE;
    // dst base address: 0x7E000-0x7FFFF (8191 bytes)
    port_p3 = dst & 0xff;
    port_p3 = 0xe0 + (dst >> 8);
    port_p3 = 0x07; // AII (address increment inhibit): advance to next address
    port_p0 = tile & 0xff;
    port_p0 = tile >> 8;
}


inline void enable_interrupts(void)
{
    set_register(9, 3);
}


inline void disable_interrupts(void)
{
    set_register(9, 0);
}


// hook on H_KEYI
extern void vblank_hook(void);


void enable_vblank_hook(void)
{
    set_interrupt_handler((uint8_t(*)(void)) vblank_hook);
    init_interrupt_handler();
}


void disable_vblank_hook(void)
{
    end_interrupt_handler();
}


inline void enable_interrupt_line(uint16_t line)
{
    di;
    enable_register(10, line & 0xff);
    enable_register(11, 0x80 | (line >> 8)); // enable IEHM (7th bit)
    ei;
}


void scroll_fg_x(uint16_t value)
{
    set_register(19, value & 0x7);                       // SCAX (bits 0-2)
    set_next_register(value >> 3);                       // SCAX (buts 10-3)
}


void scroll_bg_x(uint16_t value)
{
    di;
    set_register(23, value & 0x7);                       // SCBX (bits 0-2)
    set_next_register(value >> 3);                       // SCBX (buts 10-3)
    ei;
}
