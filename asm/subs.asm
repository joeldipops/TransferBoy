INCLUDE "utils.asm"

    ld A, 0
    sub A
    ; AF=$00xx ZNHC=1100

    resetFlags
    ld A, 2
    sub A
    ; AF=$00xx ZNHC=1100

    resetFlags
    ld A, 2    
    ld B, 0
    sub B
    ; AF=02xx BC=00xx ZNHC=0100

    resetFlags
    ld B, 2
    sub B
    ; AF=00xx BC=02xx ZNHC=1100

    resetFlags
    sub B
    ; AF=fexx BC=02xx ZNHC=0101

    resetFlags
    ld A, 8
    ld B, $0c
    sub B
    ; AF=fcxx BC=0cxx ZNHC=0111

    resetFlags
    ld A, $88
    sub B
    ; AF=7cxx BC=0cxx ZNHC=0110

; sub A, C
    resetFlags
    ld A, 2    
    ld C, 0
    sub C
    ; AF=02xx BC=0c00 ZNHC=0100

    resetFlags
    ld C, 2
    sub C
    ; AF=00xx BC=0c02 ZNHC=1100

    resetFlags
    sub C
    ; AF=fexx BC=0c02 ZNHC=0101

    resetFlags
    ld A, 8
    ld C, $0c
    sub C
    ; AF=fcxx BC=0c0c ZNHC=0111

    resetFlags
    ld A, $88
    sub C
    ; AF=7cfxx BC=0c0c ZNHC=0110

; sub A, D
    resetFlags
    ld A, 2    
    ld D, 0
    sub D
    ; AF=02xx BC=0c00 DE=00xx ZNHC=0100

    resetFlags
    ld D, 2
    sub D
    ; AF=00xx BC=0c02 DE=02xx ZNHC=1100

    resetFlags
    sub D
    ; AF=fexx BC=0c02 DE=02xx ZNHC=0101

    resetFlags
    ld A, 8
    ld D, $0c
    sub D
    ; AF=fcxx BC=0c0c DE=0cxx ZNHC=0111

    resetFlags
    ld A, $88
    sub D
    ; AF=7cxx BC=0c0c DE=0cxx ZNHC=0110

; sub A, E
    resetFlags
    ld A, 2    
    ld E, 0
    sub E
    ; AF=02xx BC=0c0c DE=0c00 ZNHC=0100

    resetFlags
    ld E, 2
    sub E
    ; AF=00xx BC=0c0c DE=0c02 ZNHC=1100

    resetFlags
    sub E
    ; AF=fexx BC=0c0c DE=0c02 ZNHC=0101

    resetFlags
    ld A, 8
    ld E, $0c
    sub E
    ; AF=fcxx BC=0c0c DE=0c0c ZNHC=0111

    resetFlags
    ld A, $88
    sub E
    ; AF=7cxx BC=0c0c DE=0c0c ZNHC=0110

; sub A, H
    resetFlags
    ld A, 2    
    ld H, 0
    sub H
    ; AF=02xx BC=0c0c DE=0c0c HL=00xx ZNHC=0100

    resetFlags
    ld H, 2
    sub H
    ; AF=00xx BC=0c0c DE=0c0c HL=02xx ZNHC=1100

    resetFlags
    sub H
    ; AF=fexx BC=0c0c DE=0c0c HL=02xx ZNHC=0101

    resetFlags
    ld A, 8
    ld H, $0c
    sub H
    ; AF=fcxx BC=0c0c DE=0c0c HL=0cxx ZNHC=0111

    resetFlags
    ld A, $88
    sub H
    ; AF=7cxx BC=0c0c DE=0c0c HL=0cxx ZNHC=0110

; sub A, L
    resetFlags
    ld A, 2    
    ld L, 0
    sub L
    ; AF=02xx BC=0c0c DE=0c0c HL=0c00 ZNHC=0100

    resetFlags
    ld L, 2
    sub L
    ; AF=00xx BC=0c0c DE=0c0c HL=0c02 ZNHC=1100

    resetFlags
    sub L
    ; AF=fexx BC=0c0c DE=0c0c HL=0c02 ZNHC=0101

    resetFlags
    ld A, 8
    ld L, $0c
    sub L
    ; AF=fcxx BC=0c0c DE=0c0c HL=0c0c ZNHC=0111

    resetFlags
    ld A, $88
    sub L
    ; AF=7cxx BC=0c0c DE=0c0c HL=0c0c ZNHC=0110

;sub A, [HL]
    ld H, $c1
    ld L, $23
    resetFlags
    ld A, 2    
    ld [HL], 0
    sub [HL]
    ; AF=02xx BC=0c0c DE=0c0c HL=c123 ZNHC=0100

    resetFlags
    ld [HL], 2
    sub [HL]
    ; AF=00xx BC=0c0c DE=0c0c HL=c123 ZNHC=1100

    resetFlags
    sub [HL]
    ; AF=fexx BC=0c0c DE=0c0c HL=c123 ZNHC=0101

    resetFlags
    ld A, 8
    ld [HL], $0c
    sub [HL]
    ; AF=fcxx BC=0c0c DE=0c0c HL=c123 ZNHC=0111

    resetFlags
    ld A, $88
    sub [HL]
    ; AF=7cxx BC=0c0c DE=0c0c HL=c123 ZNHC=0110

;sub A, n
    resetFlags
    ld A, 2    
    sub 0
    ; AF=02xx BC=0c0c DE=0c0c HL=c123 ZNHC=0100

    resetFlags
    sub 2
    ; AF=00xx BC=0c0c DE=0c0c HL=c123 ZNHC=1100

    resetFlags
    sub 2
    ; AF=fexx BC=0c0c DE=0c0c HL=c123 ZNHC=0101

    resetFlags
    ld A, 8
    sub $0c
    ; AF=fcxx BC=0c0c DE=0c0c HL=c123 ZNHC=0111

    resetFlags
    ld A, $88
    sub $0c
    ; AF=7cxx BC=0c0c DE=0c0c HL=c123 ZNHC=0110

;;;;;;;;;;;;;;;;;;;; CP ;;;;;;;;;;;;;;;;;;;;;;;;


    ld A, 0
    cp A
    ; AF=00xx ZNHC=1100

    resetFlags
    ld A, 2
    cp A
    ; AF=02xx ZNHC=1100

    resetFlags
    ld A, 2    
    ld B, 0
    cp B
    ; AF=02xx BC=00xx ZNHC=0100

    resetFlags
    ld B, 2
    cp B
    ; AF=02xx BC=02xx ZNHC=1100

    resetFlags
    ld A, 0
    cp B
    ; AF=00xx BC=02xx ZNHC=0101

    resetFlags
    ld A, 8
    ld B, $0c
    cp B
    ; AF=08xx BC=0cxx ZNHC=0111

    resetFlags
    ld A, $88
    cp B
    ; AF=88xx BC=0cxx ZNHC=0110

; cp A, C
    resetFlags
    ld A, 2    
    ld C, 0
    cp C
    ; AF=02xx BC=0c00 ZNHC=0100

    resetFlags
    ld C, 2
    cp C
    ; AF=02xx BC=0c02 ZNHC=1100

    resetFlags
    ld A, 0
    cp C
    ; AF=00xx BC=0c02 ZNHC=0101

    resetFlags
    ld A, 8
    ld C, $0c
    cp C
    ; AF=08xx BC=0c0c ZNHC=0111

    resetFlags
    ld A, $88
    cp C
    ; AF=88xx BC=0c0c ZNHC=0110

; cp A, D
    resetFlags
    ld A, 2    
    ld D, 0
    cp D
    ; AF=02xx BC=0c0c DE=00xx ZNHC=0100

    resetFlags
    ld D, 2
    cp D
    ; AF=02xx BC=0c0c DE=02xx ZNHC=1100

    resetFlags
    ld A, 0
    cp D
    ; AF=00xx BC=0c0c DE=02xx ZNHC=0101

    resetFlags
    ld A, 8
    ld D, $0c
    cp D
    ; AF=08xx BC=0c0c DE=0cxx ZNHC=0111

    resetFlags
    ld A, $88
    cp D
    ; AF=88xx BC=0c0c DE=0cxx ZNHC=0110

; cp A, E
    resetFlags
    ld A, 2    
    ld E, 0
    cp E
    ; AF=02xx BC=0c0c DE=0c00 ZNHC=0100

    resetFlags
    ld E, 2
    cp E
    ; AF=02xx BC=0c0c DE=0c02 ZNHC=1100

    resetFlags
    ld A, 0
    cp E
    ; AF=00xx BC=0c0c DE=0c02 ZNHC=0101

    resetFlags
    ld A, 8
    ld E, $0c
    cp E
    ; AF=08xx BC=0c0c DE=0c0c ZNHC=0111

    resetFlags
    ld A, $88
    cp E
    ; AF=88xx BC=0c0c DE=0c0c ZNHC=0110

; cp A, H
    resetFlags
    ld A, 2    
    ld H, 0
    cp H
    ; AF=02xx BC=0c0c DE=0c0c HL=00xx ZNHC=0100

    resetFlags
    ld H, 2
    cp H
    ; AF=02xx BC=0c0c DE=0c0c HL=02xx ZNHC=1100

    resetFlags
    ld A, 0
    cp H
    ; AF=00xx BC=0c0c DE=0c0c HL=02xx ZNHC=0101

    resetFlags
    ld A, 8
    ld H, $0c
    cp H
    ; AF=08xx BC=0c0c DE=0c0c HL=0cxx ZNHC=0111

    resetFlags
    ld A, $88
    cp H
    ; AF=88xx BC=0c0c DE=0c0c HL=0cxx ZNHC=0110

; cp A, L
    resetFlags
    ld A, 2    
    ld L, 0
    cp L
    ; AF=02xx BC=0c0c DE=0c0c HL=0c00 ZNHC=0100

    resetFlags
    ld L, 2
    cp L
    ; AF=02xx BC=0c0c DE=0c0c HL=0c02 ZNHC=1100

    resetFlags
    ld A, 0
    cp L
    ; AF=00xx BC=0c0c DE=0c0c HL=0c02 ZNHC=0101

    resetFlags
    ld A, 8
    ld L, $0c
    cp L
    ; AF=08xx BC=0c0c DE=0c0c HL=0c0c ZNHC=0111

    resetFlags
    ld A, $88
    cp L
    ; AF=88xx BC=0c0c DE=0c0c HL=0c0c ZNHC=0110

;cp A, [HL]
    ld H, $c1
    ld L, $23
    resetFlags
    ld A, 2    
    ld [HL], 0
    cp [HL]
    ; AF=02xx BC=0c0c DE=0c0c HL=c123 ZNHC=0100

    resetFlags
    ld [HL], 2
    cp [HL]
    ; AF=02xx BC=0c0c DE=0c0c HL=c123 ZNHC=1100

    resetFlags
    ld A, 0
    cp [HL]
    ; AF=00xx BC=0c0c DE=0c0c HL=c123 ZNHC=0101

    resetFlags
    ld A, 8
    ld [HL], $0c
    cp [HL]
    ; AF=08xx BC=0c0c DE=0c0c HL=c123 ZNHC=0111

    resetFlags
    ld A, $88
    cp [HL]
    ; AF=88xx BC=0c0c DE=0c0c HL=c123 ZNHC=0110

;cp A, n
    resetFlags
    ld A, 2    
    cp 0
    ; AF=02xx BC=0c0c DE=0c0c HL=c123 ZNHC=0100

    resetFlags
    cp 2
    ; AF=02xx BC=0c0c DE=0c0c HL=c123 ZNHC=1100

    resetFlags
    ld A, 0
    cp 2
    ; AF=00xx BC=0c0c DE=0c0c HL=c123 ZNHC=0101

    resetFlags
    ld A, 8
    cp $0c
    ; AF=08xx BC=0c0c DE=0c0c HL=c123 ZNHC=0111

    resetFlags
    ld A, $88
    cp $0c
    ; AF=88xx BC=0c0c DE=0c0c HL=c123 ZNHC=0110
