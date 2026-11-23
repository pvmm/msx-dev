// msxmvl EXTENSION module: unzx0 -- runtime ZX0 decompressor.
//
// Decompressor for the ZX0 format by Einar Saukas (format only; independent
// implementation). Clean-room: the RAM decoder below was written from the ZX0 v2
// format (interlaced Elias-gamma lengths, LZ literals/copies, last-offset reuse),
// with the reference decoder used only as a behavioural oracle. See
// docs/Compression.md for the measured size/speed comparison.
//
// Toolchain: SDCC --sdcccall 1. arg1 -> HL, arg2 -> DE, so UnZX0's naked body is
// the decoder itself -- no argument shuffling and no wrapper, which is why the
// C-callable routine is the same 69 bytes as the bare reference core.
#include "unzx0.h"

//=============================================================================
// RAM DECODER (hand-written, the measured hot path)
//=============================================================================
// Register use across the loop: HL = compressed source, DE = output cursor,
// BC = run length / LDIR counter, A = an 8-bit MSB-first bit reservoir (the
// 0x80 seed doubles as an "empty, reload" sentinel), and the last match offset
// kept on the stack as a negative 16-bit value (so `add hl,de` yields dst-off).
void UnZX0(const u8* src, u8* dst) __naked
{
	(void)src; (void)dst;   // src in HL, dst in DE (sdcccall 1)
	__asm
		ld   a, #0x80          ; reservoir empty -> first bit forces a reload
		ld   bc, #0xffff       ; default last offset = 1  (stored as -1)
		push bc
		inc  bc                ; BC = 0 for the first Elias-gamma
	zx0_literals:
		call zx0_gamma         ; BC = literal run length
		ldir                   ; emit literals straight from the stream
		add  a, a              ; control bit: 0 = reuse offset, 1 = new offset
		jr   C, zx0_newoffset
		call zx0_gamma         ; BC = match length (offset reused)
	zx0_match:
		ex   (sp), hl          ; HL = last offset (neg); park source on the stack
		push hl
		add  hl, de            ; HL = dst - offset
		ldir                   ; copy the match
		pop  hl
		ex   (sp), hl          ; restore source; offset stays on the stack
		add  a, a              ; 0 = back to literals, 1 = another new offset
		jr   NC, zx0_literals
	zx0_newoffset:
		call zx0_gamma         ; ((offset-1)>>7)+1 ; wraps to 256 at the end marker
		.db  0x08              ; ex af,af-shadow : stash reservoir + carry
		pop  af                ; drop the previous offset
		xor  a, a
		sub  a, c              ; A = -gamma  (0 => gamma was 256 => end of stream)
		ret  Z
		ld   b, a              ; B = high byte of the negative offset
		.db  0x08              ; ex af,af-shadow : recover reservoir + carry
		ld   c, (hl)           ; C = offset low byte from the stream
		inc  hl
		rr   b                 ; fold the first length bit out of the offset
		rr   c
		push bc                ; save the new offset (negative)
		ld   bc, #1
		call NC, zx0_gamma_more ; more length bits when that first bit was 0
		inc  bc                ; length = value + 1
		jr   zx0_match
	;; interlaced Elias-gamma: value >= 1 into BC, MSB-first from the reservoir
	zx0_gamma:
		inc  c
	zx0_gamma_loop:
		add  a, a
		jr   NZ, zx0_gamma_bit
		ld   a, (hl)           ; reservoir spent: pull the next 8 bits
		inc  hl
		rla
	zx0_gamma_bit:
		ret  C
	zx0_gamma_more:
		add  a, a
		rl   c
		rl   b
		jr   zx0_gamma_loop
	__endasm;
}

//=============================================================================
// FAST RAM DECODER (bigger, ~19% faster; still ROM-safe)
//=============================================================================
// Same ZX0 format and byte-identical output as UnZX0, tuned for speed: the
// per-token Elias reads are inlined (no call/ret) and the last match offset lives
// in a fixed RAM word instead of on the stack, so a match copy is a plain
// `ld hl,(offset)` + `add hl,de` with no `ex (sp),hl` juggling. This is the
// structure of Einar Saukas's "turbo" decoder, but where turbo self-modifies its
// own `ld hl,imm` operand (which forces the decoder to run from RAM), we keep the
// offset in RAM DATA (`ld hl,(g_ZX0_FastOff)`) -- 1 T/byte slower than turbo but
// FULLY ROM-SAFE, at the same size. Measured (4 KB corpus, openMSX): 128 bytes,
// 72.5 T/byte vs UnZX0's 89.8 and turbo's 71.4 (turbo being RAM-only).
//
// Like UnZX0 it uses the alternate AF register; mask interrupts around the call
// if an interrupt handler clobbers AF'.

u16 g_ZX0_FastOff;   // last match offset (negative); the "turbo" self-mod slot, in RAM

// UnZX0_fast(src=HL, dst=DE): decompress a ZX0 stream into RAM, the fast path.
void UnZX0_fast(const u8* src, u8* dst) __naked
{
	(void)src; (void)dst;
	__asm
		ld   bc, #0xffff
		ld   (_g_ZX0_FastOff), bc   ; default last offset = 1  (stored as -1)
		inc  bc
		ld   a, #0x80
		jr   zf_literals
	zf_newoffset:
		inc  c                      ; new offset: high part via inlined Elias
		add  a, a
		jp   NZ, zf_newoffset_skip
		ld   a, (hl)
		inc  hl
		rla
	zf_newoffset_skip:
		call NC, zf_elias
		.db  0x08                   ; ex af,af-shadow : stash reservoir
		xor  a, a
		sub  a, c
		ret  Z                      ; end marker
		ld   b, a
		.db  0x08                   ; ex af,af-shadow : recover reservoir
		ld   c, (hl)
		inc  hl
		rr   b
		rr   c
		ld   (_g_ZX0_FastOff), bc   ; store the new offset (the turbo self-mod slot)
		ld   bc, #1
		call NC, zf_elias
		inc  bc
	zf_copy:
		push hl                     ; park source
		ld   hl, (_g_ZX0_FastOff)   ; offset (negative) straight from RAM
		add  hl, de                 ; HL = dst - offset
		ldir
		pop  hl
		add  a, a
		jr   C, zf_newoffset
	zf_literals:
		inc  c                      ; literal run length via inlined Elias
		add  a, a
		jp   NZ, zf_literals_skip
		ld   a, (hl)
		inc  hl
		rla
	zf_literals_skip:
		call NC, zf_elias
		ldir
		add  a, a
		jr   C, zf_newoffset
		inc  c                      ; reused-offset match length via inlined Elias
		add  a, a
		jp   NZ, zf_reuse_skip
		ld   a, (hl)
		inc  hl
		rla
	zf_reuse_skip:
		call NC, zf_elias
		jp   zf_copy
	;; interlaced Elias-gamma continuation (unrolled first groups, then a loop)
	zf_elias:
		add  a, a
		rl   c
		add  a, a
		jr   NC, zf_elias
		ret  NZ
		ld   a, (hl)
		inc  hl
		rla
		ret  C
		add  a, a
		rl   c
		add  a, a
		ret  C
		add  a, a
		rl   c
		add  a, a
		ret  C
		add  a, a
		rl   c
		add  a, a
		ret  C
	zf_elias_loop:
		add  a, a
		rl   c
		rl   b
		add  a, a
		jr   NC, zf_elias_loop
		ret  NZ
		ld   a, (hl)
		inc  hl
		rla
		jr   NC, zf_elias_loop
		ret
	__endasm;
}

//=============================================================================
// VRAM DECODER (streams to the VDP; correctness over raw speed)
//=============================================================================
// The reference format has no VRAM variant: LZ back-references need the already
// produced output, which here lives in VRAM, so matches are resolved by reading
// VRAM back a byte at a time (this also makes overlapping RLE-style runs fall out
// for free). Written in C for clarity -- it is a one-shot asset load, not a hot
// loop, and is not part of the measured comparison.

__sfr __at(0x98) g_ZX0_P98;   // VRAM data (auto-increment)
__sfr __at(0x99) g_ZX0_P99;   // register / address setup

// Bit reader state (module-local; UnZX0_VRAM is not re-entrant).
static const u8* s_ZX0_Src;
static u8 s_ZX0_Buf;
static u8 s_ZX0_Cnt;

static u8 ZX0_Bit(void)
{
	if (s_ZX0_Cnt == 0) { s_ZX0_Buf = *s_ZX0_Src++; s_ZX0_Cnt = 8; }
	s_ZX0_Cnt--;
	u8 b = s_ZX0_Buf >> 7;
	s_ZX0_Buf <<= 1;
	return b;
}

// interlaced Elias-gamma; returns a value >= 1 (256 marks end of stream).
static u16 ZX0_Gamma(void)
{
	u16 v = 1;
	while (ZX0_Bit() == 0)
		v = (u16)((v << 1) | ZX0_Bit());
	return v;
}

// Point the VDP at a 14-bit VRAM address for writing (wr=1) or reading (wr=0).
// On a TMS9918 (MSX1) a freshly-set read address needs a short settle before the
// data byte is valid (same as VDP_Peek_16K); the whole routine runs under DI, so
// the 0x99 write pair itself is already ISR-safe.
static void ZX0_VAddr(u16 addr, u8 wr)
{
	g_ZX0_P99 = (u8)addr;
	g_ZX0_P99 = (wr ? 0x40 : 0x00) | ((u8)(addr >> 8) & 0x3F);
#if (defined(MSX_VERSION) && (MSX_VERSION == MSX_1))
	if (!wr) { __asm nop __endasm; __asm nop __endasm; __asm nop __endasm; __asm nop __endasm; }
#endif
}

void UnZX0_VRAM(const u8* src, u16 vramDst, u16 len)
{
	u16 pos = 0;         // bytes produced so far (offset from vramDst)
	u16 lastOff = 1;
	u8  newOffset;

	s_ZX0_Src = src;
	s_ZX0_Cnt = 0;

	__asm di __endasm;   // keep the VDP address latch coherent against the ISR
#if (!defined(MSX_VERSION) || (MSX_VERSION != MSX_1))
	// R#14 = the 16 KB page of the destination (region stays inside one page).
	// A TMS9918 (MSX1) has no R#14 and only 16 KB of VRAM, so skip it there
	// (writing "register 14" would alias onto R#6).
	g_ZX0_P99 = (u8)((vramDst >> 14) & 0x07);
	g_ZX0_P99 = 0x80 | 14;
#endif

	// The stream always begins with a (possibly empty) literal run.
	for (;;) {
		u16 n = ZX0_Gamma();               // literal run length
		ZX0_VAddr(vramDst + pos, 1);
		while (n--) { g_ZX0_P98 = *s_ZX0_Src++; pos++; }

		newOffset = ZX0_Bit();             // 0 = reuse offset, 1 = new offset
		if (!newOffset) {
			n = ZX0_Gamma();               // match length (offset reused)
			goto copy;
		}

	new_offset:
		{
			u16 g = ZX0_Gamma();           // offset high part (256 => done)
			if (g == 256) break;
			u8 lsb = *s_ZX0_Src++;
			u8 hi  = (u8)(0u - g);         // negative-offset high byte
			u8 nb  = (u8)(0x80 | (hi >> 1));
			u8 nc  = (u8)(((hi & 1) << 7) | (lsb >> 1));
			u8 cout = lsb & 1;             // first length bit
			u16 negOff = (u16)((nb << 8) | nc);
			lastOff = (u16)(0x10000u - negOff);
			if (cout) {
				n = 2;
			} else {
				u16 v = 1;
				do { v = (u16)((v << 1) | ZX0_Bit()); } while (ZX0_Bit() == 0);
				n = (u16)(v + 1);
			}
		}

	copy:
		{
			u16 rp = vramDst + pos - lastOff;   // match source in VRAM
			while (n--) {
				u8 v;
				ZX0_VAddr(rp, 0); v = g_ZX0_P98;      // read one byte back
				ZX0_VAddr(vramDst + pos, 1); g_ZX0_P98 = v;
				rp++; pos++;
			}
		}

		if (ZX0_Bit()) goto new_offset;    // else fall back to the literal run
	}

	(void)len;                             // output size is implicit in the stream
	__asm ei __endasm;
}

//=============================================================================
// VRAM DECODER for 128 KB VRAM (assembly, MSX2+ / V9938/V9958)
//=============================================================================
// Like UnZX0_VRAM but the destination is a 17-bit VRAM address and the output may
// span 16 KB / 64 KB boundaries (up to the full 128 KB). Literals are streamed
// sequentially to the VDP (auto-increment handles boundary crossings); back-
// references are resolved by reading VRAM back a byte at a time with a full
// 17-bit address setup per access (R#14 + 14-bit port address), so overlapping
// RLE-style runs are reproduced correctly. Not re-entrant.

// State kept in RAM.
static const u8* s_ZX0_128_Src;   // compressed stream pointer
static u8  s_ZX0_128_Bit;         // bit reservoir (MSB-first, 0 = reload)
static u16 s_ZX0_128_DstL;        // destination base, bits 0-15
static u8  s_ZX0_128_DstH;        // destination base, bit 16
static u16 s_ZX0_128_PosL;        // bytes produced, bits 0-15
static u8  s_ZX0_128_PosH;        // bytes produced, bit 16
static u16 s_ZX0_128_LastOff;     // last match offset (16-bit)

// Optional source paging (MegaROM streams that span more than one 16 KB bank).
// The source lives in the 0x8000-0xBFFF window. Set g_ZX0_128_SrcBankReg to the
// mapper register that selects the bank mapped at 0x8000 (0x7000 on ASCII16,
// 0x6000 on Konami) and g_ZX0_128_SrcBank to the bank currently mapped there.
// When the source pointer would leave the window the decoder writes ++bank to
// the register and restarts the pointer at 0x8000. Leave g_ZX0_128_SrcBankReg
// at 0 (the default) for a plain, non-paged stream.
u8  g_ZX0_128_SrcBank;
u16 g_ZX0_128_SrcBankReg;

// UnZX0_VRAM_128K(src, vramDstLow, vramDstHigh, len)
//   sdcccall(1): src in HL, vramDstLow in DE, vramDstHigh at (SP+2), len at (SP+3)
void UnZX0_VRAM_128K(const u8* src, u16 vramDstLow, u8 vramDstHigh, u16 len) __naked
{
	(void)src; (void)vramDstLow; (void)vramDstHigh; (void)len;
	__asm
		; ---- init state ----
		ld   (_s_ZX0_128_Src), hl      ; src
		ld   (_s_ZX0_128_DstL), de     ; vramDstLow
		ld   hl, #2
		add  hl, sp
		ld   a, (hl)                   ; vramDstHigh
		ld   (_s_ZX0_128_DstH), a
		xor  a
		ld   (_s_ZX0_128_Bit), a       ; reservoir empty -> reload on first bit
		ld   (_s_ZX0_128_PosL), a
		ld   (_s_ZX0_128_PosL + 1), a
		ld   (_s_ZX0_128_PosH), a
		ld   hl, #1
		ld   (_s_ZX0_128_LastOff), hl  ; default last offset = 1

		di                             ; keep VDP address latch coherent

		; ---- literals: stream always starts with a literal run ----
	zx0v128_literals:
		; set VRAM write address to dst+pos for sequential literal output
		call _zx0v128_addr_dst_wr
		; BC = literal run length
		call _zx0v128_gamma
		; copy BC literals from (src) to VRAM
		ld   a, b
		or   c
		jr   z, zx0v128_lit_end
		push bc                        ; save length to add to pos
		ld   hl, (_s_ZX0_128_Src)
	zx0v128_lit_loop:
		ld   a, h
		cp   #0xC0                     ; past the end of the 0x8000 window?
		jr   nz, zx0v128_lit_have
		call _zx0v128_next_bank
	zx0v128_lit_have:
		ld   a, (hl)
		inc  hl
		out  (0x98), a
		dec  bc
		ld   a, b
		or   c
		jr   nz, zx0v128_lit_loop
		ld   (_s_ZX0_128_Src), hl
		pop  bc
		call _zx0v128_add_pos          ; pos += BC
	zx0v128_lit_end:
		; control bit: 0 = reuse offset, 1 = new offset
		call _zx0v128_bit
		jr   c, zx0v128_new_offset
		; reuse offset: read match length
		call _zx0v128_gamma
		jr   zx0v128_copy

	zx0v128_new_offset:
		; read offset high part (gamma); value 256 (BC==0x0100? gamma returns 256 as 0x0100)
		call _zx0v128_gamma
		; end marker if BC == 256
		ld   a, b
		cp   #1
		jr   nz, zx0v128_have_off
		ld   a, c
		or   a
		jr   nz, zx0v128_have_off
		; end of stream
		ei
		pop  hl                        ; ret addr
		pop  bc                        ; discard vramDstHigh + len low
		inc  sp                        ; discard len high (callee-clean 3 bytes)
		jp   (hl)

	zx0v128_have_off:
		; compute lastOff from gamma (BC) and following byte
		; hi = (u8)(0 - gamma_lowbyte_part) -- gamma in BC
		; negative offset = (0x80|(hi>>1), ((hi&1)<<7)|(lsb>>1)), cout = lsb&1
		; lastOff = 0 - negOff
		ld   a, c
		neg                            ; A = -C (low byte of gamma; B is 0 for valid non-end values)
		ld   b, a                      ; B = -gamma (high byte of negative offset before rotate)
		ld   hl, (_s_ZX0_128_Src)
		ld   a, h
		cp   #0xC0                     ; source bank exhausted?
		jr   nz, zx0v128_lsb_have
		call _zx0v128_next_bank
	zx0v128_lsb_have:
		ld   c, (hl)                   ; C = lsb from stream
		inc  hl
		ld   (_s_ZX0_128_Src), hl
		; rr B (hi), rr C (lsb) -> new B has bit7 set from 0x80 logic: nb = 0x80|(hi>>1)
		ld   a, b
		and  #1                        ; carry-in for C rotate
		rr   b
		set  7, b                      ; nb = 0x80 | (hi>>1)
		rr   c                         ; nc = ((hi&1)<<7)|(lsb>>1), carry = lsb&1 (cout)
		push af                        ; save carry (cout = first length bit)
		; negOff = BC (B=nb, C=nc) as negative; lastOff = 0 - negOff
		ld   h, b
		ld   l, c
		xor  a
		sub  l
		ld   l, a
		ld   a, #0
		sbc  a, h
		ld   h, a                      ; HL = 0 - negOff = lastOff
		ld   (_s_ZX0_128_LastOff), hl
		pop  af                        ; restore cout
		jr   c, zx0v128_len2
		; read additional length bits: v=1; do{v=(v<<1)|bit} while(!bit) ; n=v+1
		ld   bc, #1
	zx0v128_elen:
		push bc
		call _zx0v128_bit
		pop  bc
		rl   c
		rl   b                          ; v = (v<<1)|bit
		call _zx0v128_bit
		jr   nc, zx0v128_elen          ; continue while bit==0
		inc  bc                        ; n = v+1
		jr   zx0v128_copy
	zx0v128_len2:
		ld   bc, #2

	zx0v128_copy:
		; copy BC bytes from (dst+pos-lastOff) to (dst+pos) in VRAM
		ld   a, b
		or   c
		jr   z, zx0v128_copy_end
	zx0v128_copy_loop:
		push bc
		; read byte from VRAM at dst+pos-lastOff
		call _zx0v128_addr_src_rd
		in   a, (0x98)
		ld   b, a                      ; save byte
		; write byte to VRAM at dst+pos
		call _zx0v128_addr_dst_wr
		ld   a, b
		out  (0x98), a
		; pos++
		ld   hl, (_s_ZX0_128_PosL)
		inc  hl
		ld   (_s_ZX0_128_PosL), hl
		ld   a, h
		or   l
		jr   nz, zx0v128_pos_ok
		ld   a, (_s_ZX0_128_PosH)
		inc  a
		ld   (_s_ZX0_128_PosH), a
	zx0v128_pos_ok:
		pop  bc
		dec  bc
		ld   a, b
		or   c
		jr   nz, zx0v128_copy_loop
	zx0v128_copy_end:
		; control bit: 0 = literals, 1 = new offset
		call _zx0v128_bit
		jp   c, zx0v128_new_offset
		jp   zx0v128_literals

	; ---- helpers ----

	; pos += BC (17-bit)
_zx0v128_add_pos:
		ld   hl, (_s_ZX0_128_PosL)
		add  hl, bc
		ld   (_s_ZX0_128_PosL), hl
		ret  nc
		ld   a, (_s_ZX0_128_PosH)
		inc  a
		ld   (_s_ZX0_128_PosH), a
		ret

	; compute dst+pos into HL (low16) and A (bit16)
_zx0v128_dstpos:
		ld   hl, (_s_ZX0_128_PosL)
		ld   de, (_s_ZX0_128_DstL)
		add  hl, de
		ld   a, (_s_ZX0_128_PosH)
		ld   c, a
		ld   a, (_s_ZX0_128_DstH)
		adc  a, c
		ret

	; set VRAM write address to dst+pos (17-bit)
_zx0v128_addr_dst_wr:
		call _zx0v128_dstpos           ; A:HL = address
		; R#14 = (A<<2) | (H>>6)
		push af
		add  a, a
		add  a, a
		ld   c, a
		ld   a, h
		rlca
		rlca
		and  #0x03
		or   c
		out  (0x99), a
		ld   a, #0x8E
		out  (0x99), a                 ; select R#14
		ld   a, l
		out  (0x99), a                 ; A7..A0
		ld   a, h
		and  #0x3F
		or   #0x40                     ; write
		out  (0x99), a                 ; A13..A8
		pop  af
		ret

	; set VRAM read address to dst+pos-lastOff (17-bit)
_zx0v128_addr_src_rd:
		call _zx0v128_dstpos           ; A:HL = dst+pos
		ld   de, (_s_ZX0_128_LastOff)
		or   a
		sbc  hl, de
		sbc  a, #0                     ; A:HL = dst+pos-lastOff
		; R#14 = (A<<2) | (H>>6)
		push af
		add  a, a
		add  a, a
		ld   c, a
		ld   a, h
		rlca
		rlca
		and  #0x03
		or   c
		out  (0x99), a
		ld   a, #0x8E
		out  (0x99), a                 ; select R#14
		ld   a, l
		out  (0x99), a                 ; A7..A0
		ld   a, h
		and  #0x3F                    ; read
		out  (0x99), a                 ; A13..A8
		pop  af
		ret

	; read one bit into carry flag; preserves BC, DE, HL
_zx0v128_bit:
		ld   a, (_s_ZX0_128_Bit)
		or   a
		jr   z, zx0v128_bit_reload
		cp   #0x80                     ; sentinel at top: all 8 bits consumed
		jr   nz, zx0v128_bit_have
	zx0v128_bit_reload:
		; reload reservoir
		push hl
		ld   hl, (_s_ZX0_128_Src)
		ld   a, h
		cp   #0xC0                     ; source bank exhausted?
		jr   nz, zx0v128_bit_got
		call _zx0v128_next_bank
	zx0v128_bit_got:
		ld   a, (hl)
		inc  hl
		ld   (_s_ZX0_128_Src), hl
		pop  hl
		scf                            ; sentinel: after 8 shifts -> exhausted (0x80)
		rla                            ; MSB -> carry (returned bit), 1 -> LSB sentinel
		ld   (_s_ZX0_128_Bit), a
		ret
	zx0v128_bit_have:
		add  a, a                      ; MSB -> carry
		ld   (_s_ZX0_128_Bit), a
		ret

	; Source pointer ran past the 0x8000-0xBFFF window: map the next 16 KB bank
	; and restart at 0x8000. No-op when g_ZX0_128_SrcBankReg is 0.
	; Clobbers A and HL only (BC and DE are preserved); returns HL = 0x8000.
_zx0v128_next_bank:
		ld   a, (_g_ZX0_128_SrcBankReg)
		or   a
		jr   nz, zx0v128_nb_on        ; register address <> 0 -> banking on
		ld   a, (_g_ZX0_128_SrcBankReg + 1)
		or   a
		ret  z                        ; 0x0000 -> disabled, keep reading
	zx0v128_nb_on:
		ld   hl, #0x8000
		ld   a, (_g_ZX0_128_SrcBank)
		inc  a
		ld   (_g_ZX0_128_SrcBank), a
		push de
		ld   de, (_g_ZX0_128_SrcBankReg)
		ex   de, hl                    ; HL = register address, DE = 0x8000
		ld   (hl), a                   ; select the next bank
		ex   de, hl                    ; HL = 0x8000
		pop  de
		ret

	; Elias-gamma: result in BC (>=1). Uses A; preserves DE, HL.
_zx0v128_gamma:
		ld   bc, #1
	zx0v128_gamma_loop:
		call _zx0v128_bit
		ret  c                         ; terminating 1-bit
		; value bit
		push bc
		call _zx0v128_bit
		ld   a, #0
		rla                            ; A = value bit (0 or 1)
		pop  bc
		sla  c
		rl   b
		or   c
		ld   c, a                      ; BC = (BC<<1) | bit
		jr   zx0v128_gamma_loop

	__endasm;
}
