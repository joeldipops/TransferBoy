#include <libdragon.h>
#include <stdlib.h>
#include "core.h"
#include "logger.h"
#include "rsp.h"
#include "global.h"

extern const char rsp_code_start __attribute((section(".data")));
extern const char rsp_code_end __attribute((section(".data")));
extern const char rsp_code_size __attribute((section(".data")));

typedef struct {
    uintptr_t InAddress;
    uintptr_t OutAddress;
    Rectangle Screen;
    bool IsColour;
    byte BytePadding[3];
    uint32_t WordPadding[2];
} RspInterface;

/**
 * Memory Address that RSP will read from to get
 * the data it needs to know what to render.
 */
volatile RspInterface* rspInterface = (RspInterface*) RSP_INTERFACE_ADDRESS;

/**
 * Called if the RSP hits a break instruction.
 */
static void onRSPException() {
    printSegmentToFrame(2, "RSP Exception Raised - dumping rspInterface", (byte*) rspInterface);
    printSegmentToFrame(2, "RSP Exception Raised - dumping output", (byte*) rspInterface->OutAddress);
}

// Following taken from libdragon source since it doesn't provide direct access to these registers.
typedef struct SP_regs_s {
    /** @brief RSP memory address (IMEM/DMEM) */
    volatile void * RSP_addr;
    /** @brief RDRAM memory address */
    volatile void * DRAM_addr;
    /** @brief RDRAM->RSP DMA length */
    uint32_t rsp_read_length;
    /** @brief RDP->RDRAM DMA length */
    uint32_t rsp_write_length;
    /** @brief RSP status */
    uint32_t status;
    /** @brief RSP DMA full */
    uint32_t rsp_dma_full;
    /** @brief RSP DMA busy */
    uint32_t rsp_dma_busy;
    /** @brief RSP Semaphore */
    uint32_t rsp_semaphore;
} SP_regs_t;

static volatile struct SP_regs_s* const SP_regs = (struct SP_regs_s *)0xa4040000;

#define SP_DMA_IMEM 0x04001000
#define SP_STATUS_SET_HALT 0x00002

/**
 * DMAs a fixed set of instructions to the RSP ready to be run when we call run_ucode()
 */
void prepareMicrocode() {
    register_SP_handler(&onRSPException);
    set_SP_interrupt(1);

    unsigned long size = (unsigned long)&rsp_code_size;
    load_ucode((void*)&rsp_code_start, size);

    // Zero out the padding.
    rspInterface->BytePadding[0] = 0;
    rspInterface->BytePadding[1] = 0;
    rspInterface->BytePadding[2] = 0;
    rspInterface->WordPadding[0] = 0;
    rspInterface->WordPadding[1] = 0;
}

/**
 * Sets the RSP halt status so that it stops executing while we reload IMEM/DMEM
 */
void haltRsp() {
    SP_regs->status = SP_STATUS_SET_HALT;
}

/**
 * Kicks off the RSP to render the next frame.
 * @param inBuffer gameboy screen buffer pixels are picked up by the RSP from here.
 * @param outBuffer after RSP generates a texture, it will DMA it back into DRAM at this address.
 * @param screen size and position of the textures drawn by the RSP.
 * @param isColour if true, inBuffer words represent 2 bit DMG pixels.  Otherwise they are 16bit GBC pixels
 */
void renderFrame(uintptr_t inBuffer, uintptr_t outBuffer, Rectangle* screen, bool isColour) {
    haltRsp();

    rspInterface->InAddress = inBuffer;
    rspInterface->OutAddress = outBuffer;
    rspInterface->Screen = *screen;
    rspInterface->IsColour = isColour;

    data_cache_hit_writeback(rspInterface, sizeof(RspInterface));
    run_ucode();
}
