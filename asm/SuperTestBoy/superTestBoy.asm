INCLUDE "addresses.asm"
INCLUDE "constants.asm"
; constants and addresses here for now since I don't have an ide to play around with files atm
;;;;;;;;;;;;;; HEADER OPTIONS ;;;;;;;;;;;;;;;;;
NO_COLOUR_SUPPORT EQU $00
COLOUR_SUPPORT EQU $80
COLOUR_ONLY EQU $C0

SGB_SUPPORTED EQU $03
ROM_ONLYRomOnly EQU $00
ROM_MBC1 EQU $01
ROM_MBC1_RAM EQU $02

ROM_32KB EQU $00
RAM_8KB EQU $02

FOREIGN_DESTINATION EQU $01

USE_NEW_LICENSEE_CODE EQU $33

;;;;;;;;;;;;;;; ADDRESSES ;;;;;;;;;;;;;;;;;;;;;
InterruptFlags EQU $ff0f
VBlankFlag EQU $01

JoypadRegister EQU $ff00
GetButtonsBit EQU %0010000
GetDPadBit EQU %00010000


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

SECTION "Vblank", ROM0[$0040]
    pushAll
    popAll
    reti

SECTION "LCDC", ROM0[$0048]
    pushAll
    popAll
    reti

SECTION "Timer_Overflow", ROM0[$0050]
    pushAll
    popAll
    reti

SECTION "Serial", ROM0[$0058]
    pushAll
    popAll
    reti

SECTION "p1thru4", ROM0[$0060]
    pushAll
    popAll
    reti

SECTION "start", ROM0[$0100]
    nop
    jp mainLoop
    
    ; HEADER START
    ; not adding the logo here #illegal ;)
    db $ff,$ff,$ff,$ff,$ff,$ff,$ff,$ff,$ff,$ff,$ff,$ff,$ff,$ff,$ff,$ff
    db $ff,$ff,$ff,$ff,$ff,$ff,$ff,$ff,$ff,$ff,$ff,$ff,$ff,$ff,$ff,$ff
    db $ff,$ff,$ff,$ff,$ff,$ff,$ff,$ff,$ff,$ff,$ff,$ff,$ff,$ff,$ff,$ff
    
    db "SUPERTESTBOY,0,0,0 ; name
    db NO_COLOUR_SUPPORT ; Colour type 
    db 0,0 ; Licensee
    db SGB_SUPPORTED ; SuperGameBoy support
    db ROM_ONLY ; Cartridge structure type
    db ROM32KB ; ROM size
    db FOREIGN_DESTINATION ; Destination code
    db USE_NEW_LICENSEE_CODE ; old licensee code
    db 0 ; Mask ROM version - what should this be?
    db 0 ; complement check - ""
    db 0 ; checksum - ""
    
    

    ROM_HEADER ROM_NOMBC, ROM_SIZE_32KBYTE, RAM_SIZE_0KBYTE
    
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
        ld [InterruptFlags] ; isInVBlank = 0
        
  
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
    ld A, GetButtonsBit
    ld [JoyPadRegister], A

    ; Read from JoyPadRegister until the value settles
    ld A, [JoyPadRegister]
    ld A, [JoyPadRegister]
    ld A, [JoyPadRegister]
    
    ; bits are 0 if pressed for some reason
    cpl
    ; throw away the upper four bits and store in B
    and $0f
    ld B, A
    
    ; Now we want to read the D-Pad
    ld A, GetDPadBit
    ld [JoyPadRegister], A

    ; Read from JoyPadRegister until the value settles
    ld A, [JoyPadRegister]
    ld A, [JoyPadRegister]
    ld A, [JoyPadRegister]
    
    cpl
    and $0f
    ; don't overwrite what we already have in B
    swap
    or B
    ld B, A
    
    ; Reset joypad register
    ld A, $30
    ld [JoyPadRegister], A
    
    
    ret

pop AF
    ret
    
runLogic:
    ret  

    

  
  
  
  
  
  
