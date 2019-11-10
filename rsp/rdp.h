#include "rsp.h"

###
# Sends an RDP command by writing to the RDP registers.
# Assumes command starts at DMEM address 0
# @input words The number of words to send
###
.macro execRdp words
    1:
        mfc0 $at, $RDP_CMD_STATUS
        # bit 8 of $RDP_CMD_STATUS = RDP DMA is busy
        andi $at, $at, RDP_DMA_BUSY
    bne $at, $0, 1b

    # send 2 to RDP_CMD_STATUS
    # in order to DMA from RSP memory (DMEM)
    addi $at, $0, DMEM_RDP_DMA
    mtc0 $at, $RDP_CMD_STATUS

    # Kick off the DMA by setting both start and end.
    mtc0 $0, $RDP_CMD_START

    ## + 4 because end expects the next address after data ends.
    addi $at, $0, (\words * 4) + 4
    mtc0 $at, $RDP_CMD_END
.endm

###
# Executes the setFillColour RDP command
# Command 0x37
# @input upper The colour to set.  Upper 16b for 32b colours.
# @input? lower Lower 16b for 32b colours. 
###
.macro setFillColour upper, lower
    # Reset accumulator.
    #add $at, $0, $0
    # "0x37 Set Fill Color"
    lui $at, 0xF700
    sw $at, 0x000($0)

    ## Second Byte - Packed Color
    lui $at, \upper

    .ifb lower
        ori $at, \lower
    .else
        ori $at, \upper
    .endif
    sw $at, 0x004($0)

    execRdp 2
.endm

###
#
# Command 0x24 Texture Rectangle
# @input tileIndex
# @input x1 x co-ord of top left
# @input y1 y co-ord of top left
# @input x2 x co-ord of bottom right
# @input y2 y co-ord of bottom right
# @input s s co-ord of texture top left
# @input t t co-ord of texture top left
# @input ds change in s / x
# @input dt change in t / y
###
.macro textureRectangle tileIndex x1 y1 x2 y2 s t ds dt
    lui $at, 0xE400

    addi $3, $0, \x1
    sll $3, $3, 12
    ori $3, $3, \y1
    or $at, $at, $3

    sw $at, 0x000($0)

    lui $at, \tileIndex
    sll $at, $at, 8

    addi $3, $0, \x2
    sll $3, $3, 12
    ori $3, $3, \y2
    or $at, $at, $3

    sw $at, 0x004($0)

    lui $at, \s
    ori $at, $at, \t

    sw $at, 0x008($0)

    lui $at, \ds
    ori $at, $at, \dt

    sw $at, 0x00C($0)
    execRdp 4
.endm

###
#
# Command 0x34 Load Tile
# @input tileIndex
# @input SL low S co-ord
# @input TL low T co-ord
# @input SH high S co-ord
# @input TH high T co-ord
###
.macro loadTile tile, sl, tl, sh, th
    # Command 0x34
    lui $at, 0xF400

    addi $3, $0, \sl
    sll $3, $3, 12
    or $at, $at, $3
    ori $at, $at, \tl

    sw $at, 0x000($0)

    lui $at, \tile
    sll $at, $at, 8
    addi $3, $0, \sh
    sll $3, $3, 12
    or $at, $at, $3
    ori $at, $at, \th

    sw $at, 0x004($0)

    execRdp 2
.endm

#
# Command 0x35 Set Tile
# @input format
# @input depth colour depth
# @input size size of tile line in 64b words
# @input tmem address in tmem (in 64b words so 2 = address 0x008)
# @input palette
# @input {boolean} clampT on/off
# @input {boolean} mirrorT on/off
# etc
###
.macro setTile \
    format depth size tmem \
    tile palette clampT mirrorT maskT \
    shiftT clampS mirrorS maskS shiftS

    # Command 0x35
    lui $at, 0xF500

    lui $3, \format
    lui $4, \depth
    sll $3, $3, 5
    sll $4, $4, 3
    or $3, $3, $4
    or $at, $at, $3

    addi $3, $0, \size
    sll $3, $3, 9
    ori $3, $3, \tmem

    or $at, $at, $3

    sw $at, 0x000($0)

    .ifb tile
        lui $at, \tile
        sll $at, $at, 8
    .else
        add $at, $0, $0
    .endif

    .ifb paletted
        lui $3, \palette
        sll $3, $3, 4
        or $at, $at, $3
    .endif

    .ifb clampT
        lui $3, \clampT
        sll $3, $3, 3
        or $at, $at, $3
    .endif

    .ifb mirrorT
        lui $3, \mirrorT
        sll $3, $3, 2
        or $at, $at, $3
    .endif

    .ifb maskT
        addi $3, $0, \maskT
        sll $3, $3, 14
        or $at, $at, $3
    .endif
    
    .ifb shiftT
        addi $3, $0, \shiftT
        sll $3, $3, 10
        or $at, $at, $3
    .endif

    .ifb clampS
        addi $3, $0, \clampS
        sll $3, $3, 9
        or $at, $at, $3
    .endif

    .ifb mirrorS
        addi $3, \mirrorS
        sll $3, $3, 8
        or $at, $at, $3
    .endif

    .ifb maskS
        addi $3, $0, \maskS
        sll $3, $3, 4
        or $at, $at, $3
    .endif

    .ifb shiftS
        ori $at, $at, \shiftS
    .endif

    sw $at, 0x004($0)

    execRdp 2

.endm

###
#
# Command 0x3D
# @input format
# @input depth colour depth
# @input width
# @input address
# @sideAffect registers $2, $3, $4 affected.
# @sideAffect DMEM 0x000 - 0x008 affected.
###
.macro setTextureImage format depth width address
    # Command 0x3D
    lui $at, 0xFD00

    lui $3, \format
    lui $4, \depth
    sll $3, $3, 5
    sll $4, $4, 3
    or $3, $3, $4
    or $at, $at, $3

    ori $at, $at, (\width + 1)
    sw $at, 0x000($0)

    ##if it's a register
        sw \address, 0x004($0)
    ##else it's a constant...
        #lui $A, .hi(0x10000001)
        #ori $A, .lo(0x10000001)
    ##endif

    execRdp 2
.endm