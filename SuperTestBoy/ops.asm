    IF !DEF(PSEUDO_OPS_INCLUDED)
PSEUDO_OPS_INCLUDED SET 1

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

cpIO: macro
    ldh A, \1
    cp \2
endm

cpAny: macro
    ld A, \1
    cp \2
endm
    ENDC