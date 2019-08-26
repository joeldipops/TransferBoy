#include <ucode.S>

    # initial buffer address is passed in through DMA
    # cache that in a register 
    # While index < length
    ## DMA some screen data.
    ## Massage it
    ## Send it to RDP
    ## increment index

# Fill with zeros
.org 2048, 0