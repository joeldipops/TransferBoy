INCLUDE "addresses.asm"
INCLUDE "constants.asm"
INCLUDE "ops.asm"
INCLUDE "utilMacros.asm"

SECTION "errorHandler", ROM0[$0000]
;;;
; Alias for 'rst handleError'
;;;
throw: macro
    rst handleError
endm

;;;
; Do something with the call stack and then hit the reset button.
;;;
handleError EQU $0000
rst_handleError:
    ; Do something with the stack
    pop HL
    ; Then restart from the beginning
    jp main


;;;
; Set a block of bytes to a single value.
; (Normally 0)
; @param A The value
; @param DE The address
; @param BC The number of bytes
;;;
SECTION "memset", ROM0[$0010]
memset EQU $0010
rst_memset:
    push HL
    ld L, A
.loop
        ldAny [DE], L
        inc DE
        dec BC
    orAny B, C
    jr NZ, .loop
    pop HL
    ret

;;;
; Copies memory from source to destination
; @reg HL Source address
; @reg DE Destination address
; @reg BC Number of bytes to copy.
;;;
SECTION "memcpy", ROM0[$20]
memcpy EQU $0020
rst_memcpy:
.loop
        ldiAny [DE], [HL]
        inc DE
        dec BC
    orAny B, C
    jr NZ, .loop
    ret

SECTION "Vblank", ROM0[$0040]
    jp onVBlank

SECTION "LCDC", ROM0[$0048]
    jp onHBlank

SECTION "TimerOverflow", ROM0[$0050]
    jp onTimeout

SECTION "Serial", ROM0[$0058]
    jp onTransfer

SECTION "JoypadPressed", ROM0[$0060]
    jp onJoypadEvent

SECTION "main", ROM0[$0100]
    nop
    jp main
    
    ; HEADER START
    ; not adding the logo here #illegal ;)
    db $ff,$ff,$ff,$ff,$ff,$ff,$ff,$ff,$ff,$ff,$ff,$ff,$ff,$ff,$ff,$ff
    db $ff,$ff,$ff,$ff,$ff,$ff,$ff,$ff,$ff,$ff,$ff,$ff,$ff,$ff,$ff,$ff
    db $ff,$ff,$ff,$ff,$ff,$ff,$ff,$ff,$ff,$ff,$ff,$ff,$ff,$ff,$ff,$ff
    db $00
    db "SUPERTESTBOY",0,0 ; name
    db NO_COLOUR_SUPPORT ; Colour type 
    dw UNLICENSED ; Licensee
    db SGB_SUPPORTED ; SuperGameBoy support
    db ROM_ONLY ; Cartridge structure type
    db ROM_32KB ; ROM size
    db RAM_8KB ; RAM size
    db FOREIGN_DESTINATION ; Destination code
    db USE_NEW_LICENSEE_CODE ; old licensee code
    db 0 ; Mask ROM version - rgbfix sets this
    db 0 ; complement check - rgbfix sets this
    db 0 ; checksum - rgbfix sets this

;;;
; Main loop
; Never returns.
;;;
main:
    nop
    ; Initialisation.
    di

    ; turn off outputs
    call turnOffScreen
    resH AUDIO_ON_BIT, [AudioState]

    ld SP, stackFloor

    ; 0 Out the RAM
    ld A, 0
    ld DE, $c000
    ld BC, $1fff
    rst memset
    ;call memset

    ; Use the default palette everywhere to start with
    ld A, BG_PALETTE
    ldh [BackgroundPalette], A
    ld A, FG_PALETTE
    ldh [SpritePalette1], A
    ldh [SpritePalette0], A
    
    ld A, 0
    ldh [BackgroundScrollX], A
    ldh [BackgroundScrollY], A    

    ; Clear OAM
    ld DE, OamStage
    ld BC, OAM_SIZE
    call memset

    ; Copy the ROM tile data to VRAM
    ld HL, Graphics
    ld DE, TileData
    ld BC, GraphicsEnd - Graphics
    call memcpy

    ld HL, Ascii
    ; First relevant character is $20 ascii
    ld DE, TileData + ($20 * TILE_SIZE) 
    ld BC, AsciiEnd - Ascii
    call memcpy

    ; Set the tile map to all the same colour.
    call resetBackground

    ; Copy runDma routine to HRAM, since that's the only place the CPU can call it from
    ld HL, runDmaRom ; Source in ROM
    ld DE, runDma    ; Destination in HRAM
    ld BC, (runDmaRomEnd - runDmaRom)
    rst memcpy
    ;rst $10 ; memcpy

    ; Turn the screen on and set it up.
    ldhAny [LcdControl], \
        LCD_ON | BACKGROUND_ON |  SPRITES_ON | WINDOW_OFF | SQUARE_TILES \
        | BACKGROUND_MAP_1 | WINDOW_MAP_2 | TILE_DATA_80    

    ; Set software variables
    ldAny [state], INIT_STATE
    ldAny [inputThrottleAmount], INPUT_THROTTLE    

    ; Set up the menu cursors. We can have up to 4 levels of menus, 
    ; and we preserve the last cursor position at each level.
    ld HL, cursorPositionBase
    ldAny [cursorPosition], H
    ldAny [cursorPosition+1], L

    ; Set the initial cursor at each level to 0
    ld A, 0
    ld DE, cursorPositionBase
    ld BC, MAX_MENU_DEPTH
    rst memset

    ldAny [stateInitialised], 0
    ei

.loop
        ; wait for VBlank, if another interrupt occurs, start waiting again.
        ld A, [InterruptFlags]
        and INTERRUPT_VBLANK
    jr Z, .loop          ; if (!isInVBlank) continue
  
        and ~INTERRUPT_VBLANK
        ld [InterruptFlags], A ; isInVBlank = 0
        
        call loadInput
        call runLogic
        call runDma
   jr .loop

audioTestStep:
    throw ; Not yet implemented

; Include routines
INCLUDE "lcd.asm"
INCLUDE "mainMenu.asm"
INCLUDE "joypadTest.asm"
INCLUDE "sgbTest.asm"

;;;
; Loads the pressed buttons in to B
; @output B - the pressed buttons in format <DpadABSS>
;;;
loadInput:
    ld A, [inputThrottleCount]
    or A
    jr Z, .else
        dec A
        ld [inputThrottleCount], A
        ld B, 0
        ret
.else
    ldAny [inputThrottleCount], [inputThrottleAmount]

    ; Set the bit that indicates we want to read A/B/Start/Select
    ldhAny [JoypadIo], JOYPAD_GET_BUTTONS

    ; Read from JoypadRegister until the value settles
    ld A, [JoypadIo]
    ld A, [JoypadIo]
    ld A, [JoypadIo]
    
    ; bits are 0 if pressed for some reason
    cpl
    ; throw away the upper four bits and store in B
    and $0f
    ld B, A
    
    ; Now we want to read the D-Pad
    ldhAny [JoypadIo], JOYPAD_GET_DPAD

    ; Read from JoypadRegister until the value settles
    ld A, [JoypadIo]
    ld A, [JoypadIo]
    ld A, [JoypadIo]

    cpl
    and $0f
    ; don't overwrite what we already have in B
    swap A
    or B
    ld B, A
    
    ; Reset Joypad register
    ldhAny [JoypadIo], JOYPAD_CLEAR
    
    ret
    
;;;
; The logic of the program.
; @param B The joypad state
;;;
runLogic:
    push BC

    ; Jump table
    cpAny [state], INIT_STATE
        call Z, initStep
    cpAny [state], MAIN_MENU_STATE
        call Z, mainMenuStep
    cpAny [state], JOYPAD_TEST_STATE
        call Z, joypadTestStep
    cpAny [state], SGB_TEST_STATE
        call Z, sgbTestStep
    cpAny [state], MLT_REQ_STATE
        call Z, mltReqStep
    cpAny [state], PALPQ_STATE
        call Z, palpqStep
    cpAny [state], AUDIO_TEST_STATE
        call Z, audioTestStep
    cpAny [state], MASK_EN_STATE
        call Z, maskEnStep
    cpAny [state], MASKED_EN_STATE
        call Z, maskedEnStep

    ; If set to a state higher than what's defined, it's an error
    ldAny C, [state] 
    cpAny MAX_STATE, C
        jr NC, .return
        throw
.return
    pop BC
    ret

;;;
; We're done with initialisation.  Move on to the menu.
; @param B Joypad button flags {SsABDULR}
;;;
initStep:
    ldAny [state], MAIN_MENU_STATE
    ret

;;;
; Interrupt handlers.
;;;
onHBlank:
onJoypadEvent:
onTransfer:
onTimeout:
onVBlank:
    pushAll
    popAll
    reti
    
;;;
; Kicks off the DMA copy to HRAM and then waits for it to complete
;;;
runDmaRom:
    ; DMA starts when it recieves a memory address
    ldhAny [DmaSourceRegister], (OamStage >> 8) ; Just the high byte, the low byte is already set (00)
    ld A, DMA_WAIT_TIME
.loop                   ; Loop until timer runs down and we can assume DMA is finished.
        dec A
    jr NZ, .loop
    ret
runDmaRomEnd:

SECTION "Strings ROM", ROM0[$2800]
INCLUDE "strings.asm"

SECTION "Graphics ROM", ROM0[$3000]
INCLUDE "tiles.asm"

SECTION "ASCII ROM", ROMX[$4000], BANK[1]
INCLUDE "imports/ibmpc1.inc"
Ascii:
    ; Import ascii character set
    chr_IBMPC1 2, 4
AsciiEnd:

SECTION "Main Ram", WRAM0[$C000]
OamStage:
PcY: db
PcX: db
PcImage: db
PcSpriteFlags: db

SpriteY: db
SpriteX: db
SpriteImage: db
SpriteFlags: db

ds 38 * SPRITE_WIDTH

; Heap pointer.
HP: ds $ff

; Program state flags
state: db
stateInitialised: db

; Guess we can go 4 menus deep 
cursorPositionBase: ds MAX_MENU_DEPTH
cursorPosition: dw
inputThrottleCount: db
inputThrottleAmount: db

sgbTransferPacket: ds 16
