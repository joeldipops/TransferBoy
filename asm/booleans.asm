 ;  AND

    ld A, %10101010 ; 170/ $AA
    ld B, 1
    ld C, 2

    resetFlags
    and A
    ; AF=aaxx BC=0102 DE=0304 HL=0506 ZNHC=0010

    resetFlags
    ld A, 0
    and A
    ; AF=00xx BC=0102 DE=0304 HL=0506 ZNHC=1010

    resetFlags
    ld A, $aa
    and B
    ; AF=00xx BC=0102 DE=0304 HL=0506 ZNHC=1010

    resetFlags        
    ld A, $ab
    and B
    ; AF=01xx BC=0102 DE=0304 HL=0506 ZNHC=0010

    resetFlags
    ld A, $aa
    and C
    ; AF=02xx BC=0102 DE=0304 HL=0506 ZNHC=0010

    resetFlags
    ld A, $4
    and C
    ; AF=00xx BC=0102 DE=0304 HL=0506 ZNHC=1010

    resetFlags
    ld A, $0f
    ld D, $13
    and D
    ; AF=03xx BC=0102 DE=1304 HL=0506 ZNHC=0010
    
    resetFlags
    ld A, $0F
    ld D, $20
    and D
    ; AF=00xx BC=0102 DE=2004 HL=0506 ZNHC=1010

    resetFlags
    ld A, $0f
    ld E, $13
    and E
    ; AF=03xx BC=0102 DE=2013 HL=0506 ZNHC=0010

    resetFlags    
    ld A, $0f
    ld E, $20
    and E
    ; AF=00xx BC=0102 DE=2020 HL=0506 ZNHC=1010

    resetFlags
    ld A, $0f
    ld H, $13
    and H
    ; AF=03xx BC=0102 DE=2020 HL=1306 ZNHC=0010
    
    resetFlags 
    ld A, $0f
    ld H, $20
    and H
    ; AF=00xx BC=0102 DE=2020 HL=2006 ZNHC=1010
   
    resetFlags
    ld A, $0f
    ld L, $13
    and L
    ; AF=03xx BC=0102 DE=2020 HL=2013 ZNHC=0010

    resetFlags
    ld A, $0f
    ld L, $20
    and L
    ; AF=00xx BC=0102 DE=2020 HL=2020 ZNHC=1010

    resetFlags
    ld H, $ca
    ld L, $be
    ld A, $0f
    ld [HL], $13
    and [HL]
    ; AF=03xx BC=0102 DE=2020 HL=cabe ZNHC=0010    

    resetFlags
    ld A, $0f
    ld [HL], $20
    and [HL]
    ; AF=00xx BC=0102 DE=2020 HL=cabe ZNHC=1010
    
    resetFlags
    ld A, $0f
    and A, $13
    ; AF=03xx BC=0102 DE=2020 HL=cabe ZNHC=0010
    
    resetFlags
    ld A, $0f
    and A, $20
    ; AF=00xx BC=0102 DE=2020 HL=cabe ZNHC=1010     

;   OR
    ld A, %10100101 ; $a5
    or A
    ; AF=a500 BC=0102 DE=2020 HL=cabe ZNHC=0000

    ld A, 0
    or A
    ; AF=0001 BC=0102 DE=2020 HL=cabe ZNHC=1000

; or B
    ld A, %10101010
    ld B, %01010101
    or B
    ; AF=ff00 BC=5502 DE=2020 HL=cabe ZNHC=0000

    ld B, $0f
    sub A
    ; ^ set some flags so we know they get reset
    ld A, 0
    ld B, 0
    or B
    ; AF=0001 BC=0002 DE=2020 HL=cabe ZNHC=1000

    ld A, 1
    or B
    ; AF=0100 BC=0002 DE=2020 HL=cabe ZNHC=0000

; or C
    ld A, %10101010
    ld C, %01010101
    or C
    ; AF=ff00 BC=0055 DE=2020 HL=cabe ZNHC=0000

    ld B, $0f
    sub B
    ; ^ set some flags so we know they get reset
    ld A, 0
    ld C, 0
    or C
    ; AF=0001 BC=0f00 DE=2020 HL=cabe ZNHC=1000

    ld A, 0
    ld C, 1
    or C
    ; AF=0100 BC=0f01 DE=2020 HL=cabe ZNHC=0000

; or D
    ld A, %10101010
    ld D, %01010101
    or D
    ; AF=ff00 BC=0f01 DE=5520 HL=cabe ZNHC=0000

    sub B
    ; ^ set some flags so we know they get reset
    ld A, 0
    ld D, 0
    or D
    ; AF=0001 BC=ff01 DE=0020 HL=cabe ZNHC=1000

    ld D, 1
    ld A, 0
    or D
    ; AF=0100 BC=0f01 DE=0120 HL=cabe ZNHC=0000

; or E
    ld A, %10101010
    ld E, %01010101
    or E
    ; AF=ff00 BC=0f01 DE=0155 HL=cabe ZNHC=0000

    sub B
    ; ^ set some flags so we know they get reset
    ld A, 0
    ld E, 0
    or E
    ; AF=0001 BC=0f01 DE=0100 HL=cabe ZNHC=1000

    ld A, 1
    or E
    ; AF=0100 BC=0f01 DE=0100 HL=cabe ZNHC=0000

; or H
    ld A, %10101010
    ld H, %01010101
    or H
    ; AF=ff00 BC=0f01 DE=0100 HL=55be ZNHC=0000

    ld B, $0f
    sub B
    ; ^ set some flags so we know they get reset
    ld A, 0
    ld H, 0
    or H
    ; AF=0001 BC=0f01 DE=0100 HL=00be ZNHC=1000

    ld A, 0
    ld H, $23
    or H
    ; AF=2300 BC=0f01 DE=0100 HL=23be ZNHC=0000

; or L
    ld A, %10101010
    ld L, %01010101
    or L
    ; AF=ff00 BC=0f01 DE=0100 HL=2355 ZNHC=0000

    ld B, $0f
    sub B
    ; ^ set some flags so we know they get reset
    ld A, 0
    ld L, 0
    or L
    ; AF=0001 BC=0f01 DE=0100 HL=2355 ZNHC=1000

    ld A, 0
    ld L, $da
    or L
    ; AF=da00 BC=0f01 DE=0100 HL=23da ZNHC=0000

    ld H, $cb
    ld L, $de

    ld A, %10101010
    ld [HL], %01010101
    or [HL]
    ; AF=ff00 BC=0f01 DE=0100 HL=cbde ZNHC=0000

    ld B, $0f
    sub B
    ; ^ set some flags so we know they get reset
    ld A, 0
    ld [HL], 0
    or [HL]
    ; AF=0001 BC=0f01 DE=0100 HL=cbde ZNHC=1000

    ld A, 0
    ld [HL], $f3
    or [HL]
    ; AF=f300 BC=0f01 DE=0100 HL=cbde ZNHC=0000

    ld A, 1
    or [HL]
    ; AF=f300 BC=0f01 DE=0100 HL=cbde ZNHC=0000

    ld [HL], 4
    or [HL]
    ; AF=f700 BC=0f01 DE=0100 HL=cbde ZNHC=0000

    ld A, 1
    sub B
    ; ^ some flags set
    ld A,   %00101110
    or      %01101101
    ; AF=6400 BC=0f01 DE=0100 HL=cbde ZNHC=0000

    ld A, 0
    or 0
    ; AF=0001 BC=0f01 DE=0100 HL=cbde ZNHC=0000

; XORs

    ld A, 1
    sub B
    ; ^ set some flags
    ld A, 0
    xor A
    ; AF=0001 BC=0f01 DE=0100 HL=cbde ZNHC=1000

    ld A, %11101001
    xor A
    ; AF=0001 BC=0f01 DE=0100 HL=cbde ZNHC=1000

    ld A, $ff
    xor A
    ; AF=0001 BC=0f01 DE=0100 HL=cbde ZNHC=1000

; xor n
    sub B
    ld A, 0
    xor $9e
    ; AF=9e00 BC=0f01 DE=0100 HL=cbde ZNHC=0000

    ld A, $fe
    xor $9e ; %10011110
    ; AF=6000 BC=0f01 DE=0100 HL=cbde ZNHC=0000

    ld A, $fe
    xor $fe
    ; AF=0001 BC=0f01 DE=0100 HL=cbde ZNHC=1000

; xor B
    sub B
    ld A, 0
    ld B, $9e
    xor B
    ; AF=9e00 BC=9e01 DE=0100 HL=cbde ZNHC=0000

    ld A, $fe
    xor B ; %10011110
    ; AF=6000 BC=0f01 DE=0100 HL=cbde ZNHC=0000

    ld A, $fe
    ld B, $fe
    xor B
    ; AF=0001 BC=fe01 DE=0100 HL=cbde ZNHC=1000

; xor C
    sub B
    ld A, 0
    ld C, $9e
    xor C
    ; AF=9e00 BC=fe9e DE=0100 HL=cbde ZNHC=0000

    ld A, $fe
    xor C ; %10011110
    ; AF=6000 BC=fe9e DE=0100 HL=cbde ZNHC=0000

    ld A, $fe
    ld C, $fe
    xor C
    ; AF=0001 BC=fefe DE=0100 HL=cbde ZNHC=1000    

; xor D
    sub B
    ld A, 0
    ld D, $9e
    xor D
    ; AF=9e00 BC=fefe DE=9e00 HL=cbde ZNHC=0000

    ld A, $fe
    xor D ; %10011110
    ; AF=6000 BC=fefe DE=9e00 HL=cbde ZNHC=0000

    ld A, $fe
    ld D, $fe
    xor D
    ; AF=0001 BC=fefe DE=fe00 HL=cbde ZNHC=1000    

; xor E
    sub B
    ld A, 0
    ld E, $9e
    xor E
    ; AF=9e00 BC=fefe DE=fe9e HL=cbde ZNHC=0000

    ld A, $fe
    xor E ; %10011110
    ; AF=6000 BC=fefe DE=fe9e HL=cbde ZNHC=0000

    ld A, $fe
    ld E, $fe
    xor E
    ; AF=0001 BC=fefe DE=fefe HL=cbde ZNHC=1000

; xor H
    sub B
    ld A, 0
    ld H, $9e
    xor H
    ; AF=9e00 BC=fefe DE=fe9e HL=9ede ZNHC=0000

    ld A, $fe
    xor H ; %10011110
    ; AF=6000 BC=fefe DE=fe9e HL=9ede ZNHC=0000

    ld A, $fe
    ld H, $fe
    xor H
    ; AF=0001 BC=fefe DE=fefe HL=fede ZNHC=1000

; xor L
    sub B
    ld A, 0
    ld L, $9e
    xor L
    ; AF=9e00 BC=fefe DE=fe9e HL=9e9e ZNHC=0000

    ld A, $fe
    xor L ; %10011110
    ; AF=6000 BC=fefe DE=fe9e HL=9e9e ZNHC=0000

    ld A, $fe
    ld L, $fe
    xor L
    ; AF=0001 BC=fefe DE=fefe HL=fefe ZNHC=1000  

; xor [HL]
    ld H, $ce
    ld L, $eb

    sub B
    ld A, 0
    ld [HL], $9e
    xor [HL]
    ; AF=9e00 BC=fefe DE=fe9e HL=ceeb ZNHC=0000

    ld A, $fe
    xor [HL] ; %10011110
    ; AF=6000 BC=fefe DE=fe9e HL=ceeb ZNHC=0000

    ld A, $fe
    ld [HL], $fe
    xor [HL]
    ; AF=0001 BC=fefe DE=fefe HL=ceeb ZNHC=1000   