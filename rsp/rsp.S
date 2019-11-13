#include <ucode.S>
#include "rdp.h"

.set noat

###
# Writes to the three DMA registers to kick off a transfer
# from DMEM to DRAM then then waits for it to complete.
#
# @input {register} source The source address in DMEM
# @input {register} dest The destination adress in DRAM
# @input {register} length The length of data to transfer.
###
.macro dmaOut source, dest, length
    # Set source.
    mtc0 \source, $DMA_DMEM

    # Set destination
    mtc0 \dest, $DMA_DRAM

    # set length (which kicks off DMA)
    mtc0 \length, $DMA_OUT_LENGTH

    # wait for DMA to complete
    1:
        mfc0 $A, $DMA_BUSY
    bne $A, $0, 1b
.endm

###
# Writes to the three DMA registers to kick off a transfer
# from DRAM to DMEM then waits for it to complete.
#
# @input {register} source Holds the source address in DRAM
# @input {register} dest Holds the destination adress in DMEM
# @input {register} length Holds the length of data to transfer.
###
.macro _dmaIn source, dest, length
    # Set source.
    mtc0 \source, $DMA_DRAM

    # Set destination
    mtc0 \dest, $DMA_DMEM

    # set length (which kicks off DMA)
    mtc0 \length, $DMA_IN_LENGTH

    # wait for DMA to complete
    1:
        mfc0 $A, $DMA_BUSY
    bne $A, $0, 1b
.endm

dmaIn:
    _dmaIn $a0, $a1, $a2
    #jr $ra
    j ret

###
# Constants
###
.eqv RspIn_SPRITE_BUFFER, 0x00
.eqv RspIn_IS_COLOUR, 0x04
.eqv RspIn_OUT_BUFFER, 0x08
.eqv DMEM_SIZE, 0x1000
# Input pixels for the current rectangle go here.
.eqv DMEM_GB_BASE, 0x0800

.eqv GB_LCD_WIDTH, 160
.eqv GB_LCD_HEIGHT, 144
.eqv PIXEL_SIZE, 2
.eqv _GB_BUFFER_SIZE, GB_LCD_WIDTH * GB_LCD_HEIGHT * PIXEL_SIZE

###
# Register aliases
###


# address of gameboy pixel to be processed in DRAM
.set $GB_BUFFER_ADDRESS, $16

# points to end of gameboy pixel buffer in DRAM
.set $GB_BUFFER_END, $17

# points to current pixel to be processed in DMEM
.set $SECTION_ADDRESS, $18

# points to end of current 4kB section of gameboy pixel buffer in DRAM
.set $SECTION_END, $19

# Address to place next processed gameboy pixel in DMEM
.set $N64_BUFFER_POINTER, $20

# Set if pixels are in colour
.set $IS_COLOUR, $21

# Address to store massaged pixels where they will be picked up by the RDP
.set $OUT_BUFFER_ADDRESS, $22

###
# Sets the RDP to draw red rectangles.
main:
    # Stash config in some registers (including screen size & loc details)
    lw $GB_BUFFER_ADDRESS, RspIn_SPRITE_BUFFER($0)
    lw $OUT_BUFFER_ADDRESS, RspIn_OUT_BUFFER($0)
    lw $IS_COLOUR, RspIn_IS_COLOUR($0)
    lw $GB_BUFFER_END, GB_BUFFER_SIZE
    add $GB_BUFFER_END, $GB_BUFFER_END, $GB_BUFFER_ADDRESS

    # while address < end of buffer
    whileHasRectangles:
        # DMA in the next 2048 pixels
        add $a0, $0, $GB_BUFFER_ADDRESS
        addi $a1, $0, DMEM_GB_BASE
        addi $a2, $0, DMEM_SIZE / 2
        # somehow, jr $ra isn't working at all.  :(
        jal dmaIn
        ret:


        # for those pixels
            # put a pixel into a register
            # massage it
            
            # push massaged colour on to the out-buffer
            
            # increment address
        #end loop

        # Once that's done, ask the RDP to render it.
        
        # First, DMA the data back to DRAM so we have our limited memory back.
        
        # Then make the necessary RDP commands to render our 'textured' line.

        ## Tell RDP where to look for texture data.
        ## rbga colour format, 16 bit colours, 160 pixels, address in DRAM
        setTextureImage 0, 2, 160, $OUT_BUFFER_ADDRESS
        
        ## Set up a tile now that the RDP has the data.
        ## rgba format, 16b, 160*2B fit in 64x64 words (CHECK THIS!!), tmem address 0 (CHECK THAT TOO)
        ## maskT 8 (CHECK), maskS 2 (CHECK!!!)
        setTile 0, 2, 64, 0, 0, 0, 0, 8, 0, 0, 0, 2, 0

        ## Give tile a size & shape
        ## tile = 0 sl,tl = {0,0}, sh,th = {width - 1, height - 1}
        loadTile 0, 0, 0, 159, 11

        ## Draw a rectangle with the texture we have set up
        ## tile = 0 x1,y1 = {0, 0} x2,y2 = {160, 12}
        # not doing anything fancy with the s & ts
        textureRectangle 0 320 * 4, 240 * 4, 0, 0, 0, 0, 0, 0

        # increment address pointer.
        addi $GB_BUFFER_ADDRESS, $GB_BUFFER_ADDRESS, DMEM_SIZE
    # loop until we reach end of buffer.
    #blt $GB_BUFFER_ADDRESS, $GB_BUFFER_END, whileHasRectangles
    sub $A, $GB_BUFFER_ADDRESS, $GB_BUFFER_END
    bgtz $A, whileHasRectangles

# job done, idle until IMEM refreshed.
wait:
    nop
    j wait

# raise error if we get to this point.
break

# Array of the four colours of DMG software.
MONOCHROME_PALETTE:
    .word 0xffffffff
    .word 0xA529A529
    .word 0x52955295
    .word 0x00010001

GB_BUFFER_SIZE:
    .word _GB_BUFFER_SIZE


# Fill remainder with break instruction
.org 2048, 0x0000000D