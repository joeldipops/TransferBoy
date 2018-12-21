    IF !DEF(ADDRESSES_INCLUDED)
ADDRESSES_INCLUDED SET 1

;;;;;;;;;;;;;;; ADDRESSES ;;;;;;;;;;;;;;;;;;;;;
InterruptFlags EQU $ff0f

DMASourceRegister EQU $ff46
runDMA EQU $ff80

VBlankFlag EQU $01

JoypadRegister EQU $ff00
GetButtonsBit EQU %0010000
GetDPadBit EQU %00010000
ClearJoypad EQU %00110000
    ENDC
