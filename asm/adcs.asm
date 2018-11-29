INCLUDE "utils.asm"
; Add some value to A, plus the carry flag if it exists, for 16bit maths.

    ; test all the flags with ld A
    ld A, 0
    adc A
    ; A=0 Z=1 N=0 H=0 C=0

    ld A, 1
    adc A
    ; A=2 Z=0 N=0 H=0 C=0
    ld A, 7
    adc A
    ; A=$0E Z=0 N=0 H=1 C=0
    ld A, 240
    adc A
    ; A=$E0 Z=0, N=0, H=0, C=1
    
    call resetFlags
    
    ld A, 255
    adc A
    ; A=$FE Z=0, N=0, H=1, C=1
    
    call resetFlags

    ; Let's test each add op code for all flags.
    ld A, 0
    ld B, 0
    adc B
    ; A=0 B=0 Z=1 N=0 H=0 C=0
    ld B, 7
    adc B
    ; A=7 B=7 Z=0 N=0 H=0 C=0

    ld A, 1
    adc B
    ; A=8 B=7 Z=0 N=0 H=1 C=0

    ld A, 240
    ld B, 128
    adc B
    ; A=$70 B=$80 Z=0 N=0 H=0 C=1

    call resetFlags

    ld A, 1
    ld B, 255
    adc B
    ; A=0 B=$ff Z=1 N=0 H=1 C=1

    call resetFlags
    
    ld A, 255
    adc B
    ; A=$fe B=$ff Z=0 N=0 H=1 C=1

    call resetFlags
