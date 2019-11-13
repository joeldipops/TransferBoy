#include <libdragon.h>
#include <stdlib.h>
#include "core.h"
#include "logger.h"

extern const char rsp_code_start __attribute((section(".data")));
extern const char rsp_code_end __attribute((section(".data")));
extern const char rsp_code_size __attribute((section(".data")));

/**
 * Called if the RSP hits a break instruction.
 * TODO - Something else
 */
static void onRSPException() {
    logAndPauseFrame(2, "RSP INTERRUPT");
}

/**
 * DMAs a fixed set of instructions to the RSP ready to be run when we call run_ucode()
 */
void prepareMicrocode() {
    register_SP_handler(&onRSPException);
    set_SP_interrupt(1);

    unsigned long size = (unsigned long)&rsp_code_size;
    load_ucode((void*)&rsp_code_start, size);
}