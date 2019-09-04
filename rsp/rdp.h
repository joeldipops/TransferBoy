###
# Command 0x3D
# @input format
# @input size
# @input width
# @input address
# @sideAffect registers $2, $3 affected.
###
.macro setTextureImage format size width address
    # Send the command to load the texture
    addi $2, $0, 0x3D # command 0x3D "Set Texture Image"
    sll $2, $2, 24

    addi $3, $0, 0x02 # 16b colours
    sll $3, $3, 19
    or $2, $2, $3

    addi $3, $0, 160 # 160 pixels wide.
    or $2, $2, $3


.endm