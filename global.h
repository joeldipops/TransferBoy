#ifndef SHARED_DEFINES
#define SHARED_DEFINES

/**
 * Defined globally and shared by ASM code and linker script
 * as address of the communication point between the CPU & RSP
 * IE. If CPU puts stuff here, it will be DMA'd into DMEM by RSP
 * We can reference this in our linker script if we run it through the cpp pre-compiler.
 */
#define RSP_INTERFACE_ADDRESS 0x80041000

#define GB_FREQ 4194304 /* Hz */
#define GB_LCD_WIDTH  160 /* px */
#define GB_LCD_HEIGHT 144 /* px */

// RSP_STATUS manipulation
#define SP_STATUS_HALT_OFF       0b00000001
#define SP_STATUS_HALT_ON        0b00000010
#define SP_STATUS_BROKE_OFF      0b00000100
#define SP_STATUS_INTERRUPT_OFF  0b00001000
#define SP_STATUS_INTERRUPT_ON   0b00010000

// aka signal 0
#define SP_STATUS_BUSY_OFF 0b010 << 8
#define SP_STATUS_BUSY_ON  0b100 << 8




#endif