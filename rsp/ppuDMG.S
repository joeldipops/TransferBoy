#include <ucode.S>
#include "../global.h"
#include "init.S"
#include "rsp.mips"
#include "rdp.mips"

###------------------------------------------
# Renders a scan line of the gameboy screen.
#
###------------------------------------------

###
# Constants
###

# OAM Structure
.eqv Oam_Y_BLOCK, 0x00
.eqv Oam_X_BLOCK, 0x01
.eqv Oam_TILE_INDEX, 0x02
.eqv Oam_FLAGS, 0x03

# Oam_FLAGS Structure
.eqv Oam_PRIORITY,  0b10000000
.eqv Oam_Y_FLIP,    0b01000000
.eqv Oam_X_FLIP,    0b00100000

.eqv OAM_SIZE, 0xA0
.eqv SPRITE_COUNT, OAM_SIZE / 4 # 40 sprites in OAM
.eqv SPRITE_LIMIT, 0x0A # 10 sprites per scanline

# RSP Interface Structure
.eqv RspIn_OAM_ADDRESS, 0x00
.eqv RspIn_VRAM_TILE_DATA_ADDRESS, 0x04
.eqv RspIn_VRAM_TILE_INDEX_ADDRESS, 0x08
.eqv RspIn_PPU_SETTINGS_ADDRESS, 0x0C
.eqv RspIn_OUT_ADDRESS, 0x10
.eqv RspIn_SCREEN_POSITION, 0x14
.eqv RspIn_RECTANGLE_SIZE, 0x18
.eqv RspIn_IS_BUSY, 0x1C

.eqv INTERFACE_SIZE, 0x20

# PPU Settings Structure
## These are the gameboy LCD registers.

.eqv Ppu_LCD_CONTROL, 0x00
.eqv Ppu_LCD_STATUS, 0x01
.eqv Ppu_BACKGROUND_SCROLL_Y, 0x02
.eqv Ppu_BACKGROUND_SCROLL_X, 0x03
.eqv Ppu_CURRENT_LINE, 0x04         # This is the line we're drawing.
.eqv Ppu_NEXT_INTERRUPT_LINE, 0x05  # Probably not important.  Interrupts to be handled elsewhere?
.eqv Ppu_DMA_SOURCE, 0x06
.eqv Ppu_BACKGROUND_PALETTE, 0x07
.eqv Ppu_SPRITE_PALETTE_0, 0x08
.eqv Ppu_SPRITE_PALETTE_1, 0x09
.eqv Ppu_WINDOW_TOP, 0x0A
.eqv Ppu_WINDOW_LEFT, 0x0B
.eqv PPU_SETTINGS_SIZE, 0x10

# LCDC Flags structure
.eqv Lcdc_LCD_SWITCH,           0b10000000
.eqv Lcdc_WINDOW_TILE_MAP,      0b01000000
.eqv Lcdc_WINDOW_SWITCH,        0b00100000
.eqv Lcdc_TILE_DATA,            0b00010000
.eqv Lcdc_BACKGROUND_TILE_MAP,  0b00001000
.eqv Lcdc_SPRITE_SIZE,          0b00000100
.eqv Lcdc_SPRITE_SWITCH,        0b00000010
.eqv Lcdc_LCD_PRIORITY,         0b00000001

.eqv DMEM_SIZE, 0x1000
.eqv BUFFER_SIZE, GB_LCD_WIDTH * 2 # ( x2 for 16bit colours)

###
# Memory Map
###
.eqv TILE_CACHE, 0xBD0      # 50 possible tiles with 16B/tile assuming 2b colour.
.eqv PPU_SETTINGS, 0x0EF0
.eqv INPUT, 0x0F00
.eqv OAM_CACHE, 0x0F40      # 10 OAM entries per scanline at 4B/entry
.eqv OAM_MIRROR, 0x0F50
.eqv MONOCHROME_PALETTE, 0x0FF0

### Palette Values

.eqv LIGHTEST,0x0000FFFF
.eqv LIGHTER, 0x0000A529
.eqv DARKER,  0x00005295
.eqv DARKEST, 0x00000001



###
# Register aliases
###

# Address to store massaged pixels where they will be picked up by the RDP
.set $OUT_BUFFER_ADDRESS, $s0

# Number of pixels left to render in current iteration.
.set $PIXEL_COUNTER, $s1

# Address to place next processed gameboy pixel in DMEM
.set $N64_BUFFER_POINTER, $s2

# Cached Height may not be necessary
.set $RECTANGLE_HEIGHT, $s3

# Cached value of LCDC bit 2 flag
.set $IS_8x16_MODE, $s4

# Left and Top position of line to render.
.set $START_POSITION, $s5

# Length & Width of the line to draw in N64 pixels so it can be scaled.
.set $RECTANGLE_SIZE, $s6

.set $SCAN_LINE, $s7



###
# Takes a 2bit DMG colour index and converts to a 16bit n64 colour.
#
# @input reg the dmg colour index (0 - 3)
# @output reg the 16bit dmg colour.
###
massageMonochrome:
    # zero out irrelevant bits
    andi $t0, $t0, 3
    # and multiply by four since we deal with 32bit addresses
    sll $t0, $t0, 2
    lw $t0, MONOCHROME_PALETTE($t0)
    jr $ra

###
# Sets the RDP to draw red rectangles.
main:
    # Initialise this task

    ## Bring in the configuration.
    liw $t0, RSP_INTERFACE_ADDRESS
    dmaIn $t0, DMEM_GB_BASE, INTERFACE_SIZE

    ## Stash config in some registers so whole memory space is available.
    lw $OUT_BUFFER_ADDRESS, RspIn_OUT_ADDRESS($0)
    lw $START_POSITION, RspIn_SCREEN_POSITION($0)
    lw $RECTANGLE_SIZE, RspIn_RECTANGLE_SIZE($0)
    andi $RECTANGLE_HEIGHT, $RECTANGLE_SIZE, 0x0000FFFF

    ## Set up monochrome colour palette in memory
    liw $t0, LIGHTEST
    sw $t0, MONOCHROME_PALETTE($0)
    liw $t0, LIGHTER
    sw $t0, (MONOCHROME_PALETTE + 4)($0)
    liw $t0, DARKER
    sw $t0, (MONOCHROME_PALETTE + 8)($0)
    liw $t0, DARKEST
    sw $t0, (MONOCHROME_PALETTE + 0xC)($0)

    # Load and cache the gameboy LCD registers.
    lw $t0, (RSP_INTERFACE_ADDRESS + RspIn_PPU_SETTINGS_ADDRESS)($0)
    dmaIn $t0, PPU_SETTINGS, PPU_SETTINGS_SIZE

    ## Cache the current scan line.
    lw $SCAN_LINE, (PPU_SETTINGS + Ppu_CURRENT_LINE)($0)

    ## Cache if we're in 8x16 sprite mode.
    lw $IS_8x16_MODE, (PPU_SETTINGS + Ppu_LCD_CONTROL)($0)
    andi $IS_8x16_MODE, $IS_8x16_MODE, Lcdc_SPRITE_SIZE
    srl $IS_8x16_MODE, $IS_8x16_MODE, 2

    # Build up this scanline to a buffer than can be rendered as a texture.

    ## Find the sprites relevant to the current scanline so we don't have to loop through 40 sprites every pixel.

    lw $t0, (INPUT + RspIn_OAM_ADDRESS)($0)
    dmaIn $t0, OAM_MIRROR, OAM_SIZE

    # Name temporary registers for this block
    .set $screenSpritesRemaining, $t1
    .set $lineSpritesRemaining, $t2
    .set $y1, $t3
    .set $y2, $t4
    .set $entry, $t5

    addi $screenSpritesRemaining, $0, SPRITE_COUNT
    addi $lineSpritesRemaining, $0, SPRITE_LIMIT
    .whileHasOam:
        # Load the OAM for the current sprite

        ## Figure out what sprite we are on.
        addi $t0, $0, SPRITE_COUNT
        sub $t0, $t0, $screenSpritesRemaining
        ## Multiply by four to get the address offset.
        sll $t0, $t0, 2
        lw $entry, OAM_MIRROR($t0)

        ## We care about the Y position, which is the lowest byte
        andi $y1, $entry, 0x000000FF

        ## Multiply by 8 since a tile is 8 pixels tall.
        sll $y1, $y1, 3

        # Test that the sprite will appear on this scanline

        ## $t3 is lower bound, $t0 is upper bound.  Double it if 8x16 mode is on.
        addi $y2, $y1, 8
        beq $IS_8x16_MODE, $0, 1f
            addi $y2, $y2, 8
        1:

        ## if bottom of the sprite is greater than the current scan line, we don't need to render it here.
        sub $t0, $y1, $SCAN_LINE
        bgtz $t0, .irrelevantSprite

        ## if top of the sprite is less than the current scan line, skip that too.
        sub $t0, $SCAN_LINE, $y2
        bgtz $t0, .irrelevantSprite

            # Cache a sprite that exists on this scan line.

            ## Calculate the destination offset to cache at.
            addi $t0, $0, SPRITE_LIMIT
            sub $t0, $t0, $lineSpritesRemaining
            sll $t0, $t0, 2

            ## And cache it
            sw $entry, OAM_CACHE($t0)

            # There is a limit of 10 sprites per scanline.
            ## Once we've found that many relevant sprites, we're done.

            ## The limit should actually be done from left to right (i think), but assumption is most 
            ## code won't break the limit anyway.
            ## Will create a job to improve accuracy of this once general case is working.
            addi $lineSpritesRemaining, $lineSpritesRemaining, -1
            bgtz $lineSpritesRemaining, .whileHasOam_break

        .irrelevantSprite:

        addi $screenSpritesRemaining, $screenSpritesRemaining, -1
        bgtz $screenSpritesRemaining, .whileHasOam
    .whileHasOam_break:

    .set $t1 $spriteCount
    addi $t0, $0, SPRITE_LIMIT
    sub $spriteCount, $t0, $lineSpritesRemaining

    # Set them all to 0 so we'll error if try to use them as registers.
    .set $screenSpritesRemaining, 0
    .set $lineSpritesRemaining, 0
    .set $y1, 0
    .set $y2, 0
    .set $entry, 0


    addi $PIXEL_COUNTER, $0, GB_LCD_WIDTH
    .whileHasPixels:
        ## First draw any sprites.

        ### Loop through the sprite cache
        .whileHasSprites
            addi $spriteCount, $spriteCount, -1

            ### Does this sprite appear in this x position?

                ### Is the tile in the tile cache?

                ### Set up a cache entry

                ### DMA the tile into the cache
                dmaIn $t0

            ### Find the pixel that correlates to this x position.

            ### Check it against the palette

                ### Unless it's transparent, mark this pixel as filled

                ### convert to a 16bit N64 colour

                ### add it to the output buffer.
        bgtz $spriteCount, .whileHasSprites

        ## Then the window

        ## Then the background


        # .................. Go through VRAM...
        # .....................Massage the colours to n64 16bit colours.

        addi $PIXEL_COUNTER, $PIXEL_COUNTER, -1

    # end loop
    bgtz $PIXEL_COUNTER, .whileHasPixels


    # Ask the RDP to render the line we just built.

    ## Sync before loading a new texture.
    syncPipe

    ## DMA the data back to DRAM so that the RDP can pick it up.
    dmaOut DMEM_GB_BASE, $OUT_BUFFER_ADDRESS, BUFFER_SIZE

    ## Tell RDP where to look for texture data.
    setTextureImage RGBA_FORMAT, COLOUR_DEPTH_16, GB_LCD_WIDTH, $OUT_BUFFER_ADDRESS

    ## Set up a tile now that the RDP has the data.
    setTile 1, RGBA_FORMAT, COLOUR_DEPTH_16, 64, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0

    ## Give tile a size & shape
    loadTile 1, 0, 0, GB_LCD_WIDTH, 1

    # Calculate where to draw the line.
    add $a0, $START_POSITION, $0
    srl $a0, $a0, 16
    andi $a1, $START_POSITION, 0x0000FFFF

    add $a2, $RECTANGLE_SIZE, $0
    srl $a2, $a2, 16

    add $a2, $a2, $a0
    add $a3, $RECTANGLE_HEIGHT, $a1

    ## Draw a rectangle with the texture we have set up
    textureRectangle 1, $a0, $a1, $a2, $a3, 0, 0, 2048, 512


    # Clean-up now that we're done.

    ## Reset DRAM flag that says we're done with this work.
    sw $0, RspIn_IS_BUSY($0)

    liw $t0, RSP_INTERFACE_ADDRESS
    dmaOut DMEM_GB_BASE, $t0, INTERFACE_SIZE

# job done
stall:
    nop
    j stall

break
# Fill remainder with break instruction
.org 2048, 0x0000000D
