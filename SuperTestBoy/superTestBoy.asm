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
    ei
endm

;;;
; Loads a value directly to a memory address
; @param \1 the value
; @param \2 a memory address
; @registers A
;;;
ldm: macro
    ld A, \1
    ld \2, A
endm

;;;
; Loads a value directly to a memory address greater than $ff00
; @param \1 the value
; @param \2 a memory address
; @registers A
;;;
ldhm: macro
    ldh A, \1
    ldh [\2], A
endm

;;;
; Loads a value directly to a memory address and increments HL
; @param \1 The destination address 
; @param \2 The source address (should be HL)
;;;
ldim: macro
    ldi A, [HL]
    ld \2, A
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
        ldim [DE], [HL]
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
    jr .loop
    


;;;
; Loads the pressed buttons in to B
; @output B - the pressed buttons in format <DpadABSS>
;;;
loadInput:
    push AF
    ; Set the bit that indicates we want to read A/B/Start/Select
    ldhm [JoypadRegister], GetButtonsBit

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
    ldhm [JoypadRegister], GetDPadBit

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
    ldhm [JoypadRegister], ClearJoypad
    
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
runDMA:
    push BC
    ld A, OAMStage
    ld C, DMASourceRegister
    ld [C], A           ; DMA starts when it recieves a memory address
    ld A, DMATimer
.loop                   ; Loop until timer runs down and we can assume DMA is finished.
        dec A
    jr NZ, .loop
    pop BC 
    ret

SECTION "Main Ram", WRAM0[$C000]
OAMStage:
    ds $00

  

  
  
  
  
  
  
