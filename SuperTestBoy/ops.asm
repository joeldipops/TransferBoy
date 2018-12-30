    IF !DEF(PSEUDO_OPS_INCLUDED)
PSEUDO_OPS_INCLUDED SET 1

;;;
; Multiplies two numbers
; @param \1 the first number
; @param \2 the second number
; @return A the multiplied result.
; Don't use this as a macro if you're running out of ROM space
;;;
mult: macro
    push BC
    push DE
    ld B, \1
    ld C, \2
    ld A, 0
.loop                         ; do
        add A, C                ;     result += A
        dec B                   ;     b--
        ld D, A
        ld A, B
        cp 0                    ; until (b == 0)
        ld B, A
        ld A, D   
        jr NZ, .loop              
    pop DE
    pop BC
endm


;;;
; Loads byte from anywhere to anywhere else that takes a byte.
; ldAny r8, [n16]
; Cycles: 5 
; Bytes: 4 
; Flags: None
;
; ldAny r8, [r16]
; Cycles: 3
; Bytes: 2 
; Flags: None
;
; ldAny [n16], r8
; Cycles: 5
; Bytes: 4
; Flags: None
;
; ldAny [r16], r8
; Cycles: 3
; Bytes: 2
; Flags: None
;
; ldAny [r16], [r16]
; Cycles: 4
; Bytes: 2
; Flags: None
;
; ldAny [r16], [n16]
; Cycles: 6
; Bytes: 4
; Flags: None
;
; ldAny [n16], [r16]
; Cycles: 6
; Bytes: 4
; Flags: None
;
; ldAny [n16], [n16]
; Cycles: 8
; Bytes: 6
; Flags: None
;
; ldAny [n16], n8
; Cycles: 
; Bytes:
; Flags: None
;
;;;
ldAny: macro
    ld A, \2
    ld \1, A
endm

;;;
; Loads to an address in IO space
;
; ldhAny [$ff00 + n8], n8
; Cycles: 5
; Bytes: 4 
; Flags: None 
;
; ldhAny [$ff00 + n8], r8
; Cycles: 4
; Bytes: 3 
; Flags: None
;
; ldhAny [$ff00 + n8], [r16]
; Cycles: 5
; Bytes: 3 
; Flags: None 
;
; ldhAny [$ff00 + n8], [n16]
; Cycles: 7
; Bytes: 5 
; Flags: None 
;;;
ldhAny: macro
    ld A, \2
    ldh \1, A
endm

;;;
; Loads from [HL] then increments HL
; ldiAny r8, [HL]
; Cycles: 3
; Bytes: 2
; Flags: None
;
; ldiAny [r16], [HL]
; Cycles: 4
; Bytes: 2
; Flags: None
;
; ldiAny [n16], [HL]
; Cycles: 6
; Bytes: 4
; Flags: None
;;;
ldiAny: macro
    ldi A, [HL]
    ld \1, A
endm

;;;
; ldHLi [HL], n8
; Cycles: 6
; Bytes: 4
; Flags: None
;;;
ldHLi: macro
    ld A, \2
    ldi [HL], A
endm

;;;
; ORs the bits of two registers, result in A
;
; orAny r8, r8
; Cycles: 2
; Bytes: 2
; Flags: Z=? N=0 H=0 C=0
; 
; orAny r8, n8
; Cycles: 4
; Bytes: 4 
; Flags: Z=? N=0 H=0 C=0
;
; orAny r8, [HL]
; Cycles: 3
; Bytes: 2
; Flags: Z=? N=0 H=0 C=0
;
; orAny [r16], r8
; Cycles: 3
; Bytes: 2
; Flags: Z=? N=0 H=0 C=0
; 
; orAny [r16], n8
; Cycles: 4
; Bytes: 3 
; Flags: Z=? N=0 H=0 C=0
;
; orAny [r16], [HL]
; Cycles: 4
; Bytes: 2
; Flags: Z=? N=0 H=0 C=0
;
; orAny [n16], r8
; Cycles: 5
; Bytes: 4
; Flags: Z=? N=0 H=0 C=0
; 
; orAny [n16], n8
; Cycles: 6
; Bytes: 5 
; Flags: Z=? N=0 H=0 C=0
;
; orAny [n16], [HL]
; Cycles: 6
; Bytes: 4
; Flags: Z=? N=0 H=0 C=0
;;;
orAny: macro
    ld A, \1
    or \2
endm

;;;
; ANDs the bit of two registers, result in A
;
; andAny r8, r8
; Cycles: 2
; Bytes: 2
; Flags: Z=? N=0 H=1 C=0
; 
; andAny r8, n8
; Cycles: 4
; Bytes: 4 
; Flags: Z=? N=0 H=1 C=0
;
; andAny r8, [HL]
; Cycles: 3
; Bytes: 2
; Flags: Z=? N=0 H=1 C=0
;
; andAny [r16], r8
; Cycles: 3
; Bytes: 2
; Flags: Z=? N=0 H=1 C=0
; 
; andAny [r16], n8
; Cycles: 4
; Bytes: 3 
; Flags: Z=? N=0 H=1 C=0
;
; andAny [r16], [HL]
; Cycles: 4
; Bytes: 2
; Flags: Z=? N=0 H=1 C=0
;
; andAny [n16], r8
; Cycles: 5
; Bytes: 4
; Flags: Z=? N=0 H=1 C=0
; 
; andAny [n16], n8
; Cycles: 6
; Bytes: 5 
; Flags: Z=? N=0 H=1 C=0
;
; andAny [n16], [HL]
; Cycles: 6
; Bytes: 4
; Flags: Z=? N=0 H=1 C=0
;;;
andAny: macro
    ld A, \1
    and \2
endm

;;;
; Performs an XOR on any two 8bit registers. Usual flags affected and A set to the result.
;
; xorAny r8, r8
; Cycles: 2
; Bytes: 2
; Flags: Z=? N=0 H=0 C=0
; 
; xorAny r8, n8
; Cycles: 4
; Bytes: 4 
; Flags: Z=? N=0 H=0 C=0
;
; xorAny r8, [HL]
; Cycles: 3
; Bytes: 2
; Flags: Z=? N=0 H=0 C=0
;
; xorAny [r16], r8
; Cycles: 3
; Bytes: 2
; Flags: Z=? N=0 H=0 C=0
; 
; xorAny [r16], n8
; Cycles: 4
; Bytes: 3 
; Flags: Z=? N=0 H=0 C=0
;
; xorAny [r16], [HL]
; Cycles: 4
; Bytes: 2
; Flags: Z=? N=0 H=0 C=0
;
; xorAny [n16], r8
; Cycles: 5
; Bytes: 4
; Flags: Z=? N=0 H=0 C=0
; 
; xorAny [n16], n8
; Cycles: 6
; Bytes: 5 
; Flags: Z=? N=0 H=0 C=0
;
; xorAny [n16], [HL]
; Cycles: 6
; Bytes: 4
; Flags: Z=? N=0 H=0 C=0
;;;
xorAny: macro
    ld A, \1
    xor \2
endm

;;;
; Resets a bit of an 8bit piece of memory
;
; resAny u3, [n16]
; Cycles: 10 
; Bytes: 8
; Flags: None
;
; resAny u3, [r16]
; Cycles: 6
; Bytes: 4
; Flags: None
;;;
resAny: macro
    ld A, \2
    res \1, A
    ld \2, A
endm

;;;
; Resets a bit of an 8bit piece of IO space memory
;
; resH u3, [$ff00 + n8]
; Cycles: 8 
; Bytes: 6
; Flags: None
;;;
resH: macro
    ldh A, \2
    res \1, A
    ldh \2, A
endm

;;;
; Compares a value with a value in the IO space of memory and sets flags.
;
; cpH [$ff00 + n8], n8
; Cycles: 5
; Bytes: 4
; Flags: Z=? N=1, H=? C=?
;
; cpH [$ff00 + n8], r8 
; Cycles: 4
; Bytes: 3
; Flags: Z=? N=1, H=? C=?
;
; cpH [$ff00 + n8], [HL] 
; Cycles: 5
; Bytes: 3
; Flags: Z=? N=1, H=? C=?
;;;
cpH: macro
    ldh A, \1
    cp \2
endm

;;;
; Compares two values, setting appropriate flags.
;
; cpAny r8, n8
; Cycles: 4
; Bytes: 4
; Flags: Z=? N=1, H=? C=?
;
; cpAny r8, r8
; Cycles: 3
; Bytes: 3
; Flags: Z=? N=1, H=? C=?
;
; cpAny r8, [HL]
; Cycles: 4
; Bytes: 3
; Flags: Z=? N=1, H=? C=?
;
; cpAny [r16], n8
; Cycles: 4
; Bytes: 3
; Flags: Z=? N=1, H=? C=?
;
; cpAny [r16], r8
; Cycles: 3
; Bytes: 2
; Flags: Z=? N=1, H=? C=?
;
; cpAny [r16], [HL]
; Cycles: 4
; Bytes: 2
; Flags: Z=? N=1, H=? C=?
;
; cpAny [n16], n8
; Cycles: 6
; Bytes: 5
; Flags: Z=? N=1, H=? C=?
;
; cpAny [n16], r8
; Cycles: 5
; Bytes: 4
; Flags: Z=? N=1, H=? C=?
;
; cpAny [n16], [HL]
; Cycles: 6
; Bytes: 4
; Flags: Z=? N=1, H=? C=?
;;;
cpAny: macro
    ld A, \1
    cp \2
endm

;;;
; Increment an 8bit value
;
; incAny [n16]
; Cycles: 9
; Bytes: 7
; Flags: Z=? N=0 H=? C=C
;
; incAny [r16]
; Cycles: 5
; Bytes: 3
; Flags: Z=? N=0 H=? C=C
;;;
incAny: macro
    ld A, \1
    inc A
    ld \1, A
endm

;;;
; Decrement an 8bit value
;
; decAny [n16]
; Cycles: 9
; Bytes: 7
; Flags: Z=? N=1 H=? C=C
;
; decAny [r16]
; Cycles: 5
; Bytes: 3
; Flags: Z=? N=1 H=? C=C
;;;
decAny: macro
    ld A, \1
    dec A
    ld \1, A
endm

;;;
; Loads one 16 bit register into another
;
; ld16 r16high,r16low, r16high,r16low
; Cycles: 2
; Bytes: 2
; Flags: None
;
; @example - ld16 H,L, B,C
; @notes - rgbasm has the conditional assembly directive IF - I thought I would be able to be like 
; IF (\2 == BC) 
;    \3 = B 
;    \4 = C 
; ENDC
; so that this could be called as ld16 HL, BC.
; But no, it seems IFs can only be used with integer constants, so I'm stuck with this syntax until I feel like forking rednex 
;;;
ld16: macro
    ld \1, \3
    ld \2, \4
endm

;;;
; Subtracts one 16 bit register from another with result in \1,\2.
; 
; sub16 r16high,r16low, r16high,r16low
; Cycles: 6
; Bytes: 6
; Flags: Z=? N=1 H=? C=?
;
; @example - sub16 H,L, B,C
; @notes - rgbasm has the conditional assembly directive IF - I thought I would be able to be like 
; IF (\2 == BC) 
;    \3 = B 
;    \4 = C 
; ENDC
; so that this could be called as sub16 HL, BC.
; But no, it seems IFs can only be used with integer constants, so I'm stuck with this syntax until I feel like forking rednex.
;;;
sub16: macro
    ld A, \2
    sub \4
    ld \2, A
    ld A, \1
    sbc \3
    ld \1, A 
endm
    
    ENDC