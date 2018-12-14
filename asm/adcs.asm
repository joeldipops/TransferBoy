INCLUDE "utils.asm"
; adc some value to A, plus the carry flag if it exists, for 16bit maths.

;adc A, A (C=0)
    ; test all the flags with A and C=0
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
    
    resetFlags
    
    ld A, 255
    adc A
    ; A=$FE Z=0, N=0, H=1, C=1
    
    resetFlags

; adc A, A (C=1)
    scf 
    ld A, 0
    adc A
    ; A=1 Z=0 N=0 H=0 C=0

    scf
    ld A, 1
    adc A
    ; A=3 Z=0 N=0 H=0 C=0

    scf
    ld A, 7
    adc A
    ; A=$0F Z=0 N=0 H=1 C=0
    
    scf
    ld A, 240
    adc A
    ; A=$E1 Z=0, N=0, H=0, C=1
    
    scf    
    ld A, 255
    adc A
    ; A=$FF Z=0, N=0, H=1, C=1
    
; adc A, B (C=0)
    resetFlags

    ; Let's test each adc op code for all flags.
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

    resetFlags

    ld A, 1
    ld B, 255
    adc B
    ; A=0 B=$ff Z=1 N=0 H=1 C=1

    resetFlags
    
    ld A, 255
    adc B
    ; A=$fe B=$ff Z=0 N=0 H=1 C=1

;adc A, B (C=1)
    resetFlags

    scf 
    ld A, 0
    ld B, 0
    adc B
    ; A=1 B=0 Z=1 N=0 H=0 C=0
    
    scf
    ld B, 7
    adc B
    ; A=8 B=7 Z=0 N=0 H=0 C=0

    scf
    ld A, 1
    adc B
    ; A=9 B=7 Z=0 N=0 H=1 C=0

    scf
    ld A, 240
    ld B, 128
    adc B
    ; A=$71 B=$80 Z=0 N=0 H=0 C=1

    scf
    ld A, 1
    ld B, 255
    adc B
    ; A=1 B=$ff Z=0 N=0 H=1 C=1

    scf    
    ld A, 255
    adc B
    ; A=$ff B=$ff Z=0 N=0 H=1 C=1

; adc A, C 
; adc A, C (C=0)
    resetFlags

    ; Let's test each adc op code for all flags.
    ld A, 0
    ld C, 0
    adc C
    ; A=0 C=0 Z=1 N=0 H=0 C=0
    ld C, 7
    adc C
    ; A=7 C=7 Z=0 N=0 H=0 C=0

    ld A, 1
    adc C
    ; A=8 C=7 Z=0 N=0 H=1 C=0

    ld A, 240
    ld C, 128
    adc C
    ; A=$70 C=$80 Z=0 N=0 H=0 C=1

    resetFlags

    ld A, 1
    ld C, 255
    adc C
    ; A=0 C=$ff Z=1 N=0 H=1 C=1

    resetFlags
    
    ld A, 255
    adc C
    ; A=$fe C=$ff Z=0 N=0 H=1 C=1

;adc A, C (C=1)
    resetFlags

    scf 
    ld A, 0
    ld C, 0
    adc C
    ; A=1 C=0 Z=1 N=0 H=0 C=0
    
    scf
    ld C, 7
    adc C
    ; A=8 C=7 Z=0 N=0 H=0 C=0

    scf
    ld A, 1
    adc C
    ; A=9 C=7 Z=0 N=0 H=1 C=0

    scf
    ld A, 240
    ld C, 128
    adc C
    ; A=$71 C=$80 Z=0 N=0 H=0 C=1

    scf
    ld A, 1
    ld C, 255
    adc C
    ; A=1 C=$ff Z=0 N=0 H=1 C=1

    scf    
    ld A, 255
    adc C
    ; A=$ff C=$ff Z=0 N=0 H=1 C=1'

; adc A, D
; adc A, D (C=0)
    resetFlags

    ; Let's test each adc op code for all flags.
    ld A, 0
    ld D, 0
    adc D
    ; A=0 D=0 Z=1 N=0 H=0 C=0
    ld D, 7
    adc D
    ; A=7 D=7 Z=0 N=0 H=0 C=0

    ld A, 1
    adc D
    ; A=8 D=7 Z=0 N=0 H=1 C=0

    ld A, 240
    ld D, 128
    adc D
    ; A=$70 D=$80 Z=0 N=0 H=0 C=1

    resetFlags

    ld A, 1
    ld D, 255
    adc D
    ; A=0 D=$ff Z=1 N=0 H=1 C=1

    resetFlags
    
    ld A, 255
    adc D
    ; A=$fe D=$ff Z=0 N=0 H=1 C=1

;adc A, D (C=1)
    resetFlags

    scf 
    ld A, 0
    ld D, 0
    adc D
    ; A=1 D=0 Z=1 N=0 H=0 C=0
    
    scf
    ld D, 7
    adc D
    ; A=8 D=7 Z=0 N=0 H=0 C=0

    scf
    ld A, 1
    adc D
    ; A=9 D=7 Z=0 N=0 H=1 C=0

    scf
    ld A, 240
    ld D, 128
    adc D
    ; A=$71 D=$80 Z=0 N=0 H=0 C=1

    scf
    ld A, 1
    ld D, 255
    adc D
    ; A=1 D=$ff Z=0 N=0 H=1 C=1

    scf    
    ld A, 255
    adc D
    ; A=$ff D=$ff Z=0 N=0 H=1 C=1

; adc A, E
; adc A, E (C=0)
    resetFlags

    ; Let's test each adc op code for all flags.
    ld A, 0
    ld E, 0
    adc E
    ; A=0 E=0 Z=1 N=0 H=0 C=0
    ld E, 7
    adc E
    ; A=7 E=7 Z=0 N=0 H=0 C=0

    ld A, 1
    adc E
    ; A=8 E=7 Z=0 N=0 H=1 C=0

    ld A, 240
    ld E, 128
    adc E
    ; A=$70 E=$80 Z=0 N=0 H=0 C=1

    resetFlags

    ld A, 1
    ld E, 255
    adc E
    ; A=0 E=$ff Z=1 N=0 H=1 C=1

    resetFlags
    
    ld A, 255
    adc E
    ; A=$fe E=$ff Z=0 N=0 H=1 C=1

;adc A, E (C=1)
    resetFlags

    scf 
    ld A, 0
    ld E, 0
    adc E
    ; A=1 E=0 Z=1 N=0 H=0 C=0
    
    scf
    ld E, 7
    adc E
    ; A=8 E=7 Z=0 N=0 H=0 C=0

    scf
    ld A, 1
    adc E
    ; A=9 E=7 Z=0 N=0 H=1 C=0

    scf
    ld A, 240
    ld E, 128
    adc E
    ; A=$71 E=$80 Z=0 N=0 H=0 C=1

    scf
    ld A, 1
    ld E, 255
    adc E
    ; A=1 E=$ff Z=0 N=0 H=1 C=1

    scf    
    ld A, 255
    adc E
    ; A=$ff E=$ff Z=0 N=0 H=1 C=1

; adc A, H
; adc A, H (C=0)
    resetFlags

    ; Let's test each adc op code for all flags.
    ld A, 0
    ld H, 0
    adc H
    ; A=0 H=0 Z=1 N=0 H=0 C=0
    ld H, 7
    adc H
    ; A=7 H=7 Z=0 N=0 H=0 C=0

    ld A, 1
    adc H
    ; A=8 H=7 Z=0 N=0 H=1 C=0

    ld A, 240
    ld H, 128
    adc H
    ; A=$70 H=$80 Z=0 N=0 H=0 C=1

    resetFlags

    ld A, 1
    ld H, 255
    adc H
    ; A=0 H=$ff Z=1 N=0 H=1 C=1

    resetFlags
    
    ld A, 255
    adc H
    ; A=$fe H=$ff Z=0 N=0 H=1 C=1

;adc A, H (C=1)
    resetFlags

    scf 
    ld A, 0
    ld H, 0
    adc H
    ; A=1 H=0 Z=1 N=0 H=0 C=0
    
    scf
    ld H, 7
    adc H
    ; A=8 H=7 Z=0 N=0 H=0 C=0

    scf
    ld A, 1
    adc H
    ; A=9 H=7 Z=0 N=0 H=1 C=0

    scf
    ld A, 240
    ld H, 128
    adc H
    ; A=$71 H=$80 Z=0 N=0 H=0 C=1

    scf
    ld A, 1
    ld H, 255
    adc H
    ; A=1 H=$ff Z=0 N=0 H=1 C=1

    scf    
    ld A, 255
    adc H
    ; A=$ff H=$ff Z=0 N=0 H=1 C=1

; adc A, L (C=0)
    resetFlags

    ; Let's test each adc op code for all flags.
    ld A, 0
    ld L, 0
    adc L
    ; A=0 L=0 Z=1 N=0 H=0 C=0
    ld L, 7
    adc L
    ; A=7 L=7 Z=0 N=0 H=0 C=0

    ld A, 1
    adc L
    ; A=8 L=7 Z=0 N=0 H=1 C=0

    ld A, 240
    ld L, 128
    adc L
    ; A=$70 L=$80 Z=0 N=0 H=0 C=1

    resetFlags

    ld A, 1
    ld L, 255
    adc L
    ; A=0 L=$ff Z=1 N=0 H=1 C=1

    resetFlags
    
    ld A, 255
    adc L
    ; A=$fe L=$ff Z=0 N=0 H=1 C=1

;adc A, L (C=1)
    resetFlags

    scf 
    ld A, 0
    ld L, 0
    adc L
    ; A=1 L=0 Z=1 N=0 H=0 C=0
    
    scf
    ld L, 7
    adc L
    ; A=8 L=7 Z=0 N=0 H=0 C=0

    scf
    ld A, 1
    adc L
    ; A=9 L=7 Z=0 N=0 H=1 C=0

    scf
    ld A, 240
    ld L, 128
    adc L
    ; A=$71 L=$80 Z=0 N=0 H=0 C=1

    scf
    ld A, 1
    ld L, 255
    adc L
    ; A=1 L=$ff Z=0 N=0 H=1 C=1

    scf    
    ld A, 255
    adc L
    ; A=$ff L=$ff Z=0 N=0 H=1 C=1

    ; Time to read from memory
    ld H, $c0
    ld L, $10

    ld A, 0
    ld [HL], 0
    adc [HL]
    ; A=0 Z=1 N=0 H=0 C=0
    ld [HL], 7
    adc [HL]
    ; A=7 Z=0 N=0 H=0 C=0

    ld A, 1
    adc [HL]
    ; A=8 Z=0 N=0 H=1 C=0

    ld A, 240
    ld [HL], 128
    adc [HL]
    ; A=$70 Z=0 N=0 H=0 C=1

    resetFlags

    ld A, 1
    ld [HL], 255
    adc [HL]
    ; A=0 Z=1 N=0 H=1 C=1

    resetFlags

    ld A, 255
    adc [HL]
    ; A=$fe [HL]=$ff Z=0 N=0 H=1 C=1

    resetFlags
    
    ld H, $c0
    ld L, $10

    scf
    ld A, 0
    ld [HL], 0
    adc [HL]
    ; A=1 Z=0 N=0 H=0 C=0
    
    scf
    ld [HL], 7
    adc [HL]
    ; A=8 Z=0 N=0 H=0 C=0

    scf
    ld A, 1
    adc [HL]
    ; A=9 Z=0 N=0 H=1 C=0

    scf
    ld A, 240
    ld [HL], 128
    adc [HL]
    ; A=$71 Z=0 N=0 H=0 C=1

    scf
    ld A, 1
    ld [HL], 255
    adc [HL]
    ; A=1 Z=1 N=0 H=1 C=1

    scf
    ld A, 255
    adc [HL]
    ; A=$ff [HL]=$ff Z=0 N=0 H=1 C=1

    ; adc literals
    resetFlags

    ld A, 0
    adc 0
    ; A=0 Z=1 N=0 H=0 C=0

    adc 7
    ; A=7 L=7 Z=0 N=0 H=0 C=0

    adc 1
    ; A=8 L=7 Z=0 N=0 H=1 C=0

    ld A, 240
    adc 128
    ; A=$70 L=$80 Z=0 N=0 H=0 C=1

    ld A, 1
    adc 255
    ; A=0 L=$ff Z=1 N=0 H=1 C=1

    ld A, 255
    adc 255
    ; A=$fe L=$ff Z=0 N=0 H=1 C=1

    scf
    ld A, 0
    adc 0
    ; A=1 Z=1 N=0 H=0 C=0

    scf
    adc 7
    ; A=8 L=7 Z=0 N=0 H=0 C=0

    scf
    adc 1
    ; A=9 L=7 Z=0 N=0 H=1 C=0

    scf
    ld A, 240
    adc 128
    ; A=$71 L=$80 Z=0 N=0 H=0 C=1

    scf
    ld A, 1
    adc 255
    ; A=1 L=$ff Z=1 N=0 H=1 C=1

    scf
    ld A, 255
    adc 255
    ; AF=$ff03 BC=$ffff DE=$ffff HL=$c010 ZNHC=0011