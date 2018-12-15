INCLUDE "utils.asm"
; with C=0

    resetFlags
    ld A, 0
    sbc A
    ; AF=$00xx ZNHC=1100

    resetFlags
    ld A, 2
    sbc A
    ; AF=$00xx ZNHC=1100

    resetFlags
    ld A, 2
    ld B, 0
    sbc B
    ; AF=02xx BC=00xx ZNHC=0100

    resetFlags
    ld B, 2
    sbc B
    ; AF=00xx BC=02xx ZNHC=1100

    resetFlags
    sbc B
    ; AF=fexx BC=02xx ZNHC=0101

    resetFlags
    ld A, 8
    ld B, $0c
    sbc B
    ; AF=fcxx BC=0cxx ZNHC=0111

    resetFlags
    ld A, $88
    sbc B
    ; AF=7cxx BC=0cxx ZNHC=0110

; sbc A, C
    resetFlags
    ld A, 2
    ld C, 0
    sbc C
    ; AF=02xx BC=0c00 ZNHC=0100

    resetFlags
    ld C, 2
    sbc C
    ; AF=00xx BC=0c02 ZNHC=1100

    resetFlags
    sbc C
    ; AF=fexx BC=0c02 ZNHC=0101

    resetFlags
    ld A, 8
    ld C, $0c
    sbc C
    ; AF=fcxx BC=0c0c ZNHC=0111

    resetFlags
    ld A, $88
    sbc C
    ; AF=7cfxx BC=0c0c ZNHC=0110

; sbc A, D
    resetFlags
    ld A, 2    
    ld D, 0
    sbc D
    ; AF=02xx BC=0c00 DE=00xx ZNHC=0100

    resetFlags
    ld D, 2
    sbc D
    ; AF=00xx BC=0c02 DE=02xx ZNHC=1100

    resetFlags
    sbc D
    ; AF=fexx BC=0c02 DE=02xx ZNHC=0101

    resetFlags
    ld A, 8
    ld D, $0c
    sbc D
    ; AF=fcxx BC=0c0c DE=0cxx ZNHC=0111

    resetFlags
    ld A, $88
    sbc D
    ; AF=7cxx BC=0c0c DE=0cxx ZNHC=0110

; sbc A, E
    resetFlags
    ld A, 2    
    ld E, 0
    sbc E
    ; AF=02xx BC=0c0c DE=0c00 ZNHC=0100

    resetFlags
    ld E, 2
    sbc E
    ; AF=00xx BC=0c0c DE=0c02 ZNHC=1100

    resetFlags
    sbc E
    ; AF=fexx BC=0c0c DE=0c02 ZNHC=0101

    resetFlags
    ld A, 8
    ld E, $0c
    sbc E
    ; AF=fcxx BC=0c0c DE=0c0c ZNHC=0111

    resetFlags
    ld A, $88
    sbc E
    ; AF=7cxx BC=0c0c DE=0c0c ZNHC=0110

; sbc A, H
    resetFlags
    ld A, 2    
    ld H, 0
    sbc H
    ; AF=02xx BC=0c0c DE=0c0c HL=00xx ZNHC=0100

    resetFlags
    ld H, 2
    sbc H
    ; AF=00xx BC=0c0c DE=0c0c HL=02xx ZNHC=1100

    resetFlags
    sbc H
    ; AF=fexx BC=0c0c DE=0c0c HL=02xx ZNHC=0101

    resetFlags
    ld A, 8
    ld H, $0c
    sbc H
    ; AF=fcxx BC=0c0c DE=0c0c HL=0cxx ZNHC=0111

    resetFlags
    ld A, $88
    sbc H
    ; AF=7cxx BC=0c0c DE=0c0c HL=0cxx ZNHC=0110

; sbc A, L
    resetFlags
    ld A, 2    
    ld L, 0
    sbc L
    ; AF=02xx BC=0c0c DE=0c0c HL=0c00 ZNHC=0100

    resetFlags
    ld L, 2
    sbc L
    ; AF=00xx BC=0c0c DE=0c0c HL=0c02 ZNHC=1100

    resetFlags
    sbc L
    ; AF=fexx BC=0c0c DE=0c0c HL=0c02 ZNHC=0101

    resetFlags
    ld A, 8
    ld L, $0c
    sbc L
    ; AF=fcxx BC=0c0c DE=0c0c HL=0c0c ZNHC=0111

    resetFlags
    ld A, $88
    sbc L
    ; AF=7cxx BC=0c0c DE=0c0c HL=0c0c ZNHC=0110

;sbc A, [HL]
    ld H, $c1
    ld L, $23
    resetFlags
    ld A, 2
    ld [HL], 0
    sbc [HL]
    ; AF=02xx BC=0c0c DE=0c0c HL=c123 ZNHC=0100

    resetFlags
    ld [HL], 2
    sbc [HL]
    ; AF=00xx BC=0c0c DE=0c0c HL=c123 ZNHC=1100

    resetFlags
    sbc [HL]
    ; AF=fexx BC=0c0c DE=0c0c HL=c123 ZNHC=0101

    resetFlags
    ld A, 8
    ld [HL], $0c
    sbc [HL]
    ; AF=fcxx BC=0c0c DE=0c0c HL=c123 ZNHC=0111

    resetFlags
    ld A, $88
    sbc [HL]
    ; AF=7cxx BC=0c0c DE=0c0c HL=c123 ZNHC=0110

;sbc A, n
    resetFlags
    ld A, 2
    sbc 0
    ; AF=02xx BC=0c0c DE=0c0c HL=c123 ZNHC=0100

    resetFlags
    sbc 2
    ; AF=00xx BC=0c0c DE=0c0c HL=c123 ZNHC=1100

    resetFlags
    sbc 2
    ; AF=fexx BC=0c0c DE=0c0c HL=c123 ZNHC=0101

    resetFlags
    ld A, 8
    sbc $0c
    ; AF=fcxx BC=0c0c DE=0c0c HL=c123 ZNHC=0111

    resetFlags
    ld A, $88
    sbc $0c
    ; AF=7cxx BC=0c0c DE=0c0c HL=c123 ZNHC=0110

; with C=1
    resetFlags
    ld A, 1
    scf
    sbc A
    ; AF=$00xx ZNHC=1100

    resetFlags
    ld A, 3
    scf
    sbc A
    ; AF=$00xx ZNHC=1100

    resetFlags
    ld A, 3
    ld B, 0
    scf
    sbc B
    ; AF=02xx BC=00xx ZNHC=0100

    add 1
    resetFlags
    ld B, 2
    scf
    sbc B
    ; AF=00xx BC=02xx ZNHC=1100

    add 1
    resetFlags
    scf
    sbc B
    ; AF=fexx BC=02xx ZNHC=0101

    resetFlags
    ld A, 9
    ld B, $0c
    scf
    sbc B
    ; AF=fcxx BC=0cxx ZNHC=0111

    resetFlags
    ld A, $89
    scf
    sbc B
    ; AF=7cxx BC=0cxx ZNHC=0110

; sbc A, C
    resetFlags
    ld A, 3
    ld C, 0
    scf
    sbc C
    ; AF=02xx BC=0c00 ZNHC=0100

    add 1
    resetFlags
    ld C, 2
    scf
    sbc C
    ; AF=00xx BC=0c02 ZNHC=1100

    add 1
    resetFlags
    scf
    sbc C
    ; AF=fexx BC=0c02 ZNHC=0101

    resetFlags
    ld A, 9
    ld C, $0c
    scf
    sbc C
    ; AF=fcxx BC=0c0c ZNHC=0111

    resetFlags
    ld A, $89
    scf
    sbc C
    ; AF=7cfxx BC=0c0c ZNHC=0110

; sbc A, D
    resetFlags
    ld A, 3
    ld D, 0
    scf
    sbc D
    ; AF=02xx BC=0c00 DE=00xx ZNHC=0100

    add 1
    resetFlags
    ld D, 2
    scf
    sbc D
    ; AF=00xx BC=0c02 DE=02xx ZNHC=1100

    add 1
    resetFlags
    sbc D
    ; AF=fexx BC=0c02 DE=02xx ZNHC=0101

    resetFlags
    ld A, 9
    ld D, $0c
    scf
    sbc D
    ; AF=fcxx BC=0c0c DE=0cxx ZNHC=0111

    resetFlags
    ld A, $89
    scf
    sbc D
    ; AF=7cxx BC=0c0c DE=0cxx ZNHC=0110

; sbc A, E
    resetFlags
    ld A, 3
    ld E, 0
    scf
    sbc E
    ; AF=02xx BC=0c0c DE=0c00 ZNHC=0100

    add 1
    resetFlags
    ld E, 2
    scf
    sbc E
    ; AF=00xx BC=0c0c DE=0c02 ZNHC=1100

    add 1
    resetFlags
    scf
    sbc E
    ; AF=fexx BC=0c0c DE=0c02 ZNHC=0101

    resetFlags
    ld A, 9
    ld E, $0c
    scf
    sbc E
    ; AF=fcxx BC=0c0c DE=0c0c ZNHC=0111

    resetFlags
    ld A, $89
    scf
    sbc E
    ; AF=7cxx BC=0c0c DE=0c0c ZNHC=0110

; sbc A, H
    resetFlags
    ld A, 3
    ld H, 0
    scf
    sbc H
    ; AF=02xx BC=0c0c DE=0c0c HL=00xx ZNHC=0100

    add 1
    resetFlags
    ld H, 2
    scf
    sbc H
    ; AF=00xx BC=0c0c DE=0c0c HL=02xx ZNHC=1100

    add 1
    resetFlags
    scf
    sbc H
    ; AF=fexx BC=0c0c DE=0c0c HL=02xx ZNHC=0101

    resetFlags
    ld A, 9
    ld H, $0c
    scf
    sbc H
    ; AF=fcxx BC=0c0c DE=0c0c HL=0cxx ZNHC=0111

    resetFlags
    ld A, $89
    scf
    sbc H
    ; AF=7cxx BC=0c0c DE=0c0c HL=0cxx ZNHC=0110

; sbc A, L
    resetFlags
    ld A, 3
    ld L, 0
    scf
    sbc L
    ; AF=02xx BC=0c0c DE=0c0c HL=0c00 ZNHC=0100

    add 1
    resetFlags
    ld L, 2
    scf
    sbc L
    ; AF=00xx BC=0c0c DE=0c0c HL=0c02 ZNHC=1100

    add 1
    resetFlags
    scf
    sbc L
    ; AF=fexx BC=0c0c DE=0c0c HL=0c02 ZNHC=0101

    resetFlags
    ld A, 9
    ld L, $0c
    scf
    sbc L
    ; AF=fcxx BC=0c0c DE=0c0c HL=0c0c ZNHC=0111

    resetFlags
    ld A, $89
    scf
    sbc L
    ; AF=7cxx BC=0c0c DE=0c0c HL=0c0c ZNHC=0110

;sbc A, [HL]
    ld H, $c1
    ld L, $23
    resetFlags
    ld A, 3
    ld [HL], 0
    scf
    sbc [HL]
    ; AF=02xx BC=0c0c DE=0c0c HL=c123 ZNHC=0100

    add 1
    resetFlags
    ld [HL], 2
    scf
    sbc [HL]
    ; AF=00xx BC=0c0c DE=0c0c HL=c123 ZNHC=1100

    add 1
    resetFlags
    scf
    sbc [HL]
    ; AF=fexx BC=0c0c DE=0c0c HL=c123 ZNHC=0101

    resetFlags
    ld A, 9
    ld [HL], $0c
    scf
    sbc [HL]
    ; AF=fcxx BC=0c0c DE=0c0c HL=c123 ZNHC=0111

    resetFlags
    ld A, $89
    scf
    sbc [HL]
    ; AF=7cxx BC=0c0c DE=0c0c HL=c123 ZNHC=0110

;sbc A, n
    resetFlags
    ld A, 3
    scf
    sbc 0
    ; AF=02xx BC=0c0c DE=0c0c HL=c123 ZNHC=0100

    add 1
    resetFlags
    scf
    sbc 2
    ; AF=00xx BC=0c0c DE=0c0c HL=c123 ZNHC=1100

    add 1
    resetFlags
    scf
    sbc 2
    ; AF=fexx BC=0c0c DE=0c0c HL=c123 ZNHC=0101

    resetFlags
    ld A, 9
    scf
    sbc $0c
    ; AF=fcxx BC=0c0c DE=0c0c HL=c123 ZNHC=0111

    resetFlags
    ld A, $89
    scf
    sbc $0c
    ; AF=7cxx BC=0c0c DE=0c0c HL=c123 ZNHC=0110
