    ; test all the flags with ld A
    ld A, 0
    add A
    ; A=0 Z=1 N=0 H=0 C=0

    ld A, 1
    add A
    ; A=2 Z=0 N=0 H=0 C=0
    ld A, 7
    add A
    ; A=$0E Z=0 N=0 H=1 C=0
    ld A, 240
    add A
    ; A=$E0 Z=0, N=0, H=0, C=1

    ld A, 255
    add A
    ; A=$FE Z=0, N=0, H=1, C=1

    ; Let's test each add op code for all flags.
    ld A, 0
    ld B, 0
    add B
    ; A=0 B=0 Z=1 N=0 H=0 C=0
    ld B, 7
    add B
    ; A=7 B=7 Z=0 N=0 H=0 C=0

    ld A, 1
    add B
    ; A=8 B=7 Z=0 N=0 H=1 C=0

    ld A, 240
    ld B, 128
    add B
    ; A=$70 B=$80 Z=0 N=0 H=0 C=1

    ld A, 1
    ld B, 255
    add B
    ; A=0 B=$ff Z=1 N=0 H=1 C=1

    ld A, 255
    add B
    ; A=$fe B=$ff Z=0 N=0 H=1 C=1

    ; ADD Cs
    ld A, 0
    ld C, 0
    add C
    ; A=0 C=0 Z=1 N=0 H=0 C=0
    ld C, 7
    add C
    ; A=7 C=7 Z=0 N=0 H=0 C=0

    ld A, 1
    add C
    ; A=8 C=7 Z=0 N=0 H=1 C=0

    ld A, 240
    ld C, 128
    add C
    ; A=$70 C=$80 Z=0 N=0 H=0 C=1

    ld A, 1
    ld C, 255
    add C
    ; A=0 C=$ff Z=1 N=0 H=1 C=1

    ld A, 255
    add C
    ; A=$fe C=$ff Z=0 N=0 H=1 C=1

    ; D
    ld A, 0
    ld D, 0
    add D
    ; A=0 D=0 Z=1 N=0 H=0 C=0
    ld D, 7
    add D
    ; A=7 D=7 Z=0 N=0 H=0 C=0

    ld A, 1
    add D
    ; A=8 D=7 Z=0 N=0 H=1 C=0

    ld A, 240
    ld D, 128
    add D
    ; A=$70 D=$80 Z=0 N=0 H=0 C=1

    ld A, 1
    ld D, 255
    add D
    ; A=0 D=$ff Z=1 N=0 H=1 C=1

    ld A, 255
    add D
    ; A=$fe D=$ff Z=0 N=0 H=1 C=1

    ; E
    ld A, 0
    ld E, 0
    add E
    ; A=0 E=0 Z=1 N=0 H=0 C=0
    ld E, 7
    add E
    ; A=7 E=7 Z=0 N=0 H=0 C=0

    ld A, 1
    add E
    ; A=8 E=7 Z=0 N=0 H=1 C=0

    ld A, 240
    ld E, 128
    add E
    ; A=$70 E=$80 Z=0 N=0 H=0 C=1

    ld A, 1
    ld E, 255
    add E
    ; A=0 E=$ff Z=1 N=0 H=1 C=1

    ld A, 255
    add E
    ; A=$fe E=$ff Z=0 N=0 H=1 C=1

; H
    ld A, 0
    ld H, 0
    add H
    ; A=0 H=0 Z=1 N=0 H=0 C=0
    ld H, 7
    add H
    ; A=7 H=7 Z=0 N=0 H=0 C=0

    ld A, 1
    add H
    ; A=8 H=7 Z=0 N=0 H=1 C=0

    ld A, 240
    ld H, 128
    add H
    ; A=$70 H=$80 Z=0 N=0 H=0 C=1

    ld A, 1
    ld H, 255
    add H
    ; A=0 H=$ff Z=1 N=0 H=1 C=1

    ld A, 255
    add H
    ; A=$fe H=$ff Z=0 N=0 H=1 C=1

; L
    ld A, 0
    ld L, 0
    add L
    ; A=0 L=0 Z=1 N=0 H=0 C=0
    ld L, 7
    add L
    ; A=7 L=7 Z=0 N=0 H=0 C=0

    ld A, 1
    add L
    ; A=8 L=7 Z=0 N=0 H=1 C=0

    ld A, 240
    ld L, 128
    add L
    ; A=$70 L=$80 Z=0 N=0 H=0 C=1

    ld A, 1
    ld L, 255
    add L
    ; A=0 L=$ff Z=1 N=0 H=1 C=1

    ld A, 255
    add L
    ; A=$fe L=$ff Z=0 N=0 H=1 C=1

    ; A=$fe B=$ff C=$ff D=$ff E=$ff H=$ff L=$ff SP=$fffe Z=0 N=0 H=1 C=1

    ; Time to read from memory
    ld H, $c0
    ld L, $10

    ld A, 0
    ld [HL], 0
    add [HL]
    ; A=0 Z=1 N=0 H=0 C=0
    ld [HL], 7
    add [HL]
    ; A=7 Z=0 N=0 H=0 C=0

    ld A, 1
    add [HL]
    ; A=8 Z=0 N=0 H=1 C=0

    ld A, 240
    ld [HL], 128
    add [HL]
    ; A=$70 Z=0 N=0 H=0 C=1

    ld A, 1
    ld [HL], 255
    add [HL]
    ; A=0 Z=1 N=0 H=1 C=1

    ld A, 255
    add [HL]
    ; A=$fe [HL]=$ff Z=0 N=0 H=1 C=1

    ; Add literals
    ld A, 0
    add 0
    ; A=0 Z=1 N=0 H=0 C=0

    add 7
    ; A=7 L=7 Z=0 N=0 H=0 C=0

    add 1
    ; A=8 L=7 Z=0 N=0 H=1 C=0

    ld A, 240
    add 128
    ; A=$70 L=$80 Z=0 N=0 H=0 C=1

    ld A, 1
    add 255
    ; A=0 L=$ff Z=1 N=0 H=1 C=1

    ld A, 255
    add 255
    ; A=$fe L=$ff Z=0 N=0 H=1 C=1
