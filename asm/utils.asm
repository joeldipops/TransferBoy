jp utilsEnd

;;;
; Resets all four flags to 0
;;;
resetFlags:
    ; Compare 1 and 2 to reset the Z flag.
    push BC
    ld B, A
    ld A, 1
    cp 2
    ld A, B
    pop BC

    ; In order to ensure the carry flag is reset, we need to set it and the complement it.
    ; This operation also reset N and H
    scf
    ccf

    ret

;;;
; terminates the program if there's an error
; @param D should equal E
; @param E should equal D
;;;
testForError:
    push AF
    ld A, D                    ; if (D != E)
    cp E                       ;
    jp NZ, terminate           ;     terminate()
    pop AF                     ; else
    ret                        ;     return

;;;
; locks up the program (permanently I think)
;;;
terminate:
    ; halt waits for an interrupt, but interrupts are disabled muahahaaa
    di
    nop
    halt

utilsEnd:
