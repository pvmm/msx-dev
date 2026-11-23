// msxmvl EXTENSION module: unzx0 -- runtime ZX0 decompressor (RAM + VRAM).
//
// Decompressor for the ZX0 format by Einar Saukas (format only; independent
// implementation). Clean-room: written from the ZX0 v2 format description
// (interlaced Elias-gamma lengths + LZ literals/copies with last-offset reuse).
// Einar Saukas's reference decoder was consulted only as a behavioural oracle;
// the code here is our own. See docs/Compression.md for the measured size/speed
// comparison against the reference and the packing workflow.
//
// Pack your data on the host with the `zx0` encoder (github.com/einar-saukas/ZX0)
// in its default (forward, v2) mode; decompress on-target with these functions.
#ifndef MSXMVL_UNZX0_H
#define MSXMVL_UNZX0_H

#include "types.h"

// Decompress a ZX0 stream from `src` into RAM at `dst`. The output size is fixed
// at pack time -- size `dst` yourself; there is no bounds check (matches the
// reference decoder). Runs with interrupts left as-is; it uses the alternate AF
// register, so an interrupt handler that clobbers AF' must be masked around the
// call (again, the reference decoder's contract).
void UnZX0(const u8* src, u8* dst);

// Faster RAM decoder: byte-identical output to UnZX0, ~19% fewer T-states (128 B
// vs 69 B). It inlines the Elias reads and keeps the last match offset in a RAM
// word instead of on the stack. This is the "turbo" decode shape, but ROM-SAFE:
// unlike Einar Saukas's turbo (which self-modifies its own code and must run from
// RAM), UnZX0_fast keeps that slot in RAM DATA, so the decoder itself runs fine
// from ROM. Costs one 2-byte RAM global (g_ZX0_FastOff) and, like UnZX0, uses the
// alternate AF register. Use it when the extra ~59 bytes buy worthwhile speed.
void UnZX0_fast(const u8* src, u8* dst);

// Decompress a ZX0 stream from `src` straight into VRAM at 14-bit address
// `vramDst`, producing `len` bytes (the original uncompressed size). Back-
// references are resolved by reading VRAM back, so the whole output must lie
// within one 16 KB VRAM page (auto-increment does not cross a page). Masks
// interrupts for the duration to keep the VDP address latch coherent. Works on
// MSX1 (TMS9918) and MSX2 (V99x8).
void UnZX0_VRAM(const u8* src, u16 vramDst, u16 len);

// Decompress a ZX0 stream from `src` straight into 128 KB VRAM at 17-bit address
// (`vramDstHigh`<<16 | `vramDstLow`), producing `len` bytes. Like UnZX0_VRAM but
// handles addresses spanning the full 128 KB VRAM (crosses 16 KB boundaries).
// Back-references are resolved by reading VRAM back byte by byte. Masks interrupts
// for the duration to keep the VDP address latch coherent. Requires V9938/V9958
// (MSX2+, 128 KB VRAM). `len` is unused (the stream is self-terminating).
void UnZX0_VRAM_128K(const u8* src, u16 vramDstLow, u8 vramDstHigh, u16 len);

// Optional source paging for UnZX0_VRAM_128K. A cartridge sees only 16 KB at a
// time at 0x8000-0xBFFF, so a packed stream bigger than that (e.g. a 60 KB
// splash picture) needs its ROM bank switched while it is read. Set
// g_ZX0_128_SrcBankReg to the mapper register that selects the 0x8000 bank
// (0x7000 on ASCII16, 0x6000 on Konami) and g_ZX0_128_SrcBank to the bank that
// is mapped there when the call starts; the decoder writes ++bank to the
// register and wraps the pointer to 0x8000 whenever it would read past 0xBFFF.
// Both default to 0 in BSS, which disables paging (src must then stay inside one
// window). If your startup code does not zero BSS, set g_ZX0_128_SrcBankReg = 0
// once at init. The `src` itself may start anywhere, including a ROM bank mapped
// at 0x4000 as long as the bytes continue contiguously across 0x8000.
// *(tested: tests/panko_370_rom, which decodes a 60,528 B stream across four
// ASCII16 banks in one call and is byte-verified against the source picture.)*
extern u8  g_ZX0_128_SrcBank;
extern u16 g_ZX0_128_SrcBankReg;

#endif // MSXMVL_UNZX0_H
