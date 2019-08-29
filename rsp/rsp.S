##include <ucode.S>

.eqv _INITIAL_IN_BUFFER, 0x8
.eqv _INITIAL_OUT_BUFFER, 0xC
.eqv _INITIAL_IS_COLOUR, 0x10

.eqv _DMA_DMEM_SET, 0xA4040000
.eqv _DMA_DRAM_SET, 0xA4040004
.eqv _DMA_IN_LENGTH, 0xA404008
.eqv _DMA_OUT_LENGTH, 0xA40400C
.eqv _DMA_STATUS, 0xA4040010

.eqv DMA_BUSY, 0x04 | 0x10

.eqv _DMA_RDP_START, 0xA4100000
.eqv _DMA_RDP_END, 0xA4100004

.eqv DMEM_SIZE, 0x1000
.eqv DMEM_GB_BASE, 0x0800

.eqv GB_LCD_WIDTH, 160
.eqv GB_LCD_HEIGHT, 144
.eqv PIXEL_SIZE, 2
.eqv GB_BUFFER_SIZE, GB_LCD_WIDTH * GB_LCD_HEIGHT * PIXEL_SIZE

# address of gameboy pixel to be processed in DRAM
.set $GB_BUFFER_ADDRESS, $8

# points to end of gameboy pixel buffer in DRAM
.set $GB_BUFFER_END, $9

# points to current pixel to be processed in DMEM
.set $SECTION_ADDRESS, $10

# points to end of current 4kB section of gameboy pixel buffer in DRAM
.set $SECTION_END, $11

# Address to place next processed gameboy pixel in DMEM
.set $N64_BUFFER_POINTER, $12

# Set if pixels are in colour
.set $IS_COLOUR_PIXELS, $13

# Address to store massaged pixels where they will be picked up by the RDP
.set $OUT_BUFFER_POINTER, $14

.macro dmaOut source, dest, length
    # Set source.
    lw $1, source
    sw $1, _DMA_DMEM_SET($0)

    # Set destination
    lw $1, dest
    sw $1, _DMA_DRAM_SET($0)

    # set length (which kicks off DMA)
    addi $1, $s0, length
    sw $1, _DMA_OUT_LENGTH

    # wait for DMA to complete
    dmaOut.loop:
        lw $1, _DMA_STATUS($0)
        and $1, $1, DMA_BUSY
    bne $1, $0, dmaOut.loop
.endm

###
# Writes to the three DMA registers to kick off a transfer
# then waits for it to complete.
#
# @input source The source address in DRAM
# @input dest The destination adress in DMEM
# @input length The length of data to transfer.
###
.macro dmaIn source, dest, length
    # Set source.
    lw $1, source
    sw $1, _DMA_DRAM_SET($0)

    # Set destination
    lw $1, dest
    sw $1, _DMA_DMEM_SET($0)

    # set length (which kicks off DMA)
    addi $1, $s0, length
    sw $1, _DMA_IN_LENGTH

    # wait for DMA to complete
    dmaIn.loop:
        lw $1, _DMA_STATUS($0)
        and $1, $1, DMA_BUSY
    bne $1, $0, dmaIn.loop
.endm

###
# Takes the 16 bit GBC colour in reg and converts
# it in to a 32 bit n64 colour.
#
# @input reg contains the 16bit colour.
# @ouput reg contains the 32bit colour.
# TODO: Make params happen
###
.macro massageColour reg
    # blue
    and $2, \reg, 0x7C00
    srl $2, $2, 0x09

    # green
    and $3, \reg, 0x03E0
    sll $3, $3, 0x01

    # red
    and $4, \reg, 0x001F
    sll $4, $4, 0x0B

    or $2, $2, $3
    or $2, $2, $4
    ori $2, $2, 0x01

    sll \reg, $2, 0x10
    or \reg, \reg, $2
.endm

###
# Takes a 2bit DMG colour index and converts to a 32bit n64 colour.
#
# @input reg the dmg colour index (0 - 3)
# @output reg the 32bit dmg colour.
# TODO: Make params happen
###
.macro massageMonochrome reg
    lw \reg, MONOCHROME_PALETTE(\reg)
.endm

###
# RSP Routine reads the GB pixel buffer in 4kB chunks,
# Transforms pxiels to the n64 colour format
# Then sends them to the RDP for rendering.
#
# @input address 0x000 of DMEM is the sprite_t containing the pixel buffer in its data property.
###
main:
    # Stash config in some registers
    lw $GB_BUFFER_ADDRESS, _INITIAL_IN_BUFFER($0)
    lw $OUT_BUFFER_POINTER, _INITIAL_OUT_BUFFER($0)
    lw $IS_COLOUR_PIXELS, _INITIAL_IS_COLOUR($0)

    # these two lines may be wrong.  need to get the upper of GB_BUFFER_SIZE
    lui $1, GB_BUFFER_SIZE
    ori $1, $0, GB_BUFFER_SIZE

    add $GB_BUFFER_END, $GB_BUFFER_ADDRESS, $1

    # while address < end of buffer
    .whileHasSections:
        # DMA in the first 2048pixels.
        dmaIn $GB_BUFFER_ADDRESS, DMEM_BUFFER_BASE, (DMEM_SIZE / 2)
        addi $SECTION_ADDRESS, $0, DMEM_BUFFER_BASE
        addi $SECTION_END, $SECTION_ADDRESS, DMEM_SIZE
        add $N64_BUFFER_POINTER, $0, $0

        .whileHasBytes:
            # put the pixel into a register
            lw $1, 0($SECTION_ADDRESS)

            # Massage it
            beq $IS_COLOUR_PIXELS, 1, .gbcMassage
                massageMonochrome $1
                j .end
            .gbcMassage:
                massageColour $1
            .end:

            # push massaged colour on to the buffer.
            sw $N64_BUFFER_POINTER, 0($1)
            addi $N64_BUFFER_POINTER, $N64_BUFFER_POINTER, 0x04

            # increment address
            addi $SECTION_ADDRESS, $SECTION_ADDRESS, 0x02
        blt $SECTION_END, $SECTION_ADDRESS, .whileHasBytes

        # Send the data to the RDP

        # First we need to DMA the data back to DRAM so we have our memory back.
        dmaOut 0x000, $OUT_BUFFER_POINTER, DMEM_SIZE

        # TODO: Pass down screen details.

        # Send the command to load the texture
        addi $1, $0, 0x3D # command 0x3D "Set Texture Image"
        sll $1, $1, 24

        addi $2, $0, 0x02 # 16b colours
        sll $2, $2, 19
        or $1, $1, $2

        addi $2, $0, 160 # 160 pixels wide.
        or $1, $1, $2

        # Set up the command in DMEM
        sd $1, 0x000($0)
        sd $OUT_BUFFER_POINTER, 0x004($0)

        # DMA the command to the RSP
        sw $0, _DMA_RDP_START
        addi $1, $1, 0x02
        sw $1, _DMA_RDP_END

        # And tell it to draw the rectangle.
        # TODO

        addi $GB_BUFFER_ADDRESS, $GB_BUFFER_ADDRESS, DMEM_SIZE
    blt $GB_BUFFER_ADDRESS, $GB_BUFFER_END, .whileHasSections


# Array of the four colours of DMG software.
MONOCHROME_PALETTE:
    .word 0xffffffff
    .word 0xA529A529
    .word 0x52955295
    .word 0x00010001

# Fill remainder with zeros
.org 2048, 0