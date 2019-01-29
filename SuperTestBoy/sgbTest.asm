    IF !DEF(SGB_TEST_INCLUDED)
SGB_TEST_INCLUDED SET 1

INCLUDE "sgbCommands.asm"

SGB_ITEMS_COUNT EQU 6

INIT_ITEM EQU       0
PALPQ_ITEM EQU      1
ATTR_LIN_ITEM EQU   2
MLT_REQ_ITEM EQU    3
PCT_TRN_ITEM EQU    4
MASK_EN_ITEM EQU    5

   
;;;
; Sets up super game boy test page.
;;;
initSgbTest:
    call resetBackground    
    ldAny [stateInitialised], 1

    ; Set up cursor
    ldAny [inputThrottleAmount], INPUT_THROTTLE

    ld16RA H,L, cursorPosition
    moveCursor MENU_MARGIN_TOP + SPRITE_WIDTH

    ldAny [PcX], MENU_MARGIN_LEFT
    ldAny [PcImage], LIGHTEST
    ldAny [PcSpriteFlags], HAS_PRIORITY | USE_PALETTE_0

    ; Title
    ld HL, SgbStepTitle
    ld D, 0
    ld E, 0
    call printString

    ld HL, InitialiseSGB
    ld D, 3
    ld E, INIT_ITEM + 1
    call printString

    ; Menu items
    ld HL, PalPqLabel
    ld D, 3
    ld E, PALPQ_ITEM + 1
    call printString

    ld HL, AttrLinLabel
    ld D, 3
    ld E, ATTR_LIN_ITEM + 1
    call printString

    ld HL, MltReqLabel
    ld D, 3
    ld E, MLT_REQ_ITEM + 1
    call printString

    ld HL, PctTrnLabel
    ld D, 3
    ld E, PCT_TRN_ITEM + 1
    call printString    

    ld HL, MaskEnLabel
    ld D, 3
    ld E, MASK_EN_ITEM + 1
    call printString

    ret

;;;
; Prepare background/sprites/variables for mlt_req setup
;;;
initMltReq:
    ld L, "_"
    ld A, 0
    ld D, 0
    ld E, SGB_ITEMS_COUNT + 1
    ld BC, BACKGROUND_WIDTH
    call setVRAM

    ld A, SGB_ITEMS_COUNT * SPRITE_WIDTH + MENU_MARGIN_TOP + (SPRITE_WIDTH * 3)
    ld [PcY], A
    ld [SpriteY + SPRITE_SIZE * 0], A
    ld [SpriteY + SPRITE_SIZE * 1], A
    ld [SpriteY + SPRITE_SIZE * 2], A

    ldAny [SpriteImage                  ], "1"
    ldAny [SpriteX                      ], 3 * SPRITE_WIDTH
    ldAny [SpriteFlags                  ], USE_PALETTE_1 | HAS_PRIORITY

    ldAny [SpriteImage + SPRITE_SIZE * 1], "2"
    ldAny [SpriteX     + SPRITE_SIZE * 1], 6 * SPRITE_WIDTH
    ldAny [SpriteFlags + SPRITE_SIZE * 1], USE_PALETTE_1 | HAS_PRIORITY

    ldAny [SpriteImage + SPRITE_SIZE * 2], "4"
    ldAny [SpriteX     + SPRITE_SIZE * 2], 9 * SPRITE_WIDTH
    ldAny [SpriteFlags + SPRITE_SIZE * 2], USE_PALETTE_1 | HAS_PRIORITY

    incAny [cursorPosition+1]
    ldAny [stateInitialised], 1
    ret

;;;
; After a mlt_req command, checks if it worked.
; @result B Id of joypad 1
; @result C Id of joypad 2
;
; This doesn't work in bgb. Can't figure out what's wrong. 
;;; 
checkMltReqResult:
    di
    ; Reset IO
    ldAny [JoypadIo], JOYPAD_CLEAR    
    ld A, [JoypadIo]
    ld A, [JoypadIo]
    ld A, [JoypadIo]
    ld A, [JoypadIo]

    ; Cache the result for player 1
    and A, %00001111
    ld B,  A

    ; Set these flags and read to switch to player 2
    ldAny [JoypadIo],  JOYPAD_GET_DPAD
    ld A, [JoypadIo]
    ldAny [JoypadIo], JOYPAD_GET_BUTTONS
    ld A, [JoypadIo]

    ; And reset
    ldAny [JoypadIo], JOYPAD_CLEAR    
    ld A, [JoypadIo]
    ld A, [JoypadIo]
    ld A, [JoypadIo]
    ld A, [JoypadIo]

    ; Test if joypad ID has changed 
    and A, %00001111
    ld C, A

    ei
    ret

;;;
; Set up a MLT_REQ command (just pick number of players)
;;;
mltReqStep:
    ; Init if not already
    orAny [stateInitialised], A
        call Z, initMltReq

    ; Go back if B pressed.
    cpAny B, B_BTN
    jr NZ, .notB
        ldAny [state], SGB_TEST_STATE
        backToPrevMenu
        jr .return

.notB
    ld16RA H,L, cursorPosition

    andAny B, LEFT | RIGHT | A_BTN | START
        jr Z, .return

    ld A, B

    ; Move the cursor if left or right pressed.
    cp LEFT
    jr NZ, .notLeft
        ; Don't go less than 0
        cpAny 0, [HL]
            jr Z, .notLeft
        dec [HL]
        jr .moveCursor

.notLeft    
    cp RIGHT
    jr NZ, .notRight
        cpAny 2, [HL]
            jr Z, .notRight
        inc [HL]
        jr .moveCursor

.notRight
    ; When a or start is pressed
    ; Depending on what's highlighted, set the corresponding value in B 
    ; And then run the command.
    ld B, A
    and A_BTN | START
    jr Z, .notA
        cpAny 0, [HL]
        jr NZ, .not1Player
            ld B, 1
            jr .sendCommand

.not1Player
        cpAny 1, [HL]
        jr NZ, .not2Player
            ld B, 2
            jr .sendCommand

.not2Player
        cpAny 2, [HL]
        jr NZ, .not4Player
            ld B, 4
            jr .sendCommand

.sendCommand
    call MLT_REQ
    call checkMltReqResult
    jr .return    

.else

.not4Player
        throw

.notA
.moveCursor
    cpAny 0, [HL]
    jr NZ, .notCursor0
        ldAny [PcX], 2 * SPRITE_WIDTH
        jr .return

.notCursor0
    cpAny 1, [HL]
    jr NZ, .notCursor1
        ldAny [PcX], 5 * SPRITE_WIDTH
        jr .return

.notCursor1
    cpAny 2, [HL]
    jr NZ, .notCursor2
        ldAny [PcX], 8 * SPRITE_WIDTH

.notCursor2
.return
    ret
;;;
; Shows the MLT_REQ submenu where you can select number of players to request.
;;;
mltReqSelected:
    ldAny [stateInitialised], 0
    ldAny [state], MLT_REQ_STATE
    ret

;;;
; Sets up and makes a PALpq command.  
; Palette numbers and colours will eventually be selectable. 
;;;
executePalpq:
    push HL
    ; push Palette q
    pushColour $1f, 0, $1f
    pushColour $1f, $1f, 0
    pushColour $0, $1f, $1f

    ; push Palette p
    pushColour $1f, 0, 0
    pushColour $0, $1f, 0
    pushColour 0, 0, $1f

    ; push shared colour.
    pushColour 0, 0, 0

    call PALpq

    add SP, 14

    ldAny [state], SGB_TEST_STATE
    backToPrevMenu

    pop HL
    ret

;;;
; Heap offsets for palpq state.
;;;
PQ SET 0
; Each colour is a word
P_COLOUR_0 SET 1
P_COLOUR_1 SET 3
P_COLOUR_2 SET 5
P_COLOUR_3 SET 7
Q_COLOUR_1 SET 9
Q_COLOUR_2 SET 11
Q_COLOUR_3 SET 13
CURRENT_COLOUR SET 15
COLOUR_STRING SET 17


colourLocations:
    db $02,$0B
    db $02,$0C
    db $02,$0D
    db $02,$0E

    db $0C,$0C
    db $0C,$0D
    db $0C,$0E

;;;
; Adds the primary colour value to the buffer.
; @param A Primary Colour 5yte 
; @param DE Next buffer address
; @affects DE += 2
; @affects B 
;;;
writeColour:
    ; Cache A
    ld B, A

    ; First character can either be 0 or 1, based on bit 4
    bit 4, A
    jr NZ, .topBitSet
        ld A, "0"
        jr .topBitSetEnd
.topBitSet
        ld A, "1"
.topBitSetEnd
    ld [DE], A
    inc DE

    ; Second character can be 0 - F
    ld A, B
    and %00001111

    cp $a 
    jr NC, .gt9
        ; if A < 10, add $30 to get the right offset
        ld B, $30
        jr .gt9End
.gt9
        ; otherwise, add $41
        ld B, $41
.gt9End
    add B
    ld [DE], A
    inc DE

    ret

;;;
; Displays two palette's worth of colour values so that PALpq can be set up
;;;
renderPalPqColours:
    push DE
    push BC

    ld C, 0 ; Number of colours
    ld HL, HP+ P_COLOUR_0
.loop
        ld DE, HP+ COLOUR_STRING ; Buffer being written to.

        ldiAny [HP+ CURRENT_COLOUR], [HL]
        ldiAny [HP+ CURRENT_COLOUR+1], [HL]

        ; Red is in the top byte.
        ld A, [HP+ CURRENT_COLOUR]
        srl A
        srl A
        and %00011111
        call writeColour

        ; Green crosses two bytes
        push DE
        ld16RA D,E, HP+ CURRENT_COLOUR
        REPT 3
            sla D
            rlc E
        ENDR
        andAny D, %00011000
        ld D, A
        andAny E, %00000111
        or D
        pop DE
        call writeColour        

        ; Blue is simple.
        ld A, [HP+ CURRENT_COLOUR+1]
        and %00011111
        call writeColour

        ; 0 terminate the string.
        xor A
        ld [DE], A
        inc DE

        push HL

        ; Place each colour string in a location specified in the colourLocations array
        ld HL, colourLocations
        ld B, 0

        ; Add C twice because each location is two bytes long 
        add HL, BC
        add HL, BC

        ld D, [HL]
        inc HL
        ld E, [HL]

        ld HL, HP+ COLOUR_STRING
        call printString
        pop HL

        inc C
        ld A, C
    cp 7
    jr NZ, .loop

    pop BC
    pop DE
    ret

;;;
; Sets up the palpq submenu.
;;;
initPalpq:
    ld L, "_"
    ld A, 0
    ld D, 0
    ld E, SGB_ITEMS_COUNT + 1
    ld BC, BACKGROUND_WIDTH
    call setVRAM

    ld A, SGB_ITEMS_COUNT * SPRITE_WIDTH + MENU_MARGIN_TOP + (SPRITE_WIDTH * 3)
    ld [PcY], A

    ld A, SGB_ITEMS_COUNT + 3
    
    ; Palette Selection
    ld HL, Pal01
    ld D, 2
    ld E, A  
    call printString

    ld HL, Pal23
    ld D, 7
    ld E, A  
    call printString

    ld HL, Pal03
    ld D, 12
    ld E, A  
    call printString

    ld HL, Pal12
    ld D, 17
    ld E, A  
    call printString

    ; Initialise the heap
    ld DE, HP+ P_COLOUR_0
    ld BC, Q_COLOUR_3 + 1
    xor A
    rst memset

    call renderPalPqColours

    incAny [cursorPosition+1]
    ldAny [stateInitialised], 1
    ret   

;;;
; Allows a PALpq command to be set up with selection of palettes and colours for each.
;;;
palpqStep:
    orAny [stateInitialised], A
        call Z, initPalpq

    ; If no button pressed, do nothing.
    andAny B, A_BTN | START | B_BTN | UP | DOWN | LEFT | RIGHT 
        jr Z, .return

    ; Back to previous menu if B pressed.
    andAny B, B_BTN
    jr Z, .notB
        ldAny [state], SGB_TEST_STATE
        backToPrevMenu
        jr .return
.notB
    andAny B, A_BTN | START
        ;onPalpqAPressed


.return    
    ret

palpqSelected:
    ldAny [stateInitialised], 0
    ldAny [state], PALPQ_STATE
    ret

;;;
; Sets up the mask_en submenu.
;;;
initMaskEn:
    ld L, "_"
    ld A, 0
    ld D, 0
    ld E, SGB_ITEMS_COUNT + 1
    ld BC, BACKGROUND_WIDTH
    call setVRAM

    ld A, SGB_ITEMS_COUNT * SPRITE_WIDTH + MENU_MARGIN_TOP + (SPRITE_WIDTH * 3)
    ld [PcY], A

    ld A, SGB_ITEMS_COUNT + 3
    ld HL, MaskFrozen
    ld D, 3
    ld E, A  
    call printString

    ld HL, MaskBlack
    ld D, 9
    ld E, A  
    call printString

    ld HL, MaskColour
    ld D, 15
    ld E, A  
    call printString    

    incAny [cursorPosition+1]
    ldAny [stateInitialised], 1
    ret

;;;
; State where user can choose what type of mask_en to send.
; @param Joypad input state.
;;;
maskEnStep:
    ; Init if not already
    orAny [stateInitialised], A
        call Z, initMaskEn

    ; Go back if B pressed.
    cpAny B, B_BTN
    jr NZ, .notB
        ldAny [state], SGB_TEST_STATE
        backToPrevMenu
        jr .return

.notB
    ld16RA H,L, cursorPosition

    andAny B, LEFT | RIGHT | A_BTN | START
        jr Z, .return

    ; Move the cursor if left or right pressed.
    andAny B, LEFT
    jr Z, .notLeft
        ; Don't go less than 0
        cpAny 0, [HL]
            jr Z, .notLeft
        dec [HL]
        jr .moveCursor

.notLeft    
    andAny B, RIGHT
    jr Z, .notRight
        ; Don't go more than 2
        cpAny 2, [HL]
            jr Z, .notRight
        inc [HL]
        jr .moveCursor

.notRight
    ; When a or start is pressed
    ; Depending on what's highlighted, set the corresponding value in B 
    ; And then run the command.
    andAny B, A_BTN | START
    jr Z, .notA
        cpAny 0, [HL]
        jr NZ, .notFrozen
            ld C, MASK_FROZEN
            jr .sendCommand

.notFrozen
        cpAny 1, [HL]
        jr NZ, .notBlack
            ld C, MASK_BLACK
            jr .sendCommand

.notBlack
        cpAny 2, [HL]
        jr NZ, .notColour
            ld C, MASK_COLOUR
            jr .sendCommand

.sendCommand
    call MASK_EN
    ldAny [state], MASKED_EN_STATE

    ; Reset the joypad and wait a bit before accepting new input
    ; so we don't immediately unmask again.
    ld B, 0
    ldAny [inputThrottleCount], 32    
    jr .return    

.notColour
        throw

.notA
.moveCursor
    cpAny 0, [HL]
    jr NZ, .notCursor0
        ldAny [PcX], 2 * SPRITE_WIDTH
        jr .return

.notCursor0
    cpAny 1, [HL]
    jr NZ, .notCursor1
        ldAny [PcX], 8 * SPRITE_WIDTH
        jr .return

.notCursor1
    cpAny 2, [HL]
    jr NZ, .notCursor2
        ldAny [PcX], 14 * SPRITE_WIDTH

.notCursor2
.return
    ret

;;;
; Waits for a button press once the screen is masked.
; @param B Joypad state.
;;;
maskedEnStep:
    xor A
    ; Wait for a button press.
    or B
        ret Z

    ld C, MASK_NONE
    call MASK_EN
    ldAny [state], MASK_EN_STATE        
    ret


;;;
; Goes to next step after selecting a command to send.
;;;
sgbItemSelected:
    ld16RA H,L, cursorPosition
    ld A, [HL]

    cp INIT_ITEM
    jr NZ, .notINIT
        call initialiseSGB
        jr .return

.notINIT
    cp MLT_REQ_ITEM
    jr NZ, .notMLT_REQ
        call mltReqSelected
        jr .return

.notMLT_REQ
    cp PALPQ_ITEM
    jr NZ, .notPALPQ
        ; for now, send PAL01 specifically and point to some random data
        call palpqSelected
        jr .return

.notPALPQ
    cp ATTR_LIN_ITEM
    jr NZ, .notATTR_LIN
        ld C, 1 ;1 packet
        call ATTR_LIN
        jr .return

.notATTR_LIN
    cp MASK_EN_ITEM
    jr NZ, .notMASK_EN
        ldAny [state], MASK_EN_STATE
        ldAny [stateInitialised], 0

        jr .return

.notMASK_EN
    cp PCT_TRN_ITEM
    jr NZ, .notPCT_TRN
        call PCT_TRN
        jr .return

.notPCT_TRN
    throw

.return
    ret

;;;
; Handles input to select an sgb command.
;;;
sgbTestStep:
    push HL
    ld A, [stateInitialised]
    or A
        call Z, initSgbTest

    ; Go back if B is pressed
    andAny B, B_BTN
    jr Z, .notB
        ldAny [state], MAIN_MENU_STATE
        backToPrevMenu
        jr .return

.notB
    ; Nothing pressed, so return
    andAny B, START | A_BTN | DOWN | UP
        jr Z, .return

    ld16RA H,L, cursorPosition
    andAny B, UP
        jr Z, .notUp

        orAny [HL], [HL]
        jr Z, .notUp
            dec [HL]

.notUp
    andAny B, DOWN
        jr Z, .notDown
        
        ; Do nothing if alread at bottom of menu.
        cpAny SGB_ITEMS_COUNT - 1, [HL]
        jr Z, .notDown
            inc [HL]

.notDown
    andAny B, START | A_BTN
    jr Z, .notA
        call sgbItemSelected
        jr .return

.notA
    moveCursor MENU_MARGIN_TOP + SPRITE_WIDTH 

.return
    pop HL
    ret

    ENDC
