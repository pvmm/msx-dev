; VDP copy direction (high byte):
; bit #5,#4: b00 = VRAM->VRAM
; bit #3: b0 = top  -> bottom
; bit #2: b0 = left -> right
; DIR_DEFAULT = 0b0000 << 8
; 
; pixel operation (vdpop):
; PO_IMP   = 0b0000
; PO_AND   = 0b0001
; PO_OR    = 0b0010
; PO_XOR   = 0b0011
; PO_NOT   = 0b0100
; PO_TIMP  = 0b1000
; PO_TAND  = 0b1001
; PO_TOR   = 0b1010
; PO_TXOR  = 0b1011
; PO_TNOT  = 0b1100
;
; void vdp(uint16_t sx, uint16_t sy, uint16_t dx, uint16_t dy, uint16_t width, uint16_t height, uint16_t direction, uint8_t vdpop) __sdcccall(0)

.globl _vdp

;*********************************************************************
;*  VDP operation
;*  sx: X coordinate of pixel source (left side)
;*  sy: Y coordinate of pixel source (upper side)
;*  dx: X coordinate of pixel destination (left side)
;*  dy: Y coordinate of pixel destination (upper side)
;*  width: number of pixels to the right
;*  height: number of pixels to the bottom
;*  direction: see DIR_DEFAULT in msx2.h
;*  vdpop: vdp + pixel operations
;*********************************************************************

_vdp::
	di                      ; disable interrupts
	call _wait_vdp          ; check if previous VDP command finished or wait if necessary

	ld a, #0 + #32          ; auto-increment is active
	out (#0x99), a          ; define VDP register to write to (R#32)

	ld a, #128 + #17
	out (#0x99), a          ; R#17 is used to indirectly access register #32

	ld c, #0x9b             ; VDP port for writing

	ld hl, #2
	add hl, sp              ; outi source address

	outi                    ; sx: low byte (R#32)
	outi                    ; sx: high byte (R#33)
	outi                    ; sy: low byte (R#34)
	outi                    ; sy: high byte (R#35)
	outi                    ; dx: low byte (R#36)
	outi                    ; dx: high byte (R#37)
	outi                    ; dy: low byte (R#38)
	outi                    ; dy: low byte (R#39)
	outi                    ; width: low byte (R#40)
	outi                    ; width: high byte (R#41)
	outi                    ; height: low byte (R#42)
	outi                    ; height: high byte (R#43)
	outi                    ; spurious write operation using direction LSB (R#44)
	outi                    ; direction MSB (R#45)
	outi                    ; VDP operation | pixel operation (R#46)
	ei                      ; enable interrupts

	ret
