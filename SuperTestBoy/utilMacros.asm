    IF (!DEF(UTILS_INCLUDED))
UTILS_INCLUDED SET 1

;;;
; Push all registers on to the stack so we can interrupt safely.
;;;
pushAll: macro
    push AF
    push BC
    push DE
    push HL
endm

;;;
; Pop all registers when we're done with an interrupt.
;;;
popAll: macro
    pop HL
    pop DE
    pop BC
    pop AF
endm

;;; 
; Lets us put a break point on a nop
;;;
debugger: macro
    or A
    jr NZ .skip
    nop
.skip 
endm

    ENDC