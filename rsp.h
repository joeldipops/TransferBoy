#ifndef RSP_INCLUDED
#define RSP_INCLUDED

#include <libdragon.h>

/**
 * DMAs a fixed set of instructions to the RSP ready to be run when we call run_ucode()
 */
void prepareMicrocode();

/**
 * DMAs a block of memory to DMEM where it can be accessed by the RSP
 * @param source Address of memory to transfer.
 * @param length size of memory to transfer.
 */
void rspDMAWrite(void* source, size_t length);

/**
 * DMAs a block of memory from RSP DMEM back to main memory.
 * @param destination address of memory to transfer to.
 * @param length size of memory to transfer.
 */
void rspDMARead(void* destination, size_t length);


#endif