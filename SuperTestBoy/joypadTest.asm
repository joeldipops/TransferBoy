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
    resetBackground
    ldAny [PcX], 0
    ldAny [PcY], 0

    ldhAny [SpritePalette2], %01001110    

    ld HL, SpriteY
    ; The first parameter to ldHLi is ignored, but we can use it to keep track of where we're up to ldi wise.
    ldHLi [A_SPRITE + SpriteY], MENU_MARGIN_TOP
    ldHLi [A_SPRITE + SpriteX], MENU_MARGIN_LEFT 
    ldHLi [A_SPRITE + SpriteImage], "A"
    ldHLi [A_SPRITE + SpriteFlags], HAS_PRIORITY | USE_PALETTE_1

    ldHLi [B_SPRITE + SpriteY], MENU_MARGIN_TOP
    ldHLi [B_SPRITE + SpriteX], MENU_MARGIN_LEFT * 2
    ldHLi [B_SPRITE + SpriteImage], "B"
    ldHLi [B_SPRITE + SpriteFlags], HAS_PRIORITY | USE_PALETTE_1

    ldHLi [SpriteY + SPRITE_SIZE * 2], MENU_MARGIN_TOP
    ldHLi [SpriteX + SPRITE_SIZE * 2], MENU_MARGIN_LEFT * 3
    ldHLi [SpriteImage + SPRITE_SIZE * 2], "S"
    ldHLi [SpriteFlags + SPRITE_SIZE *2], HAS_PRIORITY | USE_PALETTE_1

    ldHLi [SpriteY + SPRITE_SIZE * 3], MENU_MARGIN_TOP
    ldHLi [SpriteX + SPRITE_SIZE * 3], MENU_MARGIN_LEFT * 4
    ldHLi [SpriteImage + SPRITE_SIZE * 3], "s"
    ldHLi [SpriteFlags + SPRITE_SIZE * 3], HAS_PRIORITY | USE_PALETTE_1

    ldHLi [SpriteY + SPRITE_SIZE * 4], MENU_MARGIN_TOP
    ldHLi [SpriteX + SPRITE_SIZE * 4], MENU_MARGIN_LEFT * 5
    ldHLi [SpriteImage + SPRITE_SIZE * 4], "U"
    ldHLi [SpriteFlags + SPRITE_SIZE * 4], HAS_PRIORITY | USE_PALETTE_1

    ldHLi [SpriteY + SPRITE_SIZE * 5, MENU_MARGIN_TOP
    ldHLi [SpriteX + SPRITE_SIZE * 5], MENU_MARGIN_LEFT * 6
    ldHLi [SpriteImage + SPRITE_SIZE * 5], "D"
    ldHLi [SpriteFlags + SPRITE_SIZE * 5], HAS_PRIORITY | USE_PALETTE_1

    ldHLi [SpriteY + SPRITE_SIZE * 6], MENU_MARGIN_TOP
    ldHLi [SpriteX + SPRITE_SIZE * 6], MENU_MARGIN_LEFT * 7
    ldHLi [SpriteImage + SPRITE_SIZE * 6], "L"
    ldHLi [SpriteFlags + SPRITE_SIZE * 6], HAS_PRIORITY | USE_PALETTE_1

    ldHLi [SpriteY + SPRITE_SIZE * 7], MENU_MARGIN_TOP
    ldHLi [SpriteX + SPRITE_SIZE * 7], MENU_MARGIN_LEFT * 8
    ldHLi [SpriteImage + SPRITE_SIZE * 7], "R"
    ldHLi [SpriteFlags + SPRITE_SIZE * 7], HAS_PRIORITY | USE_PALETTE_1

    ldAny [stateInitialised], 1        
    
    pop HL
    ret

;;;
; Puts us back to how we were before we entered this screen.
;;;
back:

    ldAny [SpritePalette2], FG_PALETTE

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
                           
    backToMainMenu
    ret

;;;
; Screen that reacts to each button press.
; @param B pressed buttons.
;;;
joypadTestStep:
    ; Init if haven't already
    ld A, [stateInitialised]
    or A
        call Z, initJoypadTest
    
    ; Go back if B pressed
    ; Yes I know this is kinda incompatible with the whole "Test the joypad" thing.
    ; I'll change it to A & B & START & SELECT when I get it working.
    cpAny B, B_BTN
        call Z, back

    andAny B, A_BTN
    ld A, [A_SPRITE + SpriteFlags]
    jr NZ, .elseA
        or USE_PALETTE_0
        jr .endA
.elseA
        or USE_PALETTE_1   
.endA
    ld [A_SPRITE + SpriteFlags], A

    andAny B, START
    ld A, [START_SPRITE + SpriteFlags]
    jr NZ, .elseStart
        or USE_PALETTE_0
        jr .endStart
.elseStart
        or USE_PALETTE_1   
.endStart
    ld [START_SPRITE + SpriteFlags], A

.elseSelect
.endSelect
.elseU
.elseD
.elseL
.elseR

    ret
    
    ENDC
