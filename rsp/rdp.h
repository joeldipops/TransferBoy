#include "rsp.inc"

# TODO: execRdp needs to be changed from a macro to a routine to save on space.

.eqv RGBA_FORMAT, 0
.eqv COLOUR_DEPTH_16, 2

.eqv CLAMP_DISABLAED, 0
.eqv MIRROR_DISABLED, 0
.eqv MASK_T, 0
.eqv MASK_S, 0
.eqv SHIFT_T, 0
.eqv SHIFT_S, 0

###
# Sends an RDP command by writing to the RDP registers.
# Assumes command starts at DMEM address 0
# @param {address} $a0 address after last word to send.
###
_execRdp:
    1:
        mfc0 $t0, $RDP_CMD_STATUS
        # bit 8 of $RDP_CMD_STATUS = RDP DMA is busy
        andi $t0, $t0, RDP_DMA_BUSY
    bne $t0, $0, 1b

    # send 2 to RDP_CMD_STATUS
    # in order to DMA from RSP memory (DMEM)
    addi $t0, $0, DMEM_RDP_DMA
    mtc0 $t0, $RDP_CMD_STATUS

    # Kick off the DMA by setting both start and end.
    mtc0 $0, $RDP_CMD_START
    mtc0 $a0, $RDP_CMD_END
    jr $ra


###
# Sends an RDP command by writing to the RDP registers.
# Assumes command starts at DMEM address 0
# @input words The number of words to send
###
.macro execRdp words
    # + 4 because end expects the next address after data ends.
    addi $a0, $0, (\words * 4) + 4
    jal _execRdp
.endm

###
# Executes the syncPipe RDP command
# Command 0x27
###
.macro syncPipe
    lui $t0, 0xE700
    sw $t0, 0x000($0)
    execRdp 1
.endm

###
# Executes the syncFull RDP command
# Command 0x27
###
.macro syncFull
    lui $t0, 0xE900
    sw $t0, 0x000($0)
    execRdp 1
.endm

###
# Executes the setFillColour RDP command
# Command 0x37
# @input upper The colour to set.  Upper 16b for 32b colours.
# @input? lower Lower 16b for 32b colours. 
###
.macro setFillColour upper, lower
    # "0x37 Set Fill Color"
    lui $t0, 0xF700
    sw $t0, 0x000($0)

    ## Second Byte - Packed Color
    lui $t0, \upper

    .ifb lower
        ori $t0, \lower
    .else
        ori $t0, \upper
    .endif
    sw $t0, 0x004($0)

    execRdp 2
.endm

###
# Fills specified rectangle with the colour set in setFillColour command (0x37)
# Command 0x36
# @input x1 x co-ord of top left corner.
# @input y1 y co-ord of top left corner.
# @input x2 x co-ord of bottom right corner
# @input y2 y co-ord of bottom right corner
.macro fillRectangle x1, y1, x2, y2
    # Command 0x36 (shifted right twice)
    # (yes for some reason the numbers we set
    # are divided by 4 to get the numbers that
    # are displayed. ?)
    lui $t0, 0x3D80

    add $t1, $0, \x2
    sll $t1, $t1, 12
    or $t0, $t0, $t1
    or $t0, $t0, \y2

    sll $t0, $t0, 2

    sw $t0, 0x000($0)

    add $t0, \x1, $0
    sll $t0, $t0, 12
    or $t0, $t0, \y1
    sll $t0, $t0, 2

    sw $t0, 0x004($0)

    execRdp 2
.endm

###
#
# Command 0x24 Texture Rectangle
# @input tileIndex
# @input x1 x co-ord of bottom right
# @input y1 y co-ord of bottom right
# @input x2 x co-ord of top left
# @input y2 y co-ord of top left
# @input s s co-ord of texture top left
# @input t t co-ord of texture top left
# @input ds change in s / x
# @input dt change in t / y
###
.macro textureRectangle tileIndex x1 y1 x2 y2 s t ds dt
    lui $t0, 0xE400

    # We multiply these by four.  why I don't know at this point.
    sll \x1, \x1, 2
    sll \x2, \x2, 2
    sll \y1, \y1, 2
    sll \y2, \y2, 2

    add $t1, $0, \x2
    sll $t1, $t1, 12
    or $t1, $t1, \y2
    or $t0, $t0, $t1

    sw $t0, 0x000($0)

    lui $t0, \tileIndex
    sll $t0, $t0, 8

    add $t1, $0, \x1
    sll $t1, $t1, 12
    or $t1, $t1, \y1
    or $t0, $t0, $t1

    sw $t0, 0x004($0)

    lui $t0, \s
    ori $t0, $t0, \t

    sw $t0, 0x008($0)

    lui $t0, \ds
    ori $t0, $t0, \dt

    sw $t0, 0x00C($0)
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
    lui $t0, 0xF400

    addi $t1, $0, \sl
    # multiply by 4, then shift 12.
    sll $t1, $t1, 12 + 2
    
    or $t0, $t0, $t1

    addi $t1, $0, \tl
    # Multiply by 4 because the RDP says so.
    sll $t1, $t1, 2
    or $t0, $t0, $t1

    sw $t0, 0x000($0)

    lui $t0, \tile
    sll $t0, $t0, 8
    # subtract 1 for the high values (again, because RDP says so)
    addi $t1, $0, \sh - 1
    sll $t1, $t1, 12 + 2
    or $t0, $t0, $t1
    addi $t1, $0, \th -1
    sll $t1, $t1, 2
    or $t0, $t0, $t1

    sw $t0, 0x004($0)

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
    tileIndex format depth size tmem \
    palette clampT mirrorT maskT \
    shiftT clampS mirrorS maskS shiftS

    # Command 0x35
    lui $t0, 0xF500

    lui $t1, \format
    lui $t2, \depth
    sll $t1, $t1, 5
    sll $t2, $t2, 3
    or $t1, $t1, $t2
    or $t0, $t0, $t1

    addi $t1, $0, \size
    sll $t1, $t1, 9
    ori $t1, $t1, \tmem

    or $t0, $t0, $t1

    sw $t0, 0x000($0)

    lui $t0, \tileIndex
    sll $t0, $t0, 8

        # All this is a waste of time & space for now since I'm setting it all to zero.
        # But can stay until it becomes a problem or I learn mips macros a bit better.
        lui $t1, \palette
        sll $t1, $t1, 4
        or $t0, $t0, $t1

        lui $t1, \clampT
        sll $t1, $t1, 3
        or $t0, $t0, $t1

        lui $t1, \mirrorT
        sll $t1, $t1, 2
        or $t0, $t0, $t1

        addi $t1, $0, \maskT
        sll $t1, $t1, 14
        or $t0, $t0, $t1
    
        addi $t1, $0, \shiftT
        sll $t1, $t1, 10
        or $t0, $t0, $t1

        addi $t1, $0, \clampS
        sll $t1, $t1, 9
        or $t0, $t0, $t1

        addi $t1, \mirrorS
        sll $t1, $t1, 8
        or $t0, $t0, $t1

        addi $t1, $0, \maskS
        sll $t1, $t1, 4
        or $t0, $t0, $t1

        ori $t0, $t0, \shiftS

    sw $t0, 0x004($0)

    execRdp 2

.endm

###
#
# Command 0x3D
# @input format
# @input depth colour depth
# @input width
# @input address
# @sideAffect registers $2, $t1, $t2 affected.
# @sideAffect DMEM 0x000 - 0x008 affected.
###
.macro setTextureImage format depth width address
    # Command 0x3D
    lui $t0, 0xFD00

    lui $t1, \format
    lui $t2, \depth
    sll $t1, $t1, 5
    sll $t2, $t2, 3
    or $t1, $t1, $t2
    or $t0, $t0, $t1

    ori $t0, $t0, (\width - 1)
    sw $t0, 0x000($0)

    ##if it's a register
        sw \address, 0x004($0)
    ##else it's a constant...
        #liw $t0, \address
    ##endif

    execRdp 2
.endm
