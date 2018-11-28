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

    ; SP might be restricted to certain addresses?
    ld SP, $def0
    ; A=3 B=$12 C=$34 D=$56 E=$78 H=$9a L=$bc SP=$def0

    ld SP, HL
    ; A=3 B=$12 C=$34 D=$56 E=$78 H=$9a L=$bc SP=$9abc

    ; First time we have to worry about flags.
    ; H is set when 4 lowest bits of SP + 4 lowest bits of n are greater than 15
    ; C is set when when 8 lowest bits of SP + all of n are greater than 255

    ld HL, 0
    ld HL, SP + 5
    ; A=3 B=$12 C=$34 D=$56 E=$78 H=$9a L=$c1 SP=$9abc Z=0 N=0 H=1 C=0

    ld SP, $0000
    ld HL, SP + 15
    ; A=3 B=$12 C=$34 D=$56 E=$78 H=$00 L=$0F SP=$0000 Z=0 N=0 H=0 C=0

    ld HL, SP + 16
    ; A=3 B=$12 C=$34 D=$56 E=$78 H=$00 L=$10 SP=$0000 Z=0 N=0 H=1 C=0
    ld HL, SP + 240
    ; A=3 B=$12 C=$34 D=$56 E=$78 H=$00 L=$F0 SP=$0000 Z=0 N=0 H=0 C=0
    ld HL, SP + 255
    ; A=3 B=$12 C=$34 D=$56 E=$78 H=$00 L=$FF SP=$0000 Z=0 N=0 H=1 C=0
    ld SP, 1
    ld HL, SP + 255
    ; A=3 B=$12 C=$34 D=$56 E=$78 H=$01 L=$00 SP=$0000 Z=0 N=0 H=1 C=1

    ld SP, 240
    ld HL, SP + 247
    ; A=3 B=$12 C=$34 D=$56 E=$78 H=$01 L=$E7 SP=$0000 Z=0 N=0 H=0 C=1

    ld SP, $fedc
    ld [$ff0a], SP
    ld A, [$ff0a]
    ld C, A
    ld A, [$ff0b]
    ld B, A
    ; B and C /may/ be swapped, but I'm pretty sure this is right.
    ; A=$fe B=$fe C=$dc D=$56 E=$78 H=$01 L=$E7 SP=$fedc Z=0 N=0 H=0 C=1

    ld SP, $fffe

    ; This should swap the registers around.
    push AF
    push BC
    push DE
    push HL
    ; A=$fe B=$fe C=$dc D=$56 E=$78 H=$01 L=$E7 SP=$fff8 Z=0 N=0 H=0 C=1

    pop AF
    pop BC
    pop DE
    pop HL
    ; A=$01 B=$56 C=$78 D=$fe E=$dc H=$fe L=$10 SP=$fffe Z=1 N=1 H=1 C=0


    ; 8 BIT MATHS
    ; test all the flags with ld A
    ld A, 0
    add A
    ; A=0 Z=1 N=0 H=0 C=0

    ld A, 1
    add A
    ; A=2 Z=0 N=0 H=0 C=0
    ld A, 7
    add A
    ; A=$0E Z=0 N=0 H=1 C=0
    ld A, 240
    add A
    ; A=$E0 Z=0, N=0, H=0, C=1

    ld A, 255
    add A
    ; A=$FE Z=0, N=0, H=1, C=1

    ; Let's test each add op code for all flags.
    ld A, 0
    ld B, 0
    add B
    ; A=0 B=0 Z=1 N=0 H=0 C=0
    ld B, 7
    add B
    ; A=7 B=7 Z=0 N=0 H=0 C=0

    ld A, 1
    add B
    ; A=8 B=7 Z=0 N=0 H=1 C=0

    ld A, 240
    ld B, 128
    add B
    ; A=$70 B=$80 Z=0 N=0 H=0 C=1

    ld A, 1
    ld B, 255
    add B
    ; A=0 B=$ff Z=1 N=0 H=1 C=1

    ld A, 255
    add B
    ; A=$fe B=$ff Z=0 N=0 H=1 C=1

    ; ADD Cs
    ld A, 0
    ld C, 0
    add C
    ; A=0 C=0 Z=1 N=0 H=0 C=0
    ld C, 7
    add C
    ; A=7 C=7 Z=0 N=0 H=0 C=0

    ld A, 1
    add C
    ; A=8 C=7 Z=0 N=0 H=1 C=0

    ld A, 240
    ld C, 128
    add C
    ; A=$70 C=$80 Z=0 N=0 H=0 C=1

    ld A, 1
    ld C, 255
    add C
    ; A=0 C=$ff Z=1 N=0 H=1 C=1

    ld A, 255
    add C
    ; A=$fe C=$ff Z=0 N=0 H=1 C=1

    ; D
    ld A, 0
    ld D, 0
    add D
    ; A=0 D=0 Z=1 N=0 H=0 C=0
    ld D, 7
    add D
    ; A=7 D=7 Z=0 N=0 H=0 C=0

    ld A, 1
    add D
    ; A=8 D=7 Z=0 N=0 H=1 C=0

    ld A, 240
    ld D, 128
    add D
    ; A=$70 D=$80 Z=0 N=0 H=0 C=1

    ld A, 1
    ld D, 255
    add D
    ; A=0 D=$ff Z=1 N=0 H=1 C=1

    ld A, 255
    add D
    ; A=$fe D=$ff Z=0 N=0 H=1 C=1

    ; E
    ld A, 0
    ld E, 0
    add E
    ; A=0 E=0 Z=1 N=0 H=0 C=0
    ld E, 7
    add E
    ; A=7 E=7 Z=0 N=0 H=0 C=0

    ld A, 1
    add E
    ; A=8 E=7 Z=0 N=0 H=1 C=0

    ld A, 240
    ld E, 128
    add E
    ; A=$70 E=$80 Z=0 N=0 H=0 C=1

    ld A, 1
    ld E, 255
    add E
    ; A=0 E=$ff Z=1 N=0 H=1 C=1

    ld A, 255
    add E
    ; A=$fe E=$ff Z=0 N=0 H=1 C=1

; H
    ld A, 0
    ld H, 0
    add H
    ; A=0 H=0 Z=1 N=0 H=0 C=0
    ld H, 7
    add H
    ; A=7 H=7 Z=0 N=0 H=0 C=0

    ld A, 1
    add H
    ; A=8 H=7 Z=0 N=0 H=1 C=0

    ld A, 240
    ld H, 128
    add H
    ; A=$70 H=$80 Z=0 N=0 H=0 C=1

    ld A, 1
    ld H, 255
    add H
    ; A=0 H=$ff Z=1 N=0 H=1 C=1

    ld A, 255
    add H
    ; A=$fe H=$ff Z=0 N=0 H=1 C=1

; L
    ld A, 0
    ld L, 0
    add L
    ; A=0 L=0 Z=1 N=0 H=0 C=0
    ld L, 7
    add L
    ; A=7 L=7 Z=0 N=0 H=0 C=0

    ld A, 1
    add L
    ; A=8 L=7 Z=0 N=0 H=1 C=0

    ld A, 240
    ld L, 128
    add L
    ; A=$70 L=$80 Z=0 N=0 H=0 C=1

    ld A, 1
    ld L, 255
    add L
    ; A=0 L=$ff Z=1 N=0 H=1 C=1

    ld A, 255
    add L
    ; A=$fe L=$ff Z=0 N=0 H=1 C=1

    ; A=$fe B=$ff C=$ff D=$ff E=$ff H=$ff L=$ff SP=$fffe Z=0 N=0 H=1 C=1

    ; Time to read from memory
    ld H, $c0
    ld L, $10

    ld A, 0
    ld [HL], 0
    add [HL]
    ; A=0 Z=1 N=0 H=0 C=0
    ld [HL], 7
    add [HL]
    ; A=7 Z=0 N=0 H=0 C=0

    ld A, 1
    add [HL]
    ; A=8 Z=0 N=0 H=1 C=0

    ld A, 240
    ld [HL], 128
    add [HL]
    ; A=$70 Z=0 N=0 H=0 C=1

    ld A, 1
    ld [HL], 255
    add [HL]
    ; A=0 Z=1 N=0 H=1 C=1

    ld A, 255
    add [HL]
    ; A=$fe [HL]=$ff Z=0 N=0 H=1 C=1

    ; Add literals
    ld A, 0
    add 0
    ; A=0 Z=1 N=0 H=0 C=0

    add 7
    ; A=7 L=7 Z=0 N=0 H=0 C=0

    add 1
    ; A=8 L=7 Z=0 N=0 H=1 C=0

    ld A, 240
    add 128
    ; A=$70 L=$80 Z=0 N=0 H=0 C=1

    ld A, 1
    add 255
    ; A=0 L=$ff Z=1 N=0 H=1 C=1

    ld A, 255
    add 255
    ; A=$fe L=$ff Z=0 N=0 H=1 C=1


    ;
