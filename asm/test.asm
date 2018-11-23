INCLUDE "gbhw.inc"
SECTION "Vblank",HOME[$0040]
    reti
SECTION "LCDC",HOME[$0048]
    reti
SECTION "Timer_Overflow",HOME[$0050]
    reti
SECTION "Serial",HOME[$0058]
    reti
SECTION "p1thru4",HOME[$0060]
    reti

SECTION "start",HOME[$0100]
    nop
    jp begin

    ROM_HEADER ROM_NOMBC, ROM_SIZE_32KBYTE, RAM_SIZE_0KBYTE

begin:
    ld A, 1
    ld B, 2
    ld C, 3
    ld D, 4
    ld E, 5
    ld H, 6
    ld L, 7

    ld A, A ; it's in the spec soo...
    ld A, B
    ld A, C
    ld A, D
    ld A, E
    ld A, H
    ld A, L

    ; put some value HL = 7
    ld (HL), A
    ld A, 0
    ld A, (HL) ; A should be 7


    ld B, A
    ld B, B
    ld B, C
    ld B, D
    ld B, E
    ld B, H
    ld B, L

    ld C, A
    ; etc

