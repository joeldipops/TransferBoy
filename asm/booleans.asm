    ld A, %10101010 ; 170/ $AA
    ld B, 1
    ld C, 2
    ld D, 3
    ld E, 4
    ld H, 5
    ld L, 6

    and A
    ; AF=aaxx BC=0102 DE=0304 HL=0506 ZNHC=0010

    ld A, 0
    and A
    ; AF=00xx BC=0102 DE=0304 HL=0506 ZNHC=1010

    ld A, $aa
    and B
    ; AF=00xx BC=0102 DE=0304 HL=0506 ZNHC=1010

    ld A, $ab
    and B
    ; AF=01xx BC=0102 DE=0304 HL=0506 ZNHC=0010

    ld A, $aa
    and C
    ; AF=02xx BC=0102 DE=0304 HL=0506 ZNHC=0010

    ld A, $4
    and C
    ; AF=00xx BC=0102 DE=0304 HL=0506 ZNHC=1010

    ld A, $0F
    ld D, $13
    and D
    ; AF=03xx BC=0102 DE=1304 HL=0506 ZNHC=1010