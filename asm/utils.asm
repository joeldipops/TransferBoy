IF !DEF(UTILS_INCLUDED)
UTILS_INCLUDED SET 1

jp utilsEnd

;;;
; Sets all CPU registers, including flags, to 0
;;;
initialise:
    ld A, 0
    ld B, 0
    ld C, 0
    ld D, 0
    ld E, 0
    ld H, 0
    ld L, 0
    call resetFlags
    ret


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
; Divides two numbers (integer division)
; @param A the numerator
; @param B the denominator
; @return A the whole number ratio
;;;
divide:
    push BC
    
    ld C, A                   ; if (B == 0)
    ld A, B
    or A
    jp Z, .divideByZero       ;     return 0
    
    ld A, C                   ; result = -1
    ld C, -1
.do                           ; do
    sub B                     ;     A -= B
    inc C                     ;     result++    
    cp 0                      ; until (
    jp Z, .until              ;     A == 0
    jp C, .until              ;     || A < 0
    jp .do                    ; )
.until
                
    ld A, C                   ; return result
    pop BC
    ret

.divideByZero
    ld A, 0
    pop BC
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
