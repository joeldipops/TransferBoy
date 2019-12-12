
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
