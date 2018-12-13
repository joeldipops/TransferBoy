    ld A, %10101010 ; 170/ $AA
    ld B, 1
    ld C, 2

    call resetFlags
    and A
    ; AF=aaxx BC=0102 DE=0304 HL=0506 ZNHC=0010

    call resetFlags
    ld A, 0
    and A
    ; AF=00xx BC=0102 DE=0304 HL=0506 ZNHC=1010

    call resetFlags
    ld A, $aa
    and B
    ; AF=00xx BC=0102 DE=0304 HL=0506 ZNHC=1010

    call resetFlags        
    ld A, $ab
    and B
    ; AF=01xx BC=0102 DE=0304 HL=0506 ZNHC=0010

    call resetFlags
    ld A, $aa
    and C
    ; AF=02xx BC=0102 DE=0304 HL=0506 ZNHC=0010

    call resetFlags
    ld A, $4
    and C
    ; AF=00xx BC=0102 DE=0304 HL=0506 ZNHC=1010

    call resetFlags
    ld A, $0f
    ld D, $13
    and D
    ; AF=03xx BC=0102 DE=1304 HL=0506 ZNHC=0010
    
    call resetFlags
    ld A, $0F
    ld D, $20
    and D
    ; AF=00xx BC=0102 DE=2004 HL=0506 ZNHC=1010

    call resetFlags
    ld A, $0f
    ld E, $13
    and E
    ; AF=03xx BC=0102 DE=2013 HL=0506 ZNHC=0010

    call resetFlags    
    ld A, $0f
    ld E, $20
    and E
    ; AF=00xx BC=0102 DE=2020 HL=0506 ZNHC=1010

    call resetFlags
    ld A, $0f
    ld H, $13
    and H
    ; AF=03xx BC=0102 DE=2020 HL=1306 ZNHC=0010
    
    call resetFlags 
    ld A, $0f
    ld H, $20
    and H
    ; AF=00xx BC=0102 DE=2020 HL=2006 ZNHC=1010
   
    call resetFlags
    ld A, $0f
    ld L, $13
    and L
    ; AF=03xx BC=0102 DE=2020 HL=2013 ZNHC=0010

    call resetFlags
    ld A, $0f
    ld L, $20
    and L
    ; AF=00xx BC=0102 DE=2020 HL=2020 ZNHC=1010

    call resetFlags
    ld H, $ca
    ld L, $be
    ld A, $0f
    ld [HL], $13
    and [HL]
    ; AF=03xx BC=0102 DE=2020 HL=cabe ZNHC=0010    

    call resetFlags
    ld A, $0f
    ld [HL], $20
    and [HL]
    ; AF=00xx BC=0102 DE=2020 HL=cabe ZNHC=1010
    
    call resetFlags
    ld A, $0f
    and A, $13
    ; AF=03xx BC=0102 DE=2020 HL=cabe ZNHC=0010
    
    call resetFlags
    ld A, $0f
    and A, $20
    ; AF=00xx BC=0102 DE=2020 HL=cabe ZNHC=1010     
