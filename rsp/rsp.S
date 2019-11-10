#include <ucode.S>
#include "rdp.h"

.set noat

###
# Sets the RDP to draw red rectangles.
main:
    loadTile 1, 0x10, 0x10, 0x20, 0x20
    textureRectangle 1, 0x04, 0x04, 0x04, 0x04, 0x00, 0x00, 0x00, 0x00
    #setTile 0, 2, 0x0001, 0x0001, 1
    #setFillColour 0xF801
    #lui $5, 0x1234
    #ori $5, $5, 0x5678
    #setTextureImage 0, 2, 100, $5
break









