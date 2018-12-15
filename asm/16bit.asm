INCLUDE "utils.asm"
    ; AF=0fxx BC=0f0f DE=0f0f HL=c702 ZNHC=0110

    ;;;;;;;;;;;;;;; ADD HL ;;;;;;;;;;;;;;;;;;;;;;;;;;

    ; ADD HL, BC

    ld H, 1
    ld L, 2
    ld B, 3
    ld C, 4
    add HL, BC ; 0102 + 0304 = 258 + 772
    ; AF=0f00 BC=0304 DE=0f0f HL=0406 ZNHC=0000

    ; test Z flag not affected
    xor A
    add HL, BC
    ; AF=0fxx BC=0304 DE=0f0f HL=070A ZNHC=1000

    ld H, 0
    ld L, 0
    ld B, 0
    ld C, 0
    or A
    add HL, BC
    ; AF=0f00 BC=0000 DE=0f0f HL=0000 ZNHC=0000

    ; lower carry and half carries don't set flags.
    ld L, $0f
    ld C, $0f
    add HL, BC
    ; AF=0f00 BC=000f DE=0f0f HL=001E ZNHC=0000

    ld L, $f0
    ld C, $f0
    add HL, BC
    ; AF=0f00 BC=00f0 DE=0f0f HL=01E0 ZNHC=0000

    ; upper carries do
    ld H, $0f
    ld L, 0
    ld B, $0f
    ld C, 0
    add HL, BC
    ; AF=0fxx BC=0f00 DE=0f0f HL=1E00 ZNHC=0010

    ld H, $f0
    ld B, $f0
    add HL, BC
    ; AF=0fxx BC=f000 DE=0f0f HL=E000 ZNHC=0001

    ld H, $ff
    ld B, $ff
    add HL, BC
    ; AF=0fxx BC=ff00 DE=0f0f HL=FE00 ZNHC=0011

    ld H, $f1
    ld L, $e2
    ld B, $c7
    ld C, $99
    add HL, BC
    ; AF=0fxx BC=c799 DE=0f0f HL=b97b ZNHC=0001

    ; ADD HL, DE
    ld H, 1
    ld L, 2
    ld D, 3
    ld E, 4
    add HL, DE
    ; AF=0f00 BC=c799 DE=0304 HL=0406 ZNHC=0000

    ; test Z flag not affected
    xor A
    add HL, DE
    ; AF=0fxx BC=c799 DE=0304 HL=070A ZNHC=1000

    ld H, 0
    ld L, 0
    ld D, 0
    ld E, 0
    or A
    add HL, DE
    ; AF=0f00 BC=c799 DE=0000 HL=0000 ZNHC=0000

    ; lower carry and half carries don't set flags.
    ld L, $0f
    ld E, $0f
    add HL, DE
    ; AF=0f00 BC=c799 DE=000f HL=001E ZNHC=0000

    ld L, $f0
    ld E, $f0
    add HL, DE
    ; AF=0f00 BC=c799 DE=00f0 HL=01E0 ZNHC=0000

    ; upper carries do
    ld H, $0f
    ld L, 0
    ld D, $0f
    ld E, 0
    add HL, DE
    ; AF=0fxx BC=c799 DE=0f00 HL=1E00 ZNHC=0010

    ld H, $f0
    ld D, $f0
    add HL, DE
    ; AF=0fxx BC=c799 DE=f000 HL=E000 ZNHC=0001

    ld H, $ff
    ld D, $ff
    add HL, DE
    ; AF=0fxx BC=c799 DE=ff00 HL=FE00 ZNHC=0011

    ld H, $f1
    ld L, $e2
    ld D, $c7
    ld E, $99
    add HL, DE
    ; AF=0fxx BC=c799 DE=c799 HL=b97b ZNHC=0001

    ; ADD HL, HL

    ld H, 01
    ld L, 02
    add HL, HL
    ; AF=0fxx BC=c799 DE=c799 HL=0204 ZNHC=0000

    ; test Z flag not affected
    xor A
    add HL, HL
    ; AF=0fxx BC=c799 DE=c799 HL=0408 ZNHC=1000

    ld H, 0
    ld L, 0
    or A
    add HL, HL
    ; AF=0f00 BC=c799 DE=c799 HL=0000 ZNHC=0000

    ; lower carry and half carries don't set flags.
    ld L, $0f
    add HL, HL
    ; AF=0f00 BC=c799 DE=c799 HL=001e ZNHC=0000

    ld L, $f0
    add HL, HL
    ; AF=0f00 BC=c799 DE=c799 HL=01e0 ZNHC=0000

    ; upper carries do
    ld H, $0f
    ld L, 0
    add HL, HL
    ; AF=0fxx BC=c799 DE=c799 HL=1e00 ZNHC=0010

    ld H, $f0
    add HL, HL
    ; AF=0fxx BC=c799 DE=c799 HL=e000 ZNHC=0001

    ld H, $ff
    add HL, HL
    ; AF=0fxx BC=c799 DE=c799 HL=fe00 ZNHC=0011

    ; add HL, SP

    ld H, 1
    ld L, 2
    ld SP, $0304
    add HL, SP
    ; AF=0f00 BC=c799 DE=c799 HL=0406 ZNHC=0000

    ; test Z flag not affected
    xor A
    add HL, SP
    ; AF=0fxx BC=c799 DE=c799 HL=070A ZNHC=1000

    ld H, 0
    ld L, 0
    ld SP, 0
    or A
    add HL, SP
    ; AF=0f00 BC=c799 DE=c799 HL=0000 ZNHC=0000

    ; lower carry and half carries don't set flags.
    ld L, $0f
    ld SP, $000f
    add HL, SP
    ; AF=0f00 BC=c799 DE=c799 HL=001E ZNHC=0000

    ld L, $f0
    ld SP, $00f0
    add HL, SP
    ; AF=0f00 BC=c799 DE=c799 HL=01E0 ZNHC=0000

    ; upper carries do
    ld H, $0f
    ld L, 0
    ld SP, $0f00
    add HL, SP
    ; AF=0fxx BC=c799 DE=c799 HL=1E00 ZNHC=0010

    ld H, $f0
    ld SP, $f000
    add HL, SP
    ; AF=0fxx BC=c799 DE=c799 HL=E000 ZNHC=0001

    ld H, $ff
    ld SP, $ff00
    add HL, SP
    ; AF=0fxx BC=c799 DE=c799 HL=FE00 ZNHC=0011

    ld H, $f1
    ld L, $e2
    ld SP, $c799
    add HL, SP
    ; AF=0fxx BC=c799 DE=c799 HL=b97b ZNHC=0001

    ; add SP, n

    ;; According to rgbasm, "add SP, n" sets carries on bit 7 and 3 overflows...

    ld SP, $d000
    cp A ; sets Z, N
    add SP, 1
    ; AF=0fxx BC=c799 DE=c799 HL=b97b SP=$d001 ZNHC=0000

    add SP, $ff
    ; AF=0fxx BC=c799 DE=c799 HL=b97b SP=$d100 ZNHC=0001

    ld SP, $d0ff
    add SP, $0f
    ; AF=0fxx BC=c799 DE=c799 HL=b97b SP=$d10e ZNHC=0010

    ld SP, $d0ff
    add SP, $ff
    ; AF=0fxx BC=c799 DE=c799 HL=b97b SP=$d1fe ZNHC=0011

    ; set half carry? - according to rgbasm, no
    ld SP, $d800
    add SP, $ff
    ; AF=0fxx BC=c799 DE=c799 HL=b97b SP=$d100 ZNHC=0000

    ;DOn't think you can even set SP here, but let's see what happens
    ld SP, $fffe
    add SP, $ff
    ; ZNHC=0011? non?

    ;;;;;;;;;;;;; INC rr ;;;;;;;;;;;

    ; inc BC
    ld BC, $4312
    cp A
    ; set some flags
    inc BC 
    ; flags unnaffected.
    ; AF=0fxx BC=4213 DE=c799 HL=b97b ZNHC=1100

    ld BC, $00ff
    inc BC
    ; AF=0fxx BC=0100 DE=c799 HL=b97b ZNHC=1100

    resetFlags
    ld BC, $ffff
    inc BC
    ; AF=0fxx BC=0000 DE=c799 HL=b97b ZNHC=0000

    ; inc DE
    ld DE, $4312
    cp A
    ; set some flags
    inc DE 
    ; flags unnaffected.
    ; AF=0fxx BC=0000 DE=4313 HL=b97b ZNHC=1100

    ld DE, $00ff
    inc DE
    ; AF=0fxx BC=0000 DE=0100 HL=b97b ZNHC=1100

    resetFlags
    ld DE, $ffff
    inc DE
    ; AF=0fxx BC=0000 DE=0000 HL=b97b ZNHC=0000

    ; inc HL
    ld HL, $4312
    cp A
    ; set some flags
    inc HL 
    ; flags unnaffected.
    ; AF=0fxx BC=0000 DE=0000 HL=4313 ZNHC=1100

    ld HL, $00ff
    inc HL
    ; AF=0fxx BC=0000 DE=0000 HL=0100 ZNHC=1100

    resetFlags
    ld HL, $ffff
    inc HL
    ; AF=0fxx BC=0000 DE=0000 HL=0000 ZNHC=0000

    ; inc HL
    ld HL, $4312
    cp A
    ; set some flags
    inc HL 
    ; flags unnaffected.
    ; AF=0fxx BC=0000 DE=0000 HL=4313 ZNHC=1100

    ld HL, $00ff
    inc HL
    ; AF=0fxx BC=0000 DE=0000 HL=0100 ZNHC=1100

    resetFlags
    ld HL, $ffff
    inc HL
    ; AF=0fxx BC=0000 DE=0000 HL=0000 ZNHC=0000

    ; inc HL
    ld SP, $d312
    cp A
    ; set some flags
    inc SP 
    ; flags unnaffected.
    ; AF=0fxx BC=0000 DE=0000 HL=0000 SP=d312 ZNHC=1100

    ld SP, $d0ff
    inc SP
    ; AF=0fxx BC=0000 DE=0000 HL=0000 SP=d100 ZNHC=1100

    ; does it work?
    resetFlags
    ld SP, $ffff
    inc SP
    ; AF=0fxx BC=0000 DE=0000 HL=0000 SP=0000 ZNHC=0000

;;;;;;;;;;;;;;;;;; DEC rr ;;;;;;;;;;;;;;;;;;;
    dec BC
    ; AF=0fxx BC=ffff DE=0000 HL=0000 SP=0000 ZNHC=0000

    cp A
    dec BC
    ; AF=0fxx BC=fffe DE=0000 HL=0000 SP=0000 ZNHC=1100

    resetFlags
    dec DE
    ; AF=0fxx BC=fffe DE=ffff HL=0000 SP=0000 ZNHC=0000

    cp A
    dec DE
    ; AF=0fxx BC=fffe DE=fffe HL=0000 SP=0000 ZNHC=1100

    resetFlags
    dec HL
    ; AF=0fxx BC=fffe DE=fffe HL=ffff SP=0000 ZNHC=0000

    cp A
    dec HL
    ; AF=0fxx BC=fffe DE=fffe HL=fffe SP=0000 ZNHC=1100    

    ld SP, $d000
    dec SP
    ; AF=0fxx BC=fffe DE=fffe HL=ffff SP=cfff ZNHC=1100

    cp A
    dec SP
    ; AF=0fxx BC=fffe DE=fffe HL=fffe SP=cffe ZNHC=1100      