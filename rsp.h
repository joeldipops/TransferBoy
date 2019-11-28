#ifndef RSP_INCLUDED
#define RSP_INCLUDED

#include <libdragon.h>
#include "screen.h"

typedef struct {
    uintptr_t InAddress;
    uintptr_t OutAddress;
    Rectangle Screen;
    bool IsColour;
    uint32_t padding[3];
} RspIn;

typedef void (*RspEventHandler)();

/**
 * Kicks off the RSP to render the next frame.
 * @param configuration configures how and what to render.
 */
void renderFrame(RspIn* configuration, RspEventHandler onRspBreak);

/**
 * DMAs a fixed set of instructions to the RSP ready to be run when we call run_ucode()
 */
void prepareMicrocode();

/**
 * Sets the RSP halt status so that it stops executing while we reload IMEM/DMEM
 */
void haltRsp();


#endif