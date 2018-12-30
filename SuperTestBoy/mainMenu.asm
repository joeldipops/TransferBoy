    IF !DEF(MAIN_MENU_INCLUDED)
MAIN_MENU_INCLUDED SET 1

;;;
; Handle interactions with the main menu
; @param B The joypad state 
;;;
mainMenuStep:
    push HL
    ld HL, cursorPosition
    cpAny -1, [HL]
    jr NZ, .elseInit
        call initMainMenu

.elseInit
    ; if no relevant buttons pressed.
    
    andAny B, START | A_BTN | DOWN | UP
        jr Z, .return
    andAny B, UP
        jr Z, .elseUp

        ; If already at top of menu, bail
        cpAny 0, [HL]
            jr Z, .elseUp
        decAny [HL]
.elseUp
    andAny B, DOWN
        jr Z, .elseDown
        ; If already at bottom of menu, bail
        cpAny MENU_ITEMS_COUNT - 1, [HL] 
            jr Z, .elseDown
        incAny [HL]
.elseDown
    andAny B, START | A_BTN
        jr Z, .elseA
        call mainMenuItemSelected
.elseA
    ; Move the cursor
    ld A, [cursorPosition]
    mult A, 8
    add MENU_MARGIN_TOP
    ld [PcY], A
     
.return
    pop HL
    ret

;;;
; Sets up the screen for the main menu page
;;;
initMainMenu:
    ; Set up cursor
    ldAny [cursorPosition], 0
    ldAny [PcX], MENU_MARGIN_LEFT
    ldAny [PcY], MENU_MARGIN_TOP
    ldAny [PcImage], LIGHTEST
    ldAny [PcSpriteFlags], HAS_PRIORITY | USE_PALETTE_0

    ; Set up menu items
    ld HL, JoypadLabel
    ld DE, BackgroundMap1 + (MENU_MARGIN_LEFT / SPRITE_WIDTH)
    call printString

    ld HL, SGBLabel
    ld DE, BackgroundMap1 + (CANVAS_WIDTH + MENU_MARGIN_LEFT) / SPRITE_WIDTH
    call printString

    ld HL, AudioLabel
    ld DE, BackgroundMap1 + (CANVAS_WIDTH * 2 + MENU_MARGIN_LEFT) / SPRITE_WIDTH
    call printString
    ret

;;;
; When a menu item is selected, jumps to the next screen.
;;;
mainMenuItemSelected:
    push HL
    ld HL, cursorPosition
    orAny 0, [HL]
        jr NZ, .elseJoypad
        ldAny [state], JOYPAD_TEST_STATE
        jr .return
.elseJoypad
    cpAny 1, [HL]
        jr NZ, .elseSGB
        ldAny [state], SGB_TEST_STATE
        jr .return
.elseSGB
    cpAny 2, [HL]
        jr NZ, .elseAudio
        ldAny [state], AUDIO_TEST_STATE
        jr .return  

.elseAudio
    throw

.return
    pop HL
    ret

    ENDC