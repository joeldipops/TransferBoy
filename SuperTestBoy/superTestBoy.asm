INCLUDE "addresses.asm"
INCLUDE "constants.asm"

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
; Set all registers and RAM to a known start state.
; !!! Change from a macro to routine if I find I ever need to call this more than once !!!
;;;
init: macro
    di

    ; Copy runDMA routine to HRAM, since that's the only place the CPU can call it from
    ld HL, runDMAROM ; Source in ROM
    ld DE, runDMA    ; Destination in HRAM
    ld BC, (runDMAROMEnd - runDMAROM)
    rst memcpy ; rst $28 if this won't compile

    ei
endm

;;;
; Loads something directly, bypassing the accumulator;
; @param \1 the destination
; @param \2 the source
; @registers A
;;;
ldAny: macro
    ld A, \2
    ld \1, A
endm

;;;
; Loads something directly to a memory address greater than $ff00
; @param \1 the destination
; @param \2 the source
; @registers A
;;;
ldhAny: macro
    ld A, \2
    ldh \1, A
endm

;;;
; Loads a value directly to a memory address and increments HL
; @param \1 The destination address 
; @param \2 The source address (should be HL)
;;;
ldiAny: macro
    ldi A, [HL]
    ld \1, A
endm

;;;
; Performs an OR on two values other than A
;;;
orAny: macro
    ld a, \1
    or \2
endm

;;;
; Copies memory from source to destination
; @reg HL Source address
; @reg DE Destination address
; @reg BC Number of bytes to copy.
;;;
SECTION "MemCopy", ROM0[$28]
memcpy:
.loop
        ldiAny [DE], [HL]
        inc DE
        dec BC
    orAny B, C
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
    
    db "SUPERTESTBOY",0,0,0 ; name
    db NO_COLOUR_SUPPORT ; Colour type 
    dw UNLICENSED ; Licensee
    db SGB_SUPPORTED ; SuperGameBoy support
    db ROM_ONLY ; Cartridge structure type
    db ROM_32KB ; ROM size
    db FOREIGN_DESTINATION ; Destination code
    db USE_NEW_LICENSEE_CODE ; old licensee code
    db 0 ; Mask ROM version - rgbfix sets this
    db 0 ; complement check - rgbfix sets this
    db 0 ; checksum - rgbfix sets this
        
;;;
; Main loop
;;;
main:
    init
.loop
        halt
        nop

        ; wait for VBlank, if another interrupt occurs, start waiting again.
        ld A, [InterruptFlags]
        and VBlankFlag
        jr Z, .loop          ; if (!isInVBlank) continue
  
        and ~VBlankFlag
        ld [InterruptFlags], A ; isInVBlank = 0
        
        call loadInput
        call runLogic
        call runDMA
   jr .loop
    


;;;
; Loads the pressed buttons in to B
; @output B - the pressed buttons in format <DpadABSS>
;;;
loadInput:
    push AF
    ; Set the bit that indicates we want to read A/B/Start/Select
    ldhAny [JoypadRegister], GetButtonsBit

    ; Read from JoypadRegister until the value settles
    ld A, [JoypadRegister]
    ld A, [JoypadRegister]
    ld A, [JoypadRegister]
    
    ; bits are 0 if pressed for some reason
    cpl
    ; throw away the upper four bits and store in B
    and $0f
    ld B, A
    
    ; Now we want to read the D-Pad
    ldhAny [JoypadRegister], GetDPadBit

    ; Read from JoypadRegister until the value settles
    ld A, [JoypadRegister]
    ld A, [JoypadRegister]
    ld A, [JoypadRegister]
    
    cpl
    and $0f
    ; don't overwrite what we already have in B
    swap A
    or B
    ld B, A
    
    ; Reset Joypad register
    ldhAny [JoypadRegister], ClearJoypad
    
    pop AF
    ret
    
runLogic:
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
runDMAROM:
    ; DMA starts when it recieves a memory address
    ldhAny [DMASourceRegister], (OAMStage >> 8) ; Just the high byte, the low byte is already set (00)
    ld A, DMA_WAIT_TIME
.loop                   ; Loop until timer runs down and we can assume DMA is finished.
        dec A
    jr NZ, .loop
    ret
runDMAROMEnd:

SECTION "Main Ram", WRAM0[$C000]
OAMStage:
    ds $00

  

  
  
  
  
  
  
