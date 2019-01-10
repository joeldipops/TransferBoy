    IF !DEF(SGB_COMMANDS_INCLUDED)
SGB_COMMANDS_INCLUDED SET 1

INCLUDE "addresses.asm"
INCLUDE "constants.asm"
INCLUDE "ops.asm"

SGB_PACKET_SIZE EQU 16

;;;
; Transfers command to SuperGameBoy 
; Uses data at sgbTransferPacket memory address.
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

    ; Wait for a few frames (280024 clocks)
    ld BC, 7000                ; 12 cycles
.loopWait:
        nop                             ; 4 cycles
        nop                             ; 4 cycles
        nop                             ; 4 cycles
        dec BC
        orAny B, C                      ; 16 cycles
    jr NZ, .loopWait   ; 12 cycles if jumps, 8 if not

    pop DE
    pop BC
    pop HL

    ei    
    ret

;;;
; Command Codes:
; $00 - PAL01, $01 - PAL23, $02 - $PAL03, $03 - PAL12
;
; Transfers colour settings for SGB palettes.
; @param SP + 2 Palette p data.
; @param SP + 4 Palette q data.
; @param HL Indicates the palettes being set.
;;;
PALpq:
    push HL
    push BC

    ld HL, SP + 6 ; return address & two pushes 
    ld16RR B,C, H,L

    ld HL, sgbTransferPacket0
    ldHLi [HL], %00000001 ; Command code $00, 1 packet
    REPT 14
        ld A, [C]
        ldi [HL], A
        inc C
    ENDR

    ; Pad out 1 byte at the end.
    ld16RR D,E, H,L

    ld BC, 1
    xor A
    rst memset

    call transferSgbPackets

    pop BC
    pop HL
    ret

;;;
; Command Code:
; $05 - ATTR_LIN
;
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
    ld HL, SP + 6
    ld [DE], A
    inc DE
    ldAny [DE], $01 ; data group 1
    inc DE
    REPT 14
        ldiAny [DE], [HL]
        inc DE
    ENDR

    call transferSgbPackets
    pop DE
    pop HL
    ret

;;;
; Command Codes:
; $11 - MLT_REQ
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

    call transferSgbPackets
    ret

;;;
; Command Codes:
; $14 - PC_TRN
;
; Transfers screen color data for sgb frame
;;;
PCT_TRN:
    ld HL, sgbTransferPacket0
    ldHLi [HL], %10100001
    ; Pad out 15 bytes.
    REPT 15
        ldHLi [HL], 0
    ENDR
    ret
    ENDC