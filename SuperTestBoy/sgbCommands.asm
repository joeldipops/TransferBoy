    IF !DEF(SGB_COMMANDS_INCLUDED)
SGB_COMMANDS_INCLUDED SET 1

INCLUDE "addresses.asm"
INCLUDE "constants.asm"
INCLUDE "ops.asm"

SGB_PACKET_SIZE EQU 16

MASK_NONE EQU   0
MASK_FROZEN EQU 1
MASK_BLACK EQU  2
MASK_COLOUR EQU  3

;;;
; Pads remainder or SGB packet with zeroes
; @param \1 Number of bytes to pad.
;;;
padHL: macro
    push DE
    push BC
        xor A
        ld16RR D,E, H,L
        ld BC, \1
        rst memset
    pop BC
    pop DE
endm

;;;
; These are sent to the SGB after a MLT_REQ to set something or other up at the SNES end.
;;;
sgbInitPackets:
    db $79,$5d,$08,$00,$0b,$8c,$d0,$f4,$60,$00,$00,$00,$00,$00,$00,$00
    db $79,$52,$08,$00,$0b,$a9,$e7,$9f,$01,$c0,$7e,$e8,$e8,$e8,$e8,$e0
    db $79,$47,$08,$00,$0b,$c4,$d0,$16,$a5,$cb,$c9,$05,$d0,$10,$a2,$28
    db $79,$3c,$08,$00,$0b,$f0,$12,$a5,$c9,$c9,$c8,$d0,$1c,$a5,$ca,$c9
    db $79,$31,$08,$00,$0b,$0c,$a5,$ca,$c9,$7e,$d0,$06,$a5,$cb,$c9,$7e
    db $79,$26,$08,$00,$0b,$39,$cd,$48,$0c,$d0,$34,$a5,$c9,$c9,$80,$d0
    db $79,$1b,$08,$00,$0b,$ea,$ea,$ea,$ea,$ea,$a9,$01,$cd,$4f,$0c,$d0
    db $79,$10,$08,$00,$0b,$4c,$20,$08,$ea,$ea,$ea,$ea,$ea,$60,$ea,$ea

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

    ld HL, sgbTransferPacket
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
; Sends 8 initialisation packets to the Super Game Boy.
;;;
initialiseSGB:
    push HL
    push BC
    push DE

    ld HL, sgbInitPackets
    ld BC, 8 
.loop
        ld DE, sgbTransferPacket

        ; Counter on the stack
        push BC

        ld BC, SGB_PACKET_SIZE
        rst memcpy
        call transferSgbPackets

        pop BC
        dec C
    jr NZ, .loop
    
    pop DE
    pop BC
    pop HL
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

    ld HL, sgbTransferPacket
    ldHLi [HL], %00000001 ; Command code $00, 1 packet

    ; TODO - just use memcpy here.
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
    ld DE, sgbTransferPacket
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
;
; Request Multiplayer mode
; @param B Number of players requested (1, 2 or 4)
;;;
MLT_REQ:
    ld HL, sgbTransferPacket
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
    rst memset

    call transferSgbPackets
    ret

;;;
; Command Codes:
; $14 - PC_TRN
;
; Transfers screen color data for sgb frame
;;;
PCT_TRN:
    ld HL, sgbTransferPacket
    ldHLi [HL], %10100001
    padHL 15

    call transferSgbPackets
    ret

;;;
; Command Codes: 
; $17 - MASK_EN
;
; Blanks the display so VRAM can be used to transfer.
; @param C Mask mode - NONE/FROZEN/BLACK/COLOURED  
MASK_EN:
    ld HL, sgbTransferPacket
    ldHLi [HL], %10111001
    ldHLi [HL], C
    padHL 14

    call transferSgbPackets    
    ret

    ENDC