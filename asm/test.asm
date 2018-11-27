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
    ; EIGHT BIT LOADS

    ld A, 1
    ld B, 2
    ld C, 3
    ld D, 4
    ld E, 5
    ld H, 6
    ld L, 7
    ; Obvious, but for completeness sake
    ; A=1 B=2 C=3 D=4 E=5 H=6 L=7

    ld A, A ; it's in the spec soo...
    ld A, B
    ld A, C
    ld A, D
    ld A, E
    ld A, H
    ld A, L
    ; Obvious, but for completeness sake
    ; A=7 B=2 C=3 D=4 E=5 H=6 L=7

    ; put some value into HL = 7
    ld H, $C1
    ld L, $23

    ld A, 8
    ld [HL], A
    ld A, 0
    ld A, [HL]
    ; A=8 B=2 C=3 D=4 E=5 H=c1 L=23

    ld B, A
    ld B, B
    ld B, C
    ld B, D
    ld B, E
    ld B, H
    ld B, L
    ; A=8 B=23 C=3 D=4 E=5 H=c1 L=23

    ld B, [HL]
    ; A=8 B=8 C=3 D=4 E=5 H=c1 L=23

    ld C, A
    ld C, B
    ld C, C
    ld C, D
    ld C, E
    ld C, H
    ld C, L
    ; A=8 B=8 C=23 D=4 E=5 H=c1 L=23

    ld C, [HL]
    ; A=8 B=8 C=8 D=4 E=5 H=c1 L=23

    ld D, A
    ld D, B
    ld D, C
    ld D, D
    ld D, E
    ld D, H
    ld D, L
    ; A=8 B=8 C=8 D=23 E=5 H=c1 L=23

    ld D, [HL]
    ; A=8 B=8 C=8 D=8 E=5 H=c1 L=23

    ld E, A
    ld E, B
    ld E, C
    ld E, D
    ld E, E
    ld E, H
    ld E, L
    ; A=8 B=8 C=8 D=8 E=23 H=c1 L=23

    ld E, [HL]
    ; A=8 B=8 C=8 D=8 E=8 H=c1 L=23

    ld H, A
    ld H, B
    ld H, C
    ld H, D
    ld H, E
    ld H, H
    ld H, L
    ; A=8 B=8 C=8 D=8 E=8 H=23 L=23

    ld L, 0
    ld L, A
    ld L, B
    ld L, C
    ld L, D
    ld L, E
    ld L, H
    ld L, L
    ; A=8 B=8 C=8 D=8 E=8 H=23 L=23

    ld H, $c1
    ld L, $24
    ld A, 4
    ld [HL], A
    ld H, [HL]
    ; A=4 B=8 C=8 D=8 E=8 H=c1 L=24

    ld H, $c1
    ld L, $25
    ld A, 5
    ld [HL], A
    ld L, [HL]
    ; A=5  B=8 C=8 D=8 E=8 H=c1 L=25

    ld A, 1
    ld B, 2
    ld C, 3
    ld D, 4
    ld E, 5
    ld H, $c0
    ld L, $10

    ld [HL], A
    ld [HL], B
    ld A, [HL]
    ld [HL], C
    ld A, [HL]
    ld [HL], D
    ld A, [HL]
    ld [HL], E
    ld A, [HL]
    ld [HL], H
    ld A, [HL]
    ld [HL], L
    ld A, [HL]
    ; A=$10  B=2 C=3 D=4 E=5 H=c0 L=$10

    ld [HL], 3
    ld A, [HL]
    ; A=3  B=2 C=3 D=4 E=5 H=c0 L=$10

    ld B, $c0
    ld C, $00
    ld A, 6
    ld [BC], A
    ld A, 0
    ld A, [BC]
    ; A=6  B=c0 C=0 D=4 E=5 H=c0 L=$10

    ld D, $c2
    ld E, $b2
    ld A, 5
    ld [DE], A
    ld A, 0
    ld A, [DE]
    ; A=5 B=$c0 C=0 D=$c2 E=$b2 H=$c0 L=$10

    ld A, 9
    ; GameBoy CPU Manual says this is a "Two Byte immediate value, LS byte first", but I'm pretty sure it's LS byte second...
    ld [$c33c], A    
    ld A, 0
    ld A, [$c33c]
    ; A=9 B=2 C=3 D=4 E=5 H=6 L=7

    ; ld A, [C] is an alias to ld A, [$FF00+C].  Use both to check the right memory location is being written to.
    ld C, $12
    ld A, 13
    ld [C], A

    ld A, [$ff00+C]
    ld B, A

    ld A, 0
    ld A, [C]
    ; A=13 B=$0d C=$12 D=4 E=5 H=6 L=7

    ld H, $c2
    ld L, $b2
    
    ld A, 0
    ld [$c2b0], A
    ld A, 2
    ld [$c2b1], A
    ld A, 1
    ld [$c2b2], A
    ; $c2b0 = $00 $02 $01
    
    ; LOAD and DECREMENT/INCREMENT
    
    ldd A, [HL]
    ; A=1 B=$0d C=$12 D=4 E=5 H=$c2 L=$b1
    
    ldd [HL], A
    ld A, 0
    ld A, [$c2b1]
    ; A=1 B=$0d C=$12 D=4 E=5 H=$c2 L=$b0
    
    ldi A, [HL]
    ; A=0 B=$0d C=$12 D=4 E=5 H=$c2 L=$b1
    
    ld A, 3
    ldi [HL], A
    ld A, 0
    ld A, [$c2b1]
    ; A=3 B=$0d C=$12 D=4 E=5 H=$c2 L=$b2   
    
    ; LOAD $ff00 + n
    ldh [3], A
    ld A, 0
    ldh A, [3]
    ; A=3 B=$0d C=$12 D=4 E=5 H=$c2 L=$b2
    
    ld A, 0
    ld A, [$ff03]
    ; A=3 B=$0d C=$12 D=4 E=5 H=$c2 L=$b2    

    ; SIXTEEN BIT LOADS
    
    ld BC, $1234
    ld DE, $5678
    ld HL, $9abc
    ld SP, $def0
    ; A=3 B=$12 C=$34 D=$56 E=$78 H=$9a L=$bc SP=$f0 
      
