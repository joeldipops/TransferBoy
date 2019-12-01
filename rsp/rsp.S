#include <ucode.S>
#include "../global.h"
#include "init.S"
#include "rdp.h"

.set noat

###
# Constants
###
.eqv RspIn_IN_ADDRESS, 0x00
.eqv RspIn_OUT_ADDRESS, 0x04
.eqv RspIn_SCREEN_POSITION, 0x08
.eqv RspIn_SCREEN_SIZE, 0x0C
.eqv RspIn_IS_COLOUR, 0x10

.eqv INTERFACE_SIZE, 0x20

.eqv DMEM_SIZE, 0x1000
.eqv BUFFER_SIZE, 0x0F00
# Input pixels for the current rectangle go here.
.eqv DMEM_GB_BASE, 0

.eqv MONOCHROME_PALETTE, 0x0FF0
.eqv LIGHTEST, 0xFFFFFFFF
.eqv LIGHTER, 0xA529A529
.eqv DARKER,  0x52955295
.eqv DARKEST, 0x00010001

.eqv GB_LCD_WIDTH, 160
.eqv GB_LCD_HEIGHT, 144
.eqv PIXEL_SIZE, 2
.eqv GB_BUFFER_SIZE, GB_LCD_WIDTH * GB_LCD_HEIGHT * PIXEL_SIZE

###
# Register aliases
###

# address of gameboy pixel to be processed in DRAM
.set $GB_BUFFER_ADDRESS, $16

# points to current pixel to be processed in DMEM
.set $PIXEL_ADDRESS, $17

# Number of rectangles left to render in current iteration.
.set $RECTANGLE_COUNTER, $18

# Number of pixels left to render in current iteration.
.set $PIXEL_COUNTER, $19

# Address to place next processed gameboy pixel in DMEM
.set $N64_BUFFER_POINTER, $20

# Set if pixels are in colour
.set $IS_COLOUR, $21

# Address to store massaged pixels where they will be picked up by the RDP
.set $OUT_BUFFER_ADDRESS, $22

# Left and Top of screen
.set $SCREEN_POSITION, $23

# Length and Width of screen
.set $SCREEN_SIZE, $26

###
# Load Immediate Word
# loads a 32bit immediate value into register.
# @input \reg Register to load.
# @input \word Value to load.
###
.macro liw reg word
    lui \reg, \word >> 16
    ori \reg, \reg, \word & 0xFFFF
.endm


_dmaIn:
    1:
        mfc0 $A, $RSP_RESERVED
    bne $A, $0, 1b

    2:
        mfc0 $A, $DMA_FULL
    bne $A, $0, 2b

    # Set source.
    mtc0 $a0, $DMA_DRAM

    # Set destination
    mtc0 $a1, $DMA_DMEM

    # set length (which kicks off DMA)
    addi $a2, $a2, -1
    mtc0 $a2, $DMA_IN_LENGTH

    # wait for DMA to complete
    3:
        mfc0 $A, $DMA_BUSY
    bne $A, $0, 3b

    # release the flag
    mtc0 $0, $RSP_RESERVED
    jr $ra


###
# Writes to the three DMA registers to kick off a transfer
# from DRAM to DMEM then waits for it to complete.
#
# @input source Holds the source address in DRAM
# @input dest Holds the destination adress in DMEM
# @input length Holds the length of data to transfer.
###
.macro dmaIn source, dest, length
    add $a0, $0, \source
    addi $a1, $0, \dest
    addi $a2, $0, \length
    jal _dmaIn
    
.endm

_dmaOut:
    1:
        mfc0 $A, $RSP_RESERVED
    bne $A, $0, 1b

    2:
        mfc0 $A, $DMA_FULL
    bne $A, $0, 2b

    # Set source.
    mtc0 $a0, $DMA_DMEM

    # Set destination
    mtc0 $a1, $DMA_DRAM

    # set length (which kicks off DMA)
    addi $a2, $a2, -1
    mtc0 $a2, $DMA_OUT_LENGTH

    # wait for DMA to complete
    3:
        mfc0 $A, $DMA_BUSY
    bne $A, $0, 3b

    # release the flag
    mtc0 $0, $RSP_RESERVED
    jr $ra

###
# Writes to the three DMA registers to kick off a transfer
# from DMEM to DRAM then then waits for it to complete.
#
# @input source The source address in DMEM
# @input dest The destination adress in DRAM
# @input length The length of data to transfer.
###
.macro dmaOut source, dest, length
    addi $a0, $0, \source
    add $a1, $0, \dest
    addi $a2, $0, \length
    jal _dmaOut
.endm

###
# Takes a 2bit DMG colour index and converts to a 32bit n64 colour.
#
# @input reg the dmg colour index (0 - 3)
# @output reg the 32bit dmg colour.
###
massageMonochrome:
    # zero out irrelevant bits
    andi $A, $A, 3
    # and multiply by four since we deal with 32bit addresses
    sll $A, $A, 2
    lw $A, MONOCHROME_PALETTE($A)
    jr $ra

###
# Takes the 16 bit GBC colour in reg and converts
# it in to a 32 bit n64 colour.
#
# @input reg contains the 16bit colour.
# @ouput reg contains the 32bit colour.
###
massageColour:
    # it's sign extended, so...
    andi $A, $A, 0x0000FFFF

    # blue
    andi $t1, $A, 0x7C00
    srl $t1, $t1, 0x09

    # green
    andi $t2, $A, 0x03E0
    sll $t2, $t2, 0x01

    # red
    andi $t3, $A, 0x001F
    sll $t3, $t3, 0x0B

    or $t1, $t1, $t2
    or $t1, $t1, $t3
    ori $t1, $t1, 0x01

    sll $A, $t1, 0x10
    or $A, $A, $t1
    jr $ra

###
# Sets the RDP to draw red rectangles.
main:
    liw $A, RSP_INTERFACE_ADDRESS

    # Bring in the configuration.
    dmaIn $A, DMEM_GB_BASE, INTERFACE_SIZE

    # Stash config in some registers (including screen size & loc details)
    lw $GB_BUFFER_ADDRESS, RspIn_IN_ADDRESS($0)
    lw $OUT_BUFFER_ADDRESS, RspIn_OUT_ADDRESS($0)
    lw $SCREEN_POSITION, RspIn_SCREEN_POSITION($0)
    lw $SCREEN_SIZE, RspIn_SCREEN_SIZE($0)
    lw $IS_COLOUR, RspIn_IS_COLOUR($0)

    bne $IS_COLOUR, $0, 1f
        # If monochrome, set up palette in memory
        liw $A, LIGHTEST
        sw $A, MONOCHROME_PALETTE($0)
        liw $A, LIGHTER
        sw $A, (MONOCHROME_PALETTE + 4)($0)
        liw $A, DARKER
        sw $A, (MONOCHROME_PALETTE + 8)($0)
        liw $A, DARKEST
        sw $A, (MONOCHROME_PALETTE + 0xC)($0)
    1:

    # TODO - Magic number
    addi $RECTANGLE_COUNTER, $0, 24

    # while address < end of buffer
    .whileHasRectangles:
        # DMA in the next 2048 pixels
        dmaIn $GB_BUFFER_ADDRESS, DMEM_GB_BASE, BUFFER_SIZE

        addi $PIXEL_ADDRESS, $0, DMEM_GB_BASE
        # Starts at 0 and mostly fills up DMEM as we get through GB pixels.
        add $N64_BUFFER_POINTER, $0, $0

        # TODO - get rid of magic numbers.
        addi $PIXEL_COUNTER, $0, GB_LCD_WIDTH * 6

        # for those pixels
        .whileHasPixels:
            # put a pixel into a register
            lh $A, 0($PIXEL_ADDRESS)

            # massage it from gameboy format to n64 format.
            beq $IS_COLOUR, $0, 1f
                jal massageColour
                j 2f
            1:
                jal massageMonochrome
            2:

            # push massaged colour on to the out-buffer
            sw $A, 0($N64_BUFFER_POINTER)
            # and increment
            ## turns out the compiler is actually storing each 16bit gameboy pixels
            ## in 32bits.  That concerns me somewhat because if it changes this will break oh well.
            addi $PIXEL_ADDRESS, $PIXEL_ADDRESS, 0x04
            ## n64 pixels are 32bits though.
            addi $N64_BUFFER_POINTER, $N64_BUFFER_POINTER, 0x04
            addi $PIXEL_COUNTER, $PIXEL_COUNTER, -1

        # end loop
        bgtz $PIXEL_COUNTER, .whileHasPixels

        # DMA the data back to DRAM so we have our limited memory back.
        dmaOut 0x000, $OUT_BUFFER_ADDRESS, BUFFER_SIZE

        # Ask the RDP to render the textured line we just built.

        # TODO - Pass proper values to these.
        # Sync before loading a new texture.
        syncPipe

        ## Tell RDP where to look for texture data.
        ## rbga colour format, 16 bit colours, 160 pixels, address in DRAM
        setTextureImage RGBA_FORMAT, COLOUR_DEPTH_16, GB_LCD_WIDTH, $OUT_BUFFER_ADDRESS

        ## Set up a tile now that the RDP has the data.
        ## rgba format, 16b, 160*2B fit in 64x64 words.
        setTile 1, RGBA_FORMAT, COLOUR_DEPTH_16, 64, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0

        ## Give tile a size & shape
        ## tile = 1 sl,tl = {0,0}, sh,th = {width - 1, height - 1}
        loadTile 1, 0, 0, GB_LCD_WIDTH, 6

        # TODO Cache this somewhere. May need to use some less 
        # than conventional registers.
        # Once there, can be used above as well.

        add $a0, $SCREEN_POSITION, $0
        srl $a0, $a0, 16
        andi $a1, $SCREEN_POSITION, 0x0000FFFF

        add $a2, $SCREEN_SIZE, $0
        srl $a2, $a2, 16
        andi $a3, $SCREEN_SIZE, 0x0000FFFF

        add $a2, $a2, $a0
        add $a3, $a3, $a1

        #addi $a2, $a2, 160
        #addi $a3, $a3, 144

        ## Draw a rectangle with the texture we have set up
        ## tile = 1 x1,y1 = {0, 0} x2,y2 = {160, 6}
        ### not doing anything fancy with the s & ts
        textureRectangle 1, $a0, $a1, $a2, $a3, 0, 0, 2048, 512

        # increment address pointer.
        # TODO - Magic numbers.
        addi $GB_BUFFER_ADDRESS, $GB_BUFFER_ADDRESS, GB_LCD_WIDTH * 2 * 6

        # increment draw top
        # TODO - By determined block height.
        addi $SCREEN_POSITION, $SCREEN_POSITION, 12
        addi $RECTANGLE_COUNTER, $RECTANGLE_COUNTER, -1

    # loop until there are no more rectangles to draw.
    bgtz $RECTANGLE_COUNTER, .whileHasRectangles

# job done
break

# Fill remainder with break instruction
.org 2048, 0x0000000D
