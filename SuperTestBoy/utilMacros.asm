    IF (!DEF(UTILS_INCLUDED))
UTILS_INCLUDED SET 1

;;; 
; Places the cursor according to the value at [HL]
; @param \1 Top margin
; @reg [HL] Logical y-position of the cursor
;;;
moveCursor: macro
    ld A, [HL]
    mult A, SPRITE_WIDTH
    ld A, L
    add \1
    ld [PcY], A
endm

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

;;;
; Jumps back to the previous menu level.
;;;
backToPrevMenu: macro
    call resetBackground
    call resetForeground
    
    ; Set position to 0 and dec depth.
    ld16RA H,L, cursorPosition
    xor A
    ld [HL], A
    dec HL
    decAny [cursorPosition + 1]

    ldAny [stateInitialised], 0 
endm

    ENDC