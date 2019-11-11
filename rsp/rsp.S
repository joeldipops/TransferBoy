#include <ucode.S>
#include "rdp.h"

.set noat

###
# Sets the RDP to draw red rectangles.
main:
    # Stash config in some registers (including screen size & loc details)
    
    # while address < end of buffer
        # DMA in the next 2048 pixels
        
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
        setTextureImage 0, 2, 160, $OUT_BUFFER_POINTER
        
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
        textureRectangle 0 0, 0, 160, 12, 0, 0, 0, 0
        
    #end loop

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

# Fill remainder with zeros
# TODO: Check this is sensible
.org 2048, 0  # or is `break` better?  
        
    
    #loadTile 1, 0x10, 0x10, 0x20, 0x20
    #textureRectangle 1, 0x04, 0x04, 0x04, 0x04, 0x00, 0x00, 0x00, 0x00
    #setTile 0, 2, 0x0001, 0x0001, 1
    #setFillColour 0xF801
    #lui $5, 0x1234
    #ori $5, $5, 0x5678
    #setTextureImage 0, 2, 100, $5








