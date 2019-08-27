##include <ucode.S>

.eqv _INITIAL_BUFFER, 0x8
.eqv _DMA_DMEM_SET, 0xA4040000
.eqv _DMA_DRAM_SET, 0xA4040004
.eqv _DMA_IN_LENGTH, 0xA404008
.eqv _DMA_OUT_LENGTH, 0xA40400C
.eqv DMEM_SIZE, 0x1000

.eqv GB_LCD_WIDTH, 160
.eqv GB_LCD_HEIGHT, 144
.eqv PIXEL_SIZE, 2
.eqv GB_BUFFER_SIZE, GB_LCD_WIDTH * GB_LCD_HEIGHT * PIXEL_SIZE

.set $GB_BUFFER_ADDRESS, $s6
.set $GB_BUFFER_END, $s7

###
# Writes to the three DMA registers to kick off a transfer
# then waits for it to complete.
# @param source The source address in DRAM
# @param dest The destination adress in DMEM
# @param length The length of data to transfer.
###
.macro dmaIn source, dest, length
    # Set source.
    lw $s1, source
    sw $s1, _DMA_DRAM_SET($s0)

    # Set destination
    lw $s1, dest
    sw $s1, _DMA_DMEM_SET($s0)

    # set length (which kicks off DMA)
    addi $s1, $s0, length
    sw $s1, _DMA_IN_LENGTH

    # wait for DMA to complete
    # TODO
.endm

###
# RSP Routine reads the GB pixel buffer in 4kB chunks,
# Transforms pxiels to the n64 colour format
# Then sends them to the RDP for rendering.
#
# @input address 0x000 of DMEM is the sprite_t containing the pixel buffer in its data property.
###
main:
    # Cache address of buffer in DRAM
    lw $GB_BUFFER_ADDRESS, INITIAL_BUFFER_ADDR($s0)

    # these two lines may be wrong.
    lui $s1, GB_BUFFER_SIZE
    ori $s1, $s0, GB_BUFFER_SIZE

    add $GB_BUFFER_END, $GB_BUFFER_ADDRESS, $s1

# While address < end of buffer.
loop:
        # DMA in the first 4kB.
        dmaIn $GB_BUFFER_ADDRESS, 0, DMEM_SIZE

        # Massage it
        # TODO

        # Send it to RDP
        # TODO

        # increment address
        addi $GB_BUFFER_ADDRESS, $GB_BUFFER_ADDRESS, DMEM_SIZE
    blt $GB_BUFFER_ADDRESS, $GB_BUFFER_END, loop

# Fill with zeros
.org 2048, 0