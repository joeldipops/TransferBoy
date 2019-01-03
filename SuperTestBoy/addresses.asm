    IF !DEF(ADDRESSES_INCLUDED)
ADDRESSES_INCLUDED SET 1

;;;;;;;;;;;;;;; Hardware Addresses ;;;;;;;;;;;;;;;;;;;;;
TileData EQU $8000
BackgroundMap1 EQU $9800
BackgroundMap2 EQU $9c00

JoypadIo EQU $ff00
InterruptFlags EQU $ff0f

AudioSpeakerControl EQU $ff24
AudioSpeakerChannels EQU $ff25
AudioState EQU $ff26

LcdControl EQU $ff40
LcdStatus EQU $ff41

BackgroundScrollY EQU $ff42
BackgroundScrollX EQU $ff43
CurrentLine EQU $ff44

DmaSourceRegister EQU $ff46
BackgroundPalette EQU $ff47
SpritePalette0 EQU $ff48
SpritePalette1 EQU $ff49

;;;;;;;;;;;;;; Software Addresses ;;;;;;;;;;;;;;;;;;;;;;
stackFloor EQU $ffff ; Might change to DFFF when actually start using the stack
runDma EQU $ff80

    ENDC
