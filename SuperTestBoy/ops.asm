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
; Loads a byte from anywhere into anything that holds a byte
; @usage ldAny
;;;
ldAny: macro
    ld A, \2
    ld \1, A
endm

;;;
; Loads a byte from anywhere into a memory address greater than $ff00
; @usage ldhAny [Channel2Envelope], [nextEnvelope]
;;;
ldhAny: macro
    ld A, \2
    ldh \1, A
endm

;;;
; Loads a byte from [HL] into anything that holds a byte and then increments HL
; @usage ldiAny [$c123], [HL]
;;;
ldiAny: macro
    ldi A, [HL]
    ld \1, A
endm

;;;
; Performs an OR on any two 8bit registers. Usual flags affected and A set to the result.
; @usage orReg B, C
;;;
orReg: macro
    ld A, \1
    or \2
endm

;;;
; Performs an AND on any two 8bit registers. Usual flags affected and A set to the result.
; @usage andReg C, [HL], B
; @param \3 Nominate a register to be overwritten
;;;
andReg: macro
    ld A, \1
    and \2
endm

;;;
; Performs an XOR on any two 8bit registers. Usual flags affected and A set to the result.
; @usage xorReg D, $32 
;;;
xorReg: macro
    ld A, \1
    xor \2
endm

;;;
; Performs an OR on any two bytes. Usual flags affected and A set to the result.
; @usage orReg [DE], [$c337], B
; @param \3 Nominate register to be overwritten
;;;
orAny: macro
    ld A, \1
    ld \3, \2
    or \3
endm

;;;
; Performs an AND on any two bytes. Usual flags affected and A set to the result.
; @usage andAny C, [HL], B
; @param \3 Nominate register to be overwritten
;;;
andAny: macro
    ld A, \1
    ld \3, \2
    and \3
endm

;;;
; Performs a XOR on any two bytes. Usual flags affected and A set to the result.
; @usage xorAny D, $32
; @param \3 Nominate register to be overwritten
;;;
xorAny: macro
    ld A, \1
    ld \3, \2
    xor \3
endm

;;;
; Resets a bit of an 8bit piece of memory
; @usage resAny <0-7>, [memory location]
;;;
resAny: macro
    ld A, \2
    res \1, A
    ld \2, A
endm

;;;
; Resets a bit in the IO space of memory ($ff00 - $ffff)
; @usage resIO <0-7>, [memory location] 
;;
resIO: macro
    ldh A, \2
    res \1, A
    ldh \2, A
endm

;;;
; Compares a value with a value in the IO space of memory.
; @param \1 The value in IO memory.
; @param \2 The other value
; @usage cpIO $ff88, B 
;;;
cpIO: macro
    ldh A, \1
    cp \2
endm

;;;
; Compares two values, setting appropriate flags.
; @usage cpAny B, [$c312] 
;;;
cpAny: macro
    ld A, \1
    cp \2
endm

;;;
; Increment any 8bit value
; @usage incAny [SomeAddress]
;;;
incAny: macro
    ld A, \1
    inc A
    ld \1, A
endm

;;;
; Decrement any 8bit value
; @usage decAny [SomeAddress]
;;;
decAny: macro
    ld A, \1
    dec A
    ld \1, A
endm

;;;
; Loads one 16 bit register into another
; ld16 H,L, B,C
;;;
ld16: macro
    ld \1, \3
    ld \2, \4
endm

;;;
; Subtracts one 16 bit register from another.
; @usage sub16 H,L, B,C calculates HL-BC.  Result in HL 
; @notes
; Ideally I'd like to do this so we can pass it sub16 HL, BC
; registers and it could use IF/ENDC blocks to figure out whether
; to use B & C or H & L etc.  Don't see any reason why the assembler can't do this
; For now we have to pass all four registers :(
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