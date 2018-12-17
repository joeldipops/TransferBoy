INCLUDE "gbhw.asm"

SECTION "Vblank", ROM0[$0040]
    reti
SECTION "LCDC", ROM0[$0048]
    reti
SECTION "Timer_Overflow", ROM0[$0050]
    reti
SECTION "Serial", ROM0[$0058]
    reti
SECTION "p1thru4", ROM0[$0060]
    reti

SECTION "start", ROM0[$0100]
    nop
    jp main

    ROM_HEADER ROM_NOMBC, ROM_SIZE_32KBYTE, RAM_SIZE_0KBYTE

main:
    INCLUDE "utils.asm" 
    call initialise
; LOADS
    INCLUDE "loads.asm"

; MATHS
    INCLUDE "adds.asm"
    INCLUDE "adcs.asm"
    INCLUDE "subs.asm"
    INCLUDE "sbcs.asm"
    INCLUDE "booleans.asm"
    INCLUDE "incdec.asm"
    INCLUDE "16bit.asm"
    INCLUDE "transforms.asm"
    halt
