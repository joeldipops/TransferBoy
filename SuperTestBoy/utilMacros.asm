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

;;;
; Resets background to all light tiles.
;;;
resetBackground: macro
    ld A, LIGHTEST
    ld DE, BackgroundMap1
    ld BC, SCREEN_BYTE_WIDTH * SCREEN_BYTE_HEIGHT
    call setVRAM    
endm

backToMainMenu: macro
    resetBackground
    ldAny [state], MAIN_MENU_STATE
    ldAny [cursorPosition], -1        
    ldAny [stateInitialised], 0 
endm

    ENDC