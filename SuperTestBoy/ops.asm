    IF !DEF(PSEUDO_OPS_INCLUDED)
PSEUDO_OPS_INCLUDED SET 1

;;;
; Adds two values, Result in r8
; addAny r8, [r16]
;;;;
addAny: macro
    ld A, \1
    add \2
    ld \1, A 
endm

;;;
; Adds two values + 1 if carry flag set. Result in r8.
; adcAny r8, [r16]
;;;
adcAny: macro
    ld A, \1
    adc \2
    ld \1, A 
endm

;;;
; Inserts a null terminated string into ROM
; dbs string
; Bytes: Length of String + 1
; Cycles: N/A
; Flags: N/A
;;;
dbs: macro
    db \1,0
endm

;;;
; Pushes an immediate value on to the stack.
; pushAny r16
; Affects HL
;;;
pushAny: macro
    ld HL, \1
    push HL
endm

;;;
; mult r8, r8
; Multiples two numbers, result in HL
;;;
mult: macro
    push BC
    ld HL, 0
    ld B, \1
    ld C, \2

    ; If either of the operands are 0, return 0
    or B
        jr Z, .end
    or C
        jr Z, .end

    ld A, 0
.loop
        ; TODO can we use `add HL, r16`??
        add A, C
        ld L, A
        adcAny H, 0
        dec B
        ld A, L
    jr NZ, .loop
.end
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
; ld16RR r16high,r16low, r16high,r16low
; Cycles: 2
; Bytes: 2
; Flags: None
;
; @example - ld16RR H,L, B,C
; @notes - rgbasm has the conditional assembly directive IF - I thought I would be able to be like 
; IF (\2 == BC) 
;    \3 = B 
;    \4 = C 
; ENDC
; so that this could be called as ld16 HL, BC.
; But no, it seems IFs can only be used with integer constants, so I'm stuck with this syntax until I feel like forking rednex 
;;;
ld16RR: macro
    ld \1, \3
    ld \2, \4
endm

;;;
; Loads a 16bit block of memory in to r16
; ld16RA r16high,r16low, n16
;;;
ld16RA: macro
    ld A, [\3]
    ld \1, A
    ld A, [\3 + 1]
    ld \2, A
endm

;;;
; Loads a 16bit regisuter in to memory
;;;
ld16AR: macro
    ld A, \2
    ld [\1], A
    ld A, \3
    ld [\3 + 1], A
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



;;;
; Adds two 16bit registers together. Result in the first.
; add16 r16high,r16low, r16high,r16low
;;;
add16: macro
    ld A, \2
    add \4
    ld \2, A
    ld A, \1
    adc \3
    ld \1, A
endm
    
    ENDC
