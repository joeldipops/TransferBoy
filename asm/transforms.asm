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
    ld A
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
    ; AF=$6exx BC=7afe DE=fffe HL=fffe SP=cffe ZNHC=0000    

; swap C
    setSomeFlags
    ld C, 0
    swap C
    ; AF=$6exx BC=7a00=fffe HL=fffe SP=cffe ZNHC=1000
    
    ld C, $95
    ; AF=$6exx BC=7a59 DE=fffe HL=fffe SP=cffe ZNHC=0000        
    
; swap D
    
    
    
    
    
