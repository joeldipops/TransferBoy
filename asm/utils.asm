IF !DEF(UTILS_INCLUDED)
UTILS_INCLUDED SET 1

jp utilsEnd

;;;
; Pushes all registers on the stack so interrupts won't mess with them
;;;
pushAll:
    push AF
    push BC
    push DE
    push HL
    ret

;;;
; Pops all registers back off the stack when an interrupt is finished.
;;;
popAll:
    pop HL
    pop DE
    pop BC
    pop AF
    ret

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
; Multiplies two numbers
; @param A the first number
; @param B the second number
; @return A the multiplied result.
;;;
multiply:
    push BC
    push DE
    ld C, A

.do                         ; do
    add A, C                ;     result += A
    dec B                   ;     b--
    ld D, A
    ld A, B
    cp 0                    ; until (b == 0)
    ; really hope these are guaranteed not to mess with Z
    ld B, A
    ld A, D   
    jp NZ, .do              
    pop DE
    pop BC
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
ENDC