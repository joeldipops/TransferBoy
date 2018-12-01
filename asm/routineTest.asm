SECTION "start", ROM0[$0100]
    nop
    jp tests

tests:
    INCLUDE "utils.asm"

; "multiply(A, B)"
    ld A, 0
    ld B, 213
    call multiply
    cp A, 0
    jp NZ, terminate

    
    ld A, 1
    ld B, 1
    call multiply
    cp A, 1
    jp NZ, terminate

    ld A, 2
    ld B, 2
    call multiply
    cp A, 4
    jp NZ, terminate

    ld A, 23
    ld B, 3
    call multiply
    cp A, 69
    jp NZ, terminate

; "divide(A, B)"
    ld A, 12
    ld B, 3
    call divide
    cp A, 4
    jp NZ, terminate

    ld A, 15
    ld B, 15
    call divide
    cp A, 1
    jp NZ, terminate

    ld A, 12
    ld b, 5
    call divide
    cp A, 2
    jp NZ, terminate

    nop
    stop