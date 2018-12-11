INCLUDE "utils.asm"

    ld A, 0
    sub A
    ; AF=$00xx ZNHC=1100

    call resetFlags
    ld A, 2
    sub A
    ; AF=$00xx ZNHC=1100

    call resetFlags
    ld A, 2    
    ld B, 0
    sub B
    ; AF=02xx BC=00xx ZNHC=0100

    call resetFlags
    ld B, 2
    sub B
    ; AF=00xx BC=02xx ZNHC=1100

    call resetFlags
    sub B
    ; AF=fexx BC=02xx ZNHC=0101

    call resetFlags
    ld A, 8
    ld B, $0c
    sub B
    ; AF=fcxx BC=0cxx ZNHC=0111

    call resetFlags
    ld A, $88
    sub B
    ; AF=7cxx BC=0cxx ZNHC=0110

; sub A, C
    call resetFlags
    ld A, 2    
    ld C, 0
    sub C
    ; AF=02xx BC=0c00 ZNHC=0100

    call resetFlags
    ld C, 2
    sub C
    ; AF=00xx BC=0c02 ZNHC=1100

    call resetFlags
    sub C
    ; AF=fexx BC=0c02 ZNHC=0101

    call resetFlags
    ld A, 8
    ld C, $0c
    sub C
    ; AF=fcxx BC=0c0c ZNHC=0111

    call resetFlags
    ld A, $88
    sub C
    ; AF=7cfxx BC=0c0c ZNHC=0110

; sub A, D
    call resetFlags
    ld A, 2    
    ld D, 0
    sub D
    ; AF=02xx BC=0c00 DE=00xx ZNHC=0100

    call resetFlags
    ld D, 2
    sub D
    ; AF=00xx BC=0c02 DE=02xx ZNHC=1100

    call resetFlags
    sub D
    ; AF=fexx BC=0c02 DE=02xx ZNHC=0101

    call resetFlags
    ld A, 8
    ld D, $0c
    sub D
    ; AF=fcxx BC=0c0c DE=0cxx ZNHC=0111

    call resetFlags
    ld A, $88
    sub D
    ; AF=7cxx BC=0c0c DE=0cxx ZNHC=0110

; sub A, E
    call resetFlags
    ld A, 2    
    ld E, 0
    sub E
    ; AF=02xx BC=0c0c DE=0c00 ZNHC=0100

    call resetFlags
    ld E, 2
    sub E
    ; AF=00xx BC=0c0c DE=0c02 ZNHC=1100

    call resetFlags
    sub E
    ; AF=fexx BC=0c0c DE=0c02 ZNHC=0101

    call resetFlags
    ld A, 8
    ld E, $0c
    sub E
    ; AF=fcxx BC=0c0c DE=0c0c ZNHC=0111

    call resetFlags
    ld A, $88
    sub E
    ; AF=7cxx BC=0c0c DE=0c0c ZNHC=0110

; sub A, H
    call resetFlags
    ld A, 2    
    ld H, 0
    sub H
    ; AF=02xx BC=0c0c DE=0c0c HL=00xx ZNHC=0100

    call resetFlags
    ld H, 2
    sub H
    ; AF=00xx BC=0c0c DE=0c0c HL=02xx ZNHC=1100

    call resetFlags
    sub H
    ; AF=fexx BC=0c0c DE=0c0c HL=02xx ZNHC=0101

    call resetFlags
    ld A, 8
    ld H, $0c
    sub H
    ; AF=fcxx BC=0c0c DE=0c0c HL=0cxx ZNHC=0111

    call resetFlags
    ld A, $88
    sub H
    ; AF=7cxx BC=0c0c DE=0c0c HL=0cxx ZNHC=0110

; sub A, L
    call resetFlags
    ld A, 2    
    ld L, 0
    sub L
    ; AF=02xx BC=0c0c DE=0c0c HL=0c00 ZNHC=0100

    call resetFlags
    ld L, 2
    sub L
    ; AF=00xx BC=0c0c DE=0c0c HL=0c02 ZNHC=1100

    call resetFlags
    sub L
    ; AF=fexx BC=0c0c DE=0c0c HL=0c02 ZNHC=0101

    call resetFlags
    ld A, 8
    ld L, $0c
    sub L
    ; AF=fcxx BC=0c0c DE=0c0c HL=0c0c ZNHC=0111

    call resetFlags
    ld A, $88
    sub L
    ; AF=7cxx BC=0c0c DE=0c0c HL=0c0c ZNHC=0110

;sub A, [HL]
    ld H, $c1
    ld L, $23
    call resetFlags
    ld A, 2    
    ld [HL], 0
    sub [HL]
    ; AF=02xx BC=0c0c DE=0c0c HL=c123 ZNHC=0100

    call resetFlags
    ld [HL], 2
    sub [HL]
    ; AF=00xx BC=0c0c DE=0c0c HL=c123 ZNHC=1100

    call resetFlags
    sub [HL]
    ; AF=fexx BC=0c0c DE=0c0c HL=c123 ZNHC=0101

    call resetFlags
    ld A, 8
    ld [HL], $0c
    sub [HL]
    ; AF=fcxx BC=0c0c DE=0c0c HL=c123 ZNHC=0111

    call resetFlags
    ld A, $88
    sub [HL]
    ; AF=7cxx BC=0c0c DE=0c0c HL=c123 ZNHC=0110

;sub A, n
    call resetFlags
    ld A, 2    
    sub 0
    ; AF=02xx BC=0c0c DE=0c0c HL=c123 ZNHC=0100

    call resetFlags
    sub 2
    ; AF=00xx BC=0c0c DE=0c0c HL=c123 ZNHC=1100

    call resetFlags
    sub 2
    ; AF=fexx BC=0c0c DE=0c0c HL=c123 ZNHC=0101

    call resetFlags
    ld A, 8
    sub $0c
    ; AF=fcxx BC=0c0c DE=0c0c HL=c123 ZNHC=0111

    call resetFlags
    ld A, $88
    sub $0c
    ; AF=7cxx BC=0c0c DE=0c0c HL=c123 ZNHC=0110

