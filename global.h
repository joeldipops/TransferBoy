#ifndef RSP_INTERFACE_ADDRESS
/**
 * Defined globally because shared by both C and ASM code
 * as address of the communication point between the CPU & RSP
 * IE. If CPU puts stuff here, it will be DMA'd into DMEM by RSP
 */
#define RSP_INTERFACE_ADDRESS 0x80029388
#endif