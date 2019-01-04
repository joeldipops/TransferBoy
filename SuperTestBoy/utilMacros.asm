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
; Resets background to all light tiles.
;;;
resetBackground: macro
    ld A, LIGHTEST
    ld DE, BackgroundMap1
    ld BC, SCREEN_BYTE_WIDTH * SCREEN_BYTE_HEIGHT
    call setVRAM    
endm

;;;
; Loads the position of the current cursor in to HL
; @reg HL is loaded with cursor position.
;;;
loadCursorPosition: macro
    ldAny H, [cursorPosition]
    ldAny L, [cursorPosition+1]
endm

;;;
; Jumps back to the previous menu level.
;;;
backToMainMenu: macro
    resetBackground
    ldAny [state], MAIN_MENU_STATE
    
    ; Set position to 0 and dec depth.
    loadCursorPosition
    xor A
    ld [HL], A
    dec HL
    decAny [cursorPosition + 1]

    ldAny [stateInitialised], 0 
endm

    ENDC