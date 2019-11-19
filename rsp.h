#ifndef RSP_INCLUDED
#define RSP_INCLUDED

#include <libdragon.h>

typedef struct {
    uintptr_t InAddress;
    uintptr_t OutAddress;
    Rectangle Screen;
    bool IsColour;
} RspIn;

/**
 * DMAs a fixed set of instructions to the RSP ready to be run when we call run_ucode()
 */
void prepareMicrocode();

/**
 * Sets the RSP halt status so that it stops executing while we reload IMEM/DMEM
 */
void haltRsp();


#endif