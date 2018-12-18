INCLUDE "utils.asm"
; AF=0fxx BC=fffe DE=fffe HL=fffe SP=cffe ZNHC=1100

setSomeFlags: macro
    push BC
    ld B, A
    ld A, 0
    sub %01111111
    ld A, B
    pop BC
endm
;;;;;;;;;;;;;;;;;;;;; SWAP ;;;;;;;;;;;;;;

; swap A

    setSomeFlags
    ld A, 0
    swap A
    ; AF=$00xx BC=fffe DE=fffe HL=fffe SP=cffe ZNHC=1000
    
    ld A, $e6
    swap A
    ; AF=6e00 BC=fffe DE=fffe HL=fffe SP=cffe ZNHC=0000

; swap B    
    setSomeFlags
    ld B, 0
    swap B
    ; AF=$6exx BC=00fe DE=fffe HL=fffe SP=cffe ZNHC=1000
    
    ld B, $a7
    swap B
    ; AF=$6exx BC=7afe DE=fffe HL=fffe SP=cffe ZNHC=0000    

; swap C
    setSomeFlags
    ld C, 0
    swap C
    ; AF=$6exx BC=7a00 DE=fffe HL=fffe SP=cffe ZNHC=1000
    
    ld C, $95
    swap C
    ; AF=$6exx BC=7a59 DE=fffe HL=fffe SP=cffe ZNHC=0000        
    
; swap D
    setSomeFlags
    ld D, 0
    swap D
    ; AF=$6exx BC=7a59 DE=00fe HL=fffe SP=cffe ZNHC=1000
    
    ld D, $69
    swap D
    ; AF=$6e00 BC=7a59 DE=96fe HL=fffe SP=cffe ZNHC=0000
    
; swap E
    setSomeFlags
    ld E, 0
    swap E
    ; AF=$6exx BC=7a59 DE=0000 HL=fffe SP=cffe ZNHC=1000
    
    ld E, $cd
    swap E
    ; AF=$6e00 BC=7a59 DE=96dc HL=fffe SP=cffe ZNHC=0000   

; swap H
    setSomeFlags
    ld H, 0
    swap H
    ; AF=$6exx BC=7a59 DE=0000 HL=00fe SP=cffe ZNHC=1000
    
    ld H, $2c
    swap H
    ; AF=$6e00 BC=7a59 DE=96dc HL=c2fe SP=cffe ZNHC=0000

; swap L
    setSomeFlags
    ld L, 0
    swap L
    ; AF=$6exx BC=7a59 DE=0000 HL=c200 SP=cffe ZNHC=1000
    
    ld L, $12
    swap L
    ; AF=$6e00 BC=7a59 DE=96dc HL=c221 SP=cffe ZNHC=0000
    
; swap [HL]
    setSomeFlags
    ld [HL], 0
    swap [HL]
    ld A, [HL]
    ; AF=$00xx BC=7a59 DE=0000 HL=c221 SP=cffe ZNHC=1000    
    
    ld [HL], 87
    swap [HL]
    ld A, [HL]
    ; AF=$78xx BC=7a59 DE=0000 HL=c221 SP=cffe ZNHC=0000   
   
;;;;;;;;;;;;;;;;; DAA ;;;;;;;;;;;;;;;;;;;;;;;;;;    

    daa
    ; AF=$78xx BC=7a59 DE=0000 HL=c221 SP=cffe ZNHC=0000   
    
    ld A, $7c
    daa
    ; AF=$82xx BC=7a59 DE=0000 HL=c221 SP=cffe ZNHC=0000   
    
    ld A, 100
    daa
    ; AF=$00xx BC=7a59 DE=0000 HL=c221 SP=cffe ZNHC=1001
    
    ld A, 00
    daa
    ; AF=$00xx BC=7a59 DE=0000 HL=c221 SP=cffe ZNHC=1000
    
    ld A, $12
    sub $05
    daa
    ; AF=07ff BC=7a59 DE=0000 HL=c221 SP=cffe ZNHC=1100
    
    ; I'm gonna just test this and see what happens, I just don't know how this is supposed to work
    sub $08
    daa
    ; Test with half carry and/or carry set before calling daa
    
;;;;;;;;;;;;;;;; CPL ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

    ld A, 1
    setSomeFlags
    cpl
    ; AF=fexx  BC=7a59 DE=0000 HL=c221 SP=cffe ZNHC=1110
    
    sub 1
    cpl
    ; AF=02exx  BC=7a59 DE=0000 HL=c221 SP=cffe ZNHC=0111
    
    ; Z flag not affected
    ld A, 0
    cpl
    ; AF=00xx  BC=7a59 DE=0000 HL=c221 SP=cffe ZNHC=0111
    
;;;;;;;;;;;;;;; CCF ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ld A, $28
    ccf
    ; AF=28xx  BC=7a59 DE=0000 HL=c221 SP=cffe ZNHC=0000
    
    xor A
    ccf
    ; AF=00xx  BC=7a59 DE=0000 HL=c221 SP=cffe ZNHC=1001
    
;;;;;;;;;;;;;;; SCF ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

    
    
