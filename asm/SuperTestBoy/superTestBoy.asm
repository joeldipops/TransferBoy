INCLUDE "constants.asm"

pushAll: macro
    push AF
    push BC
    push DE
    push HL
endm

popAll: macro
    pop HL
    pop DE
    pop BC
    pop AF
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

    ROM_HEADER ROM_NOMBC, ROM_SIZE_32KBYTE, RAM_SIZE_0KBYTE
    
main:
    halt
    nop
    ; wait for VBlank, if another interrupt occurs, start waiting again.
    ld A, [isInVBlank]
    or A
    jr Z, main          ; if !isInVBlank restart
  
    xor A               ; isInVBlank = false
    ld [isInVBlank], A
  
    call loadInput
    call runLogic
  
    jr main
    
loadInput:
    ret
    
runLogic:
    ret

  
  
  
  
  
  
