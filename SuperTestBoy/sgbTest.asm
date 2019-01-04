    IF !DEF(SGB_TEST_INCLUDED)
SGB_TEST_INCLUDED SET 1
SGB_ITEMS_COUNT EQU 3
SGB_PACKET_SIZE EQU 16

MLT_REQ_ITEM EQU    0
PALPQ_ITEM EQU      1
ATTR_LIN_ITEM EQU        2


;;;;;; Requests - put in include file ;;;;;

;;;
; Applies given palettes to specified lines.  
; @param SP+2 line data.
; @param C number of packets 
;;;
ATTR_LIN:
    push HL
    push DE
    ld DE, sgbTransferPacket0
    ld A, C
    and %00000111
    or  %00101000 ; Command code $05
    ld HL, SP+2
    ld [DE], A
    inc DE
    ldAny [DE], $01 ; data group 1
    inc DE
    REPT 14
        ldiAny [DE], [HL]
        inc DE
    ENDR

    pop DE
    pop HL
    ret

;;;
; Transfers colour settings for SGB palettes.
; @param BC First palette address
; @param DE Second palette address
; @param HL Indicates the palettes being set.
;;;
PALpq:
    push HL

    ld HL, sgbTransferPacket0
    ldHLi [HL], %00000001 ; Command code $00, 1 packet
    REPT 8
        ldHLi [HL], [BC]
        inc BC
    ENDR
    REPT 8
        ldHLi [HL], [DE]
        inc DE
    ENDR

    ; Pad out 1 byte at the end.
    ld16RR D,E, H,L
    ld BC, 1
    xor A
    call memset

    pop HL
    ret
    
;;;
; Request SGB Multiplayer mode
; @param B Number of players requested (1, 2 or 4)
;;;
MLT_REQ:
    ld HL, sgbTransferPacket0
    ldHLi [HL], %10001001

    ld A, B
    cp 1
    jr NZ, .not1
        ldHLi [HL], %00000000
        jr .continue
.not1    
    cp 2
    jr NZ, .not2
        ldHLi [HL], %00000001
        jr .continue
.not2
    cp 4
    jr NZ, .not4
        ldHLi [HL], %00000011
        jr .continue
.not4
    throw
 
.continue
    ; Set the rest of the packet to 0
    ld16RR D,E, H,L
    ld BC, 14
    xor A
    call memset
    ret

initSgbTest:
    resetBackground    
    ldAny [stateInitialised], 1

    ; Set up cursor
    ldAny [inputThrottleAmount], INPUT_THROTTLE
    ldAny [PcX], MENU_MARGIN_LEFT
    ldAny [PcY], MENU_MARGIN_TOP + SPRITE_WIDTH
    ldAny [PcImage], LIGHTEST
    ldAny [PcSpriteFlags], HAS_PRIORITY | USE_PALETTE_0

    ; Title
    ld HL, SgbStepTitle
    ld D, 0
    ld E, 0
    call printString

    ; Menu items
    ld HL, MltReqLabel
    ld D, 3
    ld E, 1
    call printString

    ld HL, PalPqLabel
    ld D, 3
    ld E, 2
    call printString

    ld HL, AttrLinLabel
    ld D, 3
    ld E, 3
    call printString

    ret

backFromSgbTest:
    ldAny [state], MAIN_MENU_STATE
    backToPrevMenu
    ret

;;;
; Transfers command to SuperGameBoy
;;;
transferSgbPackets:
    push HL
    push BC
    push DE

    ; No interrupts during a transfer please.
    di

    ; Set bits 4 & 5 of Joypad register to 0
    ldhAny [JoypadIo], 0
    ldhAny [JoypadIo], %00110000

    ld HL, sgbTransferPacket0
    ld BC, SGB_PACKET_SIZE 

.loopBytes
        ld D, 1
        ldiAny E, [HL]
.loopBits
            andAny E, D
            jr NZ, .not0
                ; Represents a 0
                ldhAny [JoypadIo], %00100000
            jr .end0    
.not0
                ; Represents a 1
                ldhAny [JoypadIo], %00010000
.end0
            ; Sent between each bit
            ldhAny [JoypadIo], %00110000

            ; Go bit by bit, so loop until D carries
            sla D
            jr NC, .loopBits

        ; Loop until BC is 0
        dec BC
    orAny B, C
    jp NZ, .loopBytes

    ; send a final 0 to end the message
    ldhAny [JoypadIo], %00100000
    ldhAny [JoypadIo], %00110000
    ldhAny [JoypadIo], 0

    pop DE
    pop BC
    pop HL

    ei
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

    ldAny [SpriteImage              ], "1"
    ldAny [SpriteX                  ], 3 * SPRITE_WIDTH
    ldAny [SpriteFlags              ], USE_PALETTE_1 | HAS_PRIORITY

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
    call transferSgbPackets

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
; Goes to next step after selecting a command to send.
;;;
sgbItemSelected:
    ld16RA H,L, cursorPosition
    ld A, [HL]
    cp MLT_REQ_ITEM
    jr NZ, .notMLT_REQ
        call mltReqSelected
        jr .return

.notMLT_REQ
    cp PALPQ_ITEM
    jr NZ, .notPALPQ
        ; for now, send PAL01 specifically and point to some random data
        ld BC, $0000
        ld DE, $0008
        ld HL, %0000000000000011
        call PALpq
        jr .return

.notPALPQ
    cp ATTR_LIN_ITEM
    jr NZ, .notATTR_LIN
        ld C, 1 ;1 packet
        call ATTR_LIN
        jr .return

.notATTR_LIN
    throw

.return
    ;call transferSgbPackets
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
    cpAny B, B_BTN
    jr NZ, .notB
        call backFromSgbTest
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
