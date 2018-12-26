INCLUDE "addresses.asm"
INCLUDE "constants.asm"
INCLUDE "ops.asm"
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
; Copies memory from source to destination
; @reg HL Source address
; @reg DE Destination address
; @reg BC Number of bytes to copy.
;;;
SECTION "MemCopy", ROM0[$0028]
memcpy:
.loop
        ldiAny [DE], [HL]
        inc DE
        dec BC
    orReg B, C
    jr NZ, .loop
    reti

SECTION "Vblank", ROM0[$0040]
    jp onVBlank

SECTION "LCDC", ROM0[$0048]
    jp onHBlank

SECTION "Timer_Overflow", ROM0[$0050]
    jp onTimeout

SECTION "Serial", ROM0[$0058]
    jp onTransfer

SECTION "p1thru4", ROM0[$0060]
    jp onJoypadEvent

SECTION "main", ROM0[$0100]
    nop
    jr main
    
    ; HEADER START
    ; not adding the logo here #illegal ;)
    db $ff,$ff,$ff,$ff,$ff,$ff,$ff,$ff,$ff,$ff,$ff,$ff,$ff,$ff,$ff,$ff
    db $ff,$ff,$ff,$ff,$ff,$ff,$ff,$ff,$ff,$ff,$ff,$ff,$ff,$ff,$ff,$ff
    db $ff,$ff,$ff,$ff,$ff,$ff,$ff,$ff,$ff,$ff,$ff,$ff,$ff,$ff,$ff,$ff
    db $00
    db "SUPERTESTBOY",0,0,0 ; name
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
; Set up registers and RAM to a known start state.
; Conceptually a routine, but implemented as a macro since we probably don't need to call it more than once.
;;;
init: macro
    di
    ; turn off the screen and audio
    call turnOffScreen
    resIO AUDIO_ON_BIT, [AudioState]

    ld SP, stackFloor

    ; 0 Out the RAM
    ld A, 0
    ld DE, $c000
    ld BC, $1fff
    call memset

    ; Use the default palette everywhere to start with
    ld A, DEFAULT_PALETTE
    ldh [BackgroundPalette], A
    ldh [SpritePalette1], A
    ldh [SpritePalette2], A

    ld A, 0
    ldh [BackgroundScrollX], A
    ldh [BackgroundScrollY], A

    ; Clear OAM
    ld DE, OamStage
    ld BC, OAM_SIZE
    call memset

    ; Copy the ROM tile data to VRAM
    ld HL, graphics
    ld DE, TileData
    ld BC, NUMBER_OF_TILES * TILE_SIZE
    call memcpy

    ; Set the tile map to all the same colour.
    ld A, 0
    ld DE, BackgroundMap1
    ld BC, SCREEN_BYTE_WIDTH * SCREEN_BYTE_HEIGHT
    call setVRAM

    ; Copy runDma routine to HRAM, since that's the only place the CPU can call it from
    ld HL, runDmaRom ; Source in ROM
    ld DE, runDma    ; Destination in HRAM
    ld BC, (runDmaRomEnd - runDmaRom)
    rst $28 ; memcpy

    ; Turn the screen on and set it up.
    ldhAny [LcdControl], \
        LCD_ON | BACKGROUND_ON |  SPRITES_ON | WINDOW_OFF | SQUARE_TILES \
        | BACKGROUND_MAP_1 | WINDOW_MAP_2 | TILE_DATA_80
    
    ei
endm
        
;;;
; Main loop
;;;
main:
    nop ; This instruction sometimes gets skipped???
    init
.loop
        halt
        nop

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

;;;
; Copy data to VRAM, but only when available
; @param A value to set
; @param DE destination start address
; @param BC length of data.
;;; 
setVRAM:
    push HL
.untilFinished  
        push AF
        di
.untilVRAM
            andAny [LcdStatus], VRAM_BUSY, H
        jr NZ, .untilVRAM
        pop AF
        ld [DE], A
        inc DE
        dec	BC
        ei
	    orReg B, C
	jr NZ, .untilFinished
    pop HL
	ret
    
;;;
; Set a block of bytes to a single value.
; (Normally 0)
; @param A The value
; @param DE The address
; @param BC The number of bytes
;;;  
memset:
.loop
        ld [DE], A
        inc DE
        dec BC
    orReg B, C
    jr NZ, .loop
    ret

;;;
; Loads the pressed buttons in to B
; @output B - the pressed buttons in format <DpadABSS>
;;;
loadInput:
    push AF
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
    
    pop AF
    ret
    
runLogic:
    ret  

;;;
; Wait until VBlank and then turn off LCD
;;;
turnOffScreen:
    ldh A, [LcdControl]
    rlca    ; Put the high bit of LCDC into the Carry flag
    ret NC  ; Screen is off already.

    ld A, SCREEN_HEIGHT    
    push HL
    ld HL, CurrentLine
; Loop until display is past the bottom of the screen
.loop
    cp [HL]
    jr NC, .loop

    resAny LCD_ON_BIT, [LcdControl]
    pop HL
    ret

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
    ;ldhAny [DmaSourceRegister], (OamStage >> 8) ; Just the high byte, the low byte is already set (00)
    ld A, DMA_WAIT_TIME
.loop                   ; Loop until timer runs down and we can assume DMA is finished.
        dec A
    jr NZ, .loop
    ret
runDmaRomEnd:

graphics:
OPT  b.x

db %........
db %........
db %........
db %........
db %........
db %........
db %........
db %........

db %.x.x.x.x
db %.x.x.x.x
db %.x.x.x.x
db %.x.x.x.x
db %.x.x.x.x
db %.x.x.x.x
db %.x.x.x.x
db %.x.x.x.x

db %x.x.x.x.
db %x.x.x.x.
db %x.x.x.x.
db %x.x.x.x.
db %x.x.x.x.
db %x.x.x.x.
db %x.x.x.x.
db %x.x.x.x.

db %xxxxxxxx
db %xxxxxxxx
db %xxxxxxxx
db %xxxxxxxx
db %xxxxxxxx
db %xxxxxxxx
db %xxxxxxxx
db %xxxxxxxx


SECTION "Main Ram", WRAM0[$C000]
OamStage:

  

  
  
  
  
  
  
