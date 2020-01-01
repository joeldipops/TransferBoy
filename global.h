#ifndef RSP_INTERFACE_ADDRESS
/**
 * Defined globally and shared by ASM code and linker script
 * as address of the communication point between the CPU & RSP
 * IE. If CPU puts stuff here, it will be DMA'd into DMEM by RSP
 * We can reference this in our linker script if we run it through the cpp pre-compiler.
 */
#define RSP_INTERFACE_ADDRESS 0x80040000
#endif