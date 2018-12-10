; 8 BIT LOADS
    ; AF=0000 BC=0000 DE=0000 HL=0000 ZNHC=0000 
    
    ld A, 1
    ld B, 2
    ld C, 3
    ld D, 4
    ld E, 5
    ld H, 6
    ld L, 7
    ; Obvious, but for completeness sake
    ; AF=0100 BC=0203 DE=0405 HL=0607 ZNHC=0000 

    ld A, A ; it's in the spec soo...
    ld A, B
    ld A, C
    ld A, D
    ld A, E
    ld A, H
    ld A, L
    ; AF=0700 BC=0203 DE=0405 HL=0607 ZNHC=0000

    ; put some value into HL = 7
    ld H, $c1
    ld L, $23

    ld A, 8
    ld [HL], A
    ld A, 0
    ld A, [HL]
    ; AF=0800 BC=0203 DE=0405 HL=c123 ZNHC=0000

    ld B, A
    ld B, B
    ld B, C
    ld B, D
    ld B, E
    ld B, H
    ld B, L
    ; AF=0800 BC=2303 DE=0405 HL=c123 ZNHC=0000

    ld B, [HL]
    ; AF=0800 BC=0803 DE=0405 HL=c123 ZNHC=0000

    ld C, A
    ld C, B
    ld C, C
    ld C, D
    ld C, E
    ld C, H
    ld C, L
    ; AF=0800 BC=0823 DE=0405 HL=c123 ZNHC=0000

    ld C, [HL]
    ; AF=0800 BC=0808 DE=0405 HL=c123 ZNHC=0000

    ld D, A
    ld D, B
    ld D, C
    ld D, D
    ld D, E
    ld D, H
    ld D, L
    ; AF=0800 BC=0808 DE=2305 HL=c123 ZNHC=0000    

    ld D, [HL]
    ; AF=0800 BC=0808 DE=0805 HL=c123 ZNHC=0000    

    ld E, A
    ld E, B
    ld E, C
    ld E, D
    ld E, E
    ld E, H
    ld E, L
    ; AF=0800 BC=0808 DE=0823 HL=c123 ZNHC=0000    

    ld E, [HL]
    ; AF=0800 BC=0808 DE=0808 HL=c123 ZNHC=0000    

    ld H, A
    ld H, B
    ld H, C
    ld H, D
    ld H, E
    ld H, H
    ld H, L
    ; AF=0800 BC=0808 DE=0808 HL=2323 ZNHC=0000

    ld L, 0
    ld L, A
    ld L, B
    ld L, C
    ld L, D
    ld L, E
    ld L, H
    ld L, L
    ; AF=0800 BC=0808 DE=0808 HL=2323 ZNHC=0000

    ld H, $c1
    ld L, $24
    ld A, 4
    ld [HL], A
    ld H, [HL]
    ; AF=0400 BC=0808 DE=0808 HL=0424 ZNHC=0000

    ld H, $c1
    ld L, $25
    ld A, 5
    ld [HL], A
    ld L, [HL]
    ; AF=0500 BC=0808 DE=0808 HL=c105 ZNHC=0000

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
    ; AF=1000 BC=0203 DE=0405 HL=c010 ZNHC=0000

    ld [HL], 3
    ld A, [HL]
    ; AF=0300 BC=0203 DE=0405 HL=c010 ZNHC=0000

    ld B, $c0
    ld C, $00
    ld A, 6
    ld [BC], A
    ld A, 0
    ld A, [BC]
    ; AF=0600 BC=c000 DE=0405 HL=c010 ZNHC=0000

    ld D, $c2
    ld E, $b2
    ld A, 5
    ld [DE], A
    ld A, 0
    ld A, [DE]
    ; AF=0500 BC=c000 DE=c2b2 HL=c010 ZNHC=0000

    ld A, 9
    ; GameBoy CPU Manual says this is a "Two Byte immediate value, LS byte first", but I'm pretty sure it's LS byte second...
    ld [$c33c], A
    ld A, 0
    ld A, [$c33c]
    ; AF=0900 BC=c000 DE=c2b2 HL=c010 ZNHC=0000

    ; ld A, [C] is an alias to ld A, [$FF00+C].  Use both to check the right memory location is being written to.
    ld C, $12
    ld A, $d
    ld [C], A
    ld A, 0
    ld A, [$ff00+C]
    ld B, A

    ld A, 0
    ld A, [C]
    ; AF=0d00 BC=0d12 DE=c2b2 HL=c010 ZNHC=0000
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
    ; AF=0100 BC=0d12 DE=c2b2 HL=c2b1 ZNHC=0000  

    ldd [HL], A
    ld A, 0
    ld A, [$c2b1]
    ; AF=0000 BC=0d12 DE=c2b2 HL=c2b0 ZNHC=0000  
  

    ldi A, [HL]
    ; A=0 B=$0d C=$12 D=4 E=5 H=$c2 L=$b1

    ld A, 3
    ldi [HL], A
    ld A, 0
    ld A, [$c2b1]
    ; A=3 B=$0d C=$12 D=4 E=5 H=$c2 L=$b2

    ; LOAD $ff00 + n
    ldh [$26], A
    ld A, 0
    ldh A, [$26]
    ; A=3 B=$0d C=$12 D=4 E=5 H=$c2 L=$b2

    ld A, 0
    ld A, [$cc03]
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
    ld [$cc0a], SP
    ld A, [$cc0a]
    ld C, A
    ld A, [$cc0b]
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
