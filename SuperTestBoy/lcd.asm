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
; Sets a tile with a given x/y position to a certain sprite.
; @param B The tile index.
; @param D The x position.
; @param E THe y position.
;;;
drawTile:
    push HL
    ; calculate the offset from start of the map
    mult E, SCREEN_BYTE_WIDTH
    add (BackgroundMap1 & $00ff)
    add D
    ; Then load in to memory
    ld H, (BackgroundMap1 >> 8)     
    ld L, A
    ld [HL], B
    pop HL
    ret

;;;
; Prints a null terminated string directly to the screen.
; @param HL string address
; @param DE tile start address
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
; @param DE Destination start address.
; @param BC Length of data
;;;
copyToVRAM:
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