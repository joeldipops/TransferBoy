    IF (!DEF(INCLUDED_LCD))
INCLUDED_LCD SET 1

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

.loop
    ; Loop until display is past the bottom of the screen
    cp [HL]
    jr NC, .loop

    resAny LCD_ON_BIT, [LcdControl]
    pop HL
    ret

;;;
; Prints a null terminated string directly to the screen.
; @param HL string address
; @param D tile x coordinate
; @param E tile y coordinate
;;;
printString:
    push BC
    push HL
    ld16 B,C, H,L
.loop
        ldi A, [HL]
        or A
    jr NZ, .loop
    sub16 H,L, B,C
    ; 16 bit subtraction.
    ld16 B,C, H,L
    ; Don't include the null
    dec BC
    pop HL
    call copyToVRAM
    pop BC
    ret    
    

;;;
; Copy data to VRAM
; @param HL Source start address.
; @param D x coordinate of start address
; @param E y co-ordinate of start address.
; @param BC Length of data
; @param A 0 if using Map1, 1 if using Map2
;;;
copyToVRAM:
    push HL
    push AF

    ; Calculate the destination address based on co-ordinates
    mult E, SCREEN_BYTE_WIDTH   ; y * width
    ; ^ since we're multiplying by 32 here, there's probably value in using shifts to calculate this
    ld E, D
    ld D, 0
    add HL, DE                  ; += x

    ; Use either map depending on what's in A
    pop AF
    or A
    jr NZ, .elseMap
        ld DE, BackgroundMap1
        jr .endMap
.elseMap
        ld DE, BackgroundMap2
.endMap
    add HL, DE

    ; Destination address now in DE
    ld16 D,E, H,L
    pop HL

; Copy each byte across but only during H or V blank
.untilFinished  
        di
.untilVRAM
        andAny [LcdStatus], VRAM_BUSY
            jr NZ, .untilVRAM
        ldiAny [DE], [HL]
        inc DE
        dec	BC
        ei
	    orAny B, C
	jr NZ, .untilFinished
    ret

;;;
; Copy data to VRAM, but only when available
; @param A value to set
; @param DE destination start address
; @param BC length of data.
;;; 
setVRAM:
    push HL
    ld L, A
.untilFinished  
        di
.untilVRAM
        andAny [LcdStatus], VRAM_BUSY
        jr NZ, .untilVRAM
        ldAny [DE], L
        inc DE
        dec	BC
        ei
	    orAny B, C
	jr NZ, .untilFinished
    pop HL
	ret

    ENDC