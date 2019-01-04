    IF !DEF(MAIN_MENU_INCLUDED)
MAIN_MENU_INCLUDED SET 1

MENU_ITEMS_COUNT EQU 3


;;;
; Handle interactions with the main menu
; @param B The joypad state 
;;;
mainMenuStep:
    push HL

    ld A, [stateInitialised]
    or A
        call Z, initMainMenu

    ; if no relevant buttons pressed.
    ld16RA H,L, cursorPosition

    andAny B, START | A_BTN | DOWN | UP
        jr Z, .return
    andAny B, UP
        jr Z, .notUp

        ; If already at top of menu, bail
        orAny [HL], [HL]
        jr Z, .notUp
            dec [HL]

.notUp
    andAny B, DOWN
    jr Z, .notDown
        ; If already at bottom of menu, bail
        cpAny MENU_ITEMS_COUNT - 1, [HL] 
        jr Z, .notDown
            inc [HL]

.notDown
    andAny B, START | A_BTN
    jr Z, .notA
        call mainMenuItemSelected
        jr .return
.notA

    ; Move the cursor
    moveCursor MENU_MARGIN_TOP
     
.return
    pop HL
    ret

;;;
; Sets up the screen for the main menu page
;;;
initMainMenu:
    ; Set up cursor
    ldAny [inputThrottleAmount], INPUT_THROTTLE
    ldAny [PcX], MENU_MARGIN_LEFT
    ldAny [PcImage], LIGHTEST
    ldAny [PcSpriteFlags], HAS_PRIORITY | USE_PALETTE_0

    ld16RA H,L, cursorPosition
    moveCursor MENU_MARGIN_TOP

    ldAny [stateInitialised], 1

    ; Set up menu items
    ld HL, SGBLabel
    ld D, 3
    ld E, 0
    call printString

    ld HL, JoypadLabel
    ld D, 3
    ld E, 1
    call printString

    ld HL, AudioLabel
    ld D, 3
    ld E, 2
    call printString
    ret

;;;
; When a menu item is selected, jumps to the next screen.
;;;
mainMenuItemSelected:
    push HL

    ld16RA H,L, cursorPosition
    cpAny 1, [HL]
        jr NZ, .notJoypad
        ldAny [state], JOYPAD_TEST_STATE
        jr .return

.notJoypad
    orAny 0, [HL]
        jr NZ, .notSGB
        ldAny [state], SGB_TEST_STATE
        jr .return

.notSGB
    cpAny 2, [HL]
        jr NZ, .notAudio
        ldAny [state], AUDIO_TEST_STATE
        jr .return  

.notAudio
    throw

.return
    ; We're changing menu levels, so inc the cursor pointer.
    incAny [cursorPosition + 1]
    ldAny [stateInitialised], 0

    pop HL
    ret

    ENDC