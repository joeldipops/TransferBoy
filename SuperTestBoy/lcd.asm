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
    push AF
    push BC
    push HL
    ld16RR B,C, H,L
.loop
        ldi A, [HL]
        or A
    jr NZ, .loop
    sub16 H,L, B,C
    ; 16 bit subtraction.
    ld16RR B,C, H,L
    ; Don't include the null
    dec BC
    pop HL
    call copyToVRAM
    pop BC
    pop AF
    ret    
    
;;;
; Takes an x and y co-ordinate and finds the corresponding memory address
; @param D the X co-ord
; @param E the Y co-ord
; @param A 0 if using Map1, 1 if using Map2
; @returns result in DE
coordsToAddress:
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
    jr NZ, .notMap1
        ld DE, BackgroundMap1
        jr .endMap
.notMap1
        ld DE, BackgroundMap2
.endMap
    add HL, DE

    ; Destination address now in DE
    ld16RR D,E, H,L
    pop HL
    ret

;;;
; Resets background to all light tiles.
;;
resetBackground:
    ld A, 0
    ld D, 0
    ld E, 0
    ld L, LIGHTEST
    ld BC, SCREEN_BYTE_WIDTH * SCREEN_BYTE_HEIGHT
    call setVRAM    
    ret

;;;
; Moves every sprite off-screen.
;;;
resetForeground:
    ld BC, SPRITE_COUNT 
    ld HL, PcY

.loop
        ld [HL], 0
        REPT SPRITE_SIZE
            inc HL
        ENDR
        dec BC
        orAny B, C
    jr NZ, .loop    
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
    call coordsToAddress
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
; Copy data to VRAM
; @param L Character to set.
; @param D x coordinate of start address
; @param E y co-ordinate of start address.
; @param BC Length of data
; @param A 0 if using Map1, 1 if using Map2
;;;
setVRAM:
    push HL
    call coordsToAddress
    pop HL
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
	ret

    ENDC