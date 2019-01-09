    IF(!DEF(JOYPAD_TEST_INCLUDED))
JOYPAD_TEST_INCLUDED SET 1

A_SPRITE EQU 0
B_SPRITE EQU SPRITE_SIZE
START_SPRITE EQU SPRITE_SIZE * 2
SELECT_SPRITE EQU SPRITE_SIZE * 3
UP_SPRITE EQU SPRITE_SIZE * 4
DOWN_SPRITE EQU SPRITE_SIZE * 5
LEFT_SPRITE EQU SPRITE_SIZE * 6
RIGHT_SPRITE EQU SPRITE_SIZE * 7

;;;
; Sets up sprites for each buttons.
;;;
initJoypadTest:
    push HL
    call resetBackground
    ldAny [PcX], 0
    ldAny [PcY], 0

    ldhAny [SpritePalette1], %01001110    

    ld HL, SpriteY
    ; The first parameter to ldHLi is ignored, but we can use it to keep track of where we're up to ldi wise.
    ldHLi [A_SPRITE + SpriteY], MENU_MARGIN_TOP
    ldHLi [A_SPRITE + SpriteX], MARGIN_LEFT 
    ldHLi [A_SPRITE + SpriteImage], "A"
    ldHLi [A_SPRITE + SpriteFlags], HAS_PRIORITY | USE_PALETTE_1

    ldHLi [B_SPRITE + SpriteY], MENU_MARGIN_TOP
    ldHLi [B_SPRITE + SpriteX], MARGIN_LEFT * 2
    ldHLi [B_SPRITE + SpriteImage], "B"
    ldHLi [B_SPRITE + SpriteFlags], HAS_PRIORITY | USE_PALETTE_1

    ldHLi [START_SPRITE + SpriteY], MENU_MARGIN_TOP
    ldHLi [START_SPRITE + SpriteX], MARGIN_LEFT * 3
    ldHLi [START_SPRITE + SpriteImage], "S"
    ldHLi [START_SPRITE + SpriteFlags], HAS_PRIORITY | USE_PALETTE_1

    ldHLi [SELECT_SPRITE + SpriteY], MENU_MARGIN_TOP
    ldHLi [SELECT_SPRITE + SpriteX], MARGIN_LEFT * 4
    ldHLi [SELECT_SPRITE + SpriteImage], "s"
    ldHLi [SELECT_SPRITE + SpriteFlags], HAS_PRIORITY | USE_PALETTE_1

    ldHLi [UP_SPRITE + SpriteY], MENU_MARGIN_TOP
    ldHLi [UP_SPRITE + SpriteX], MARGIN_LEFT * 5
    ldHLi [UP_SPRITE + SpriteImage], "U"
    ldHLi [UP_SPRITE + SpriteFlags], HAS_PRIORITY | USE_PALETTE_1

    ldHLi [DOWN_SPRITE + SpriteY], MENU_MARGIN_TOP
    ldHLi [DOWN_SPRITE + SpriteX], MARGIN_LEFT * 6
    ldHLi [DOWN_SPRITE + SpriteImage], "D"
    ldHLi [DOWN_SPRITE + SpriteFlags], HAS_PRIORITY | USE_PALETTE_1

    ldHLi [LEFT_SPRITE + SpriteY], MENU_MARGIN_TOP
    ldHLi [LEFT_SPRITE + SpriteX], MARGIN_LEFT * 7
    ldHLi [LEFT_SPRITE + SpriteImage], "L"
    ldHLi [LEFT_SPRITE + SpriteFlags], HAS_PRIORITY | USE_PALETTE_1

    ldHLi [RIGHT_SPRITE + SpriteY], MENU_MARGIN_TOP
    ldHLi [RIGHT_SPRITE + SpriteX], MARGIN_LEFT * 8
    ldHLi [RIGHT_SPRITE + SpriteImage], "R"
    ldHLi [RIGHT_SPRITE + SpriteFlags], HAS_PRIORITY | USE_PALETTE_1

    ; Message at the bottom of the screen.
    ld HL, JoypadTestInstructions
    ld D, 0
    ld E, BACKGROUND_HEIGHT - 1
    call printString 

    ; Turn off input throttle so holding down the button counts.
    ldAny [inputThrottleAmount], 0
    ldAny [stateInitialised], 1        
   
    pop HL
    ret

;;;
; Puts us back to how we were before we entered this screen.
;;;
backFromJoypadTest:
    ldAny [SpritePalette1], FG_PALETTE

    ; hide all the sprites off screen.
    ld A, 0
    ld [A_SPRITE + SpriteY], A
    ld [B_SPRITE + SpriteY], A
    ld [START_SPRITE + SpriteY], A
    ld [SELECT_SPRITE + SpriteY], A
    ld [UP_SPRITE + SpriteY], A
    ld [DOWN_SPRITE + SpriteY], A
    ld [LEFT_SPRITE + SpriteY], A
    ld [RIGHT_SPRITE + SpriteY], A
                           
    ; Wait a bit before allowing the next input so we don't keep jumping back in to joypad test after holding down A & START 
    ldAny [inputThrottleCount], 32

    ldAny [state], MAIN_MENU_STATE
    backToPrevMenu
    ret

;;;
; Lights up the indicator if the button is pressed, turns it off if it is not.
; @param \1 The button
; @param \2 Sprite for the button.
; @reg B pressed buttons.
;;; 
setButtonIndicator: macro
    andAny B, \1
    jr NZ, .else\@
        ldAny [\2 + SpriteFlags], HAS_PRIORITY | USE_PALETTE_1
        jr .end\@
.else\@
        ldAny [\2 + SpriteFlags], HAS_PRIORITY | USE_PALETTE_0
.end\@    
endm

;;;
; Screen that reacts to each button press.
; @param B pressed buttons.
;;;
joypadTestStep:
    ; Init if haven't already
    ld A, [stateInitialised]
    or A
        call Z, initJoypadTest
    
    ; Go back if A, B, START, SELECT all held down.
    cpAny B, A_BTN | B_BTN | START | SELECT 
        call Z, backFromJoypadTest

    setButtonIndicator A_BTN, A_SPRITE
    setButtonIndicator B_BTN, B_SPRITE
    setButtonIndicator START, START_SPRITE 
    setButtonIndicator SELECT, SELECT_SPRITE
    setButtonIndicator UP, UP_SPRITE
    setButtonIndicator DOWN, DOWN_SPRITE
    setButtonIndicator LEFT, LEFT_SPRITE
    setButtonIndicator RIGHT, RIGHT_SPRITE            

    ret
    
    ENDC
