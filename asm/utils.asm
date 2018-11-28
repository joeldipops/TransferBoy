jp .utilsEnd

; Resets all four flags to 0
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
   
.utilsEnd
