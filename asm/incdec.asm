ld A, 0
scf ; carry flag not affected by inc
inc A
; AF=01xx BC=fefe DE=fefe HL=ceeb ZNHC=0001

ld A, $0f
inc A
; AF=10xx BC=fefe DE=fefe HL=ceeb ZNHC=0011

; Doesn't set the carry, but wraps around??
ld A, $ff
cp A ; Set the N flag to check it resets
inc A
; AF=00xx BC=fefe DE=fefe HL=ceeb ZNHC=1010
