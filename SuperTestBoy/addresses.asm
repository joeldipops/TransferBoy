    IF !DEF(ADDRESSES_INCLUDED)
ADDRESSES_INCLUDED SET 1

;;;;;;;;;;;;;;; ADDRESSES ;;;;;;;;;;;;;;;;;;;;;
InterruptFlags EQU $ff0f
VBlankFlag EQU $01

JoypadRegister EQU $ff00
GetButtonsBit EQU %0010000
GetDPadBit EQU %00010000
    ENDC