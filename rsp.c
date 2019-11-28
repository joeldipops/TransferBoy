#include <libdragon.h>
#include <stdlib.h>
#include "core.h"
#include "logger.h"
#include "rsp.h"

extern const char rsp_code_start __attribute((section(".data")));
extern const char rsp_code_end __attribute((section(".data")));
extern const char rsp_code_size __attribute((section(".data")));

/**
 * Memory Address that RSP will read from to get
 * the data it needs to know what to render.
 * TODO: Is there are way to fix this at a known address?
 * Currently each compile might end up putting it somewhere completely different
 * and breaking everythig until I find what the new value is.
 */
static RspIn RspInterface;

/**
 * Called if the RSP hits a break instruction.
 */
static void onRSPException() {
    //while(true);
}

/**
 * Kicks off the RSP to render the next frame.
 * @param configuration configures how and what to render.
 * @param onRspBreak callback to run in the event of an rsp break instruction.
 */
void renderFrame(RspIn* configuration, RspEventHandler onRspBreak) {
    haltRsp();
    if (onRspBreak) {
        register_SP_handler(onRspBreak);
        set_SP_interrupt(1);
    }

    memcpy(&RspInterface, configuration, sizeof(RspIn));
    data_cache_hit_writeback(&RspInterface, sizeof(RspIn));
    run_ucode();
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
 * Sets the RSP halt status so that it stops executing while we reload IMEM/DMEM
 */
void haltRsp() {
    *(volatile uint32_t *) 0xA4080000 = SP_DMA_IMEM;
    SP_regs->status = SP_STATUS_SET_HALT;
}

/**
 * DMAs a fixed set of instructions to the RSP ready to be run when we call run_ucode()
 */
void prepareMicrocode() {
    unsigned long size = (unsigned long)&rsp_code_size;
    load_ucode((void*)&rsp_code_start, size);
}