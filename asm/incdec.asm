; INC A

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

INC B
ld B, 0
scf ; carry flag not affected by inc
inc B
; AF=00xx BC=01fe DE=fefe HL=ceeb ZNHC=0001

ld B, $0f
inc B
; AF=00xx BC=10fe DE=fefe HL=ceeb ZNHC=0011

; Doesn't set the carry, but wraps around??
ld B, $ff
cp B ; Set the N flag to check it resets
inc B
; AF=00xx BC=00fe DE=fefe HL=ceeb ZNHC=1010

; INC C
ld C, 0
scf ; carry flag not affected by inc
inc C
; AF=00xx BC=0010 DE=fefe HL=ceeb ZNHC=0001

ld C, $0f
inc C
; AF=00xx BC=0001 DE=fefe HL=ceeb ZNHC=0011

; Doesn't set the carry, but wraps around??
ld C, $ff
cp C ; Set the N flag to check it resets
inc C
; AF=00xx BC=0000 DE=fefe HL=ceeb ZNHC=1010

; INC D
ld D, 0
scf ; carry flag not affected by inc
inc D
; AF=00xx BC=0000 DE=01fe HL=ceeb ZNHC=0001

ld D, $0f
inc D
; AF=00xx BC=0000 DE=01fe HL=ceeb ZNHC=0011

; Doesn't set the carry, but wraps around??
ld D, $ff
cp D ; Set the N flag to check it resets
inc D
; AF=00xx BC=0000 DE=00fe HL=ceeb ZNHC=1010

; INC E
ld E, 0
scf ; carry flag not affected by inc
inc E
; AF=00xx BC=0000 DE=0001 HL=ceeb ZNHC=0001

ld E, $0f
inc E
; AF=00xx BC=0000 DE=0001 HL=ceeb ZNHC=0011

; Doesn't set the carry, but wraps around??
ld E, $ff
cp E ; Set the N flag to check it resets
inc E
; AF=00xx BC=0000 DE=0000 HL=ceeb ZNHC=1010

; INC H
ld H, 0
scf ; carry flag not affected by inc
inc H
; AF=00xx BC=0000 DE=0000 HL=01eb ZNHC=0001

ld H, $0f
inc H
; AF=00xx BC=0000 DE=0000 HL=01eb ZNHC=0011

; Doesn't set the carry, but wraps around??
ld H, $ff
cp H ; Set the N flag to check it resets
inc H
; AF=00xx BC=0000 DE=0000 HL=00eb ZNHC=1010

; INC L
ld L, 0
scf ; carry flag not affected by inc
inc L
; AF=00xx BC=0000 DE=0000 HL=0001 ZNHC=0001

ld L, $0f
inc L
; AF=00xx BC=0000 DE=0000 HL=0001 ZNHC=0011

; Doesn't set the carry, but wraps around??
ld L, $ff
cp L ; Set the N flag to check it resets
inc L
; AF=00xx BC=0000 DE=0000 HL=0000 ZNHC=1010

; INC [HL]
ld H, c4
ld L, cd

ld [HL], 0
scf ; carry flag not affected by inc
inc [HL]
ld A, [HL]
; AF=01xx BC=0000 DE=0000 HL=0001 ZNHC=0001

ld [HL], $0f
inc [HL]
ld A, [HL]
; AF=01xx BC=0000 DE=0000 HL=0001 ZNHC=0011

; Doesn't set the carry, but wraps around??
ld [HL], $ff
cp [HL] ; Set the N flag to check it resets
inc [HL]
ld A, [HL]
; AF=00xx BC=0000 DE=0000 HL=0000 ZNHC=1010
