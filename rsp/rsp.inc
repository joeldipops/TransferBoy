#ifndef RSP_DEFS_INCLUDED
#define RSP_DEFS_INCLUDED

# co-processor 0 registers
.set $c0,  $0
.set $c1,  $1
.set $c2,  $2
.set $c3,  $3
.set $c4,  $4
.set $c5,  $5
.set $c6,  $6
.set $c7,  $7
.set $c8,  $8
.set $c9,  $9
.set $c10, $10
.set $c11, $11
.set $c12, $12
.set $c13, $13
.set $c14, $14
.set $c15, $15

# co-processor 0 register descriptions.
.set $DMA_DMEM,         $c0
.set $DMA_DRAM,         $c1
.set $DMA_IN_LENGTH,    $c2
.set $DMA_OUT_LENGTH,   $c3
.set $RSP_STATUS,       $c4
.set $DMA_FULL,         $c5
.set $DMA_BUSY,         $c6
.set $RSP_RESERVED,     $c7
.set $RDP_CMD_START,    $c8
.set $RDP_CMD_END,      $c9
.set $RDP_CMD_CURRENT,  $c10
.set $RDP_CMD_STATUS,   $c11
.set $RDP_CMD_CLOCK,    $c12
.set $RDP_CMD_BUSY,     $c13
.set $RDP_CMD_PIPE_BUSY,$c14
.set $RDP_CMD_TMEM_BUSY,$c15

.equ DRAM_RDP_DMA, 1
.equ DMEM_RDP_DMA, 2
.equ RDP_DMA_BUSY, 0x0100

# primary accumulator
.set $A, $8

.set $a0, $4
.set $a1, $5
.set $a2, $6
.set $a3, $7

.set $t1, $9
.set $t2, $10

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

###
# See dmaIn macro below
###
_dmaIn:
    # Don't do anything until DMA is no longer busy
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

###
# See dmaOut macro below
###
_dmaOut:
    # Don't do anything until DMA is not busy.
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

#endif