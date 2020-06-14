#include <libdragon.h>
#include <stdlib.h>
#include "core.h"
#include "rsp.h"
#include "global.h"
#include "logger.h"

extern const char ppuDMG_code_start __attribute((section(".data")));
extern const char ppuDMG_code_end __attribute((section(".data")));
extern const char ppuDMG_code_size __attribute((section(".data")));

extern const char ppuDMG_data_start __attribute((section(".data")));
extern const char ppuDMG_data_end __attribute((section(".data")));
extern const char ppuDMG_data_size __attribute((section(".data")));

typedef struct {
    u32 Settings[8];
} RspInterface;

/**
 * Memory Address that RSP will read from to get data shared between the two processors.
 */
volatile RspInterface rspInterface __attribute__ ((section (".rspInterface"))) __attribute__ ((__used__));

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

static volatile struct SP_regs_s* const SP_regs = (struct SP_regs_s *)0xA4040000;

#define SP_DMA_IMEM 0x04001000
#define SP_STATUS_GET_IS_BUSY   0x080



/**
 * Called if the RSP hits a break instruction.
 */
static void onRSPException() {
    data_cache_hit_invalidate(&rspInterface, sizeof(RspInterface));
    printSegmentToFrame(rootState.Frame, "RSP Exception Raised - dumping rspInterface", (byte*) &rspInterface);
}

/**
 * Extend interface
 */
void* allocRspInterface(size_t size) {
    if (size != sizeof(RspInterface)) {
        // Raise some sort of exception.

        return null;
    }

    return (void*) &rspInterface;
}

/**
 * DMAs a fixed set of instructions to the RSP ready to be run when we call run_ucode()
 */
s8 prepareMicrocode(const Microcode code) {
    register_SP_handler(&onRSPException);
    set_SP_interrupt(1);

    unsigned long size = 0;
    switch (code) {
        case UCODE_DMG_PPU:
            size = (unsigned long) &ppuDMG_data_size;
            load_data((void*)&ppuDMG_data_start, size);

            size = (unsigned long)&ppuDMG_code_size;
            load_ucode((void*)&ppuDMG_code_start, size);
            break;
        case UCODE_GBC_PPU:
        case UCODE_SGB_PPU:
            return RSP_ERR_UNIMPLEMENTED_UCODE;
        default:
            return RSP_ERR_INVALID_UCODE;
    }

    SP_regs->status = SP_STATUS_BUSY_OFF;

    return RSP_SUCCESS;
}

/**
 * Signals to the RSP whether there is more data to be processed.
 * @param value to set.
 */
void setDataReady(bool value) {
    SP_regs->status = value ? SP_DATA_READY : SP_DATA_PENDING;
}

/**
 * Gets whether the RSP has more data to be processed.
 */
bool getDataReady() {
    return SP_regs->status & SP_STATUS_GET_IS_READY;
}

/**
 * Sets which frame buffer the RSP should be targetting.
 * @param id identifies the buffer (1 or 2)
 */
void setFrameBufferId(display_context_t id) {
    SP_regs->status = id == 1 ? SP_BUFFER_1 : SP_BUFFER_2;
}

/**
 * Sets the RSP halt status so that it stops executing while we reload IMEM/DMEM
 */
void haltRsp() {
    SP_regs->status = SP_STATUS_HALT_ON | SP_STATUS_BUSY_OFF;
}

/**
 * Checks the RSP interface to determine what it's doing.
 * @returns True if the RSP is working, false if it's idle.
 */
bool isRspBusy() {
    return SP_regs->status & SP_STATUS_GET_IS_BUSY;
}

/**
 * Kicks off the RSP to render the next frame.
 * @param inBuffer gameboy screen buffer pixels are picked up by the RSP from here.
 * @param outBuffer after RSP generates a texture, it will DMA it back into DRAM at this address.
 * @param screen size and position of the textures drawn by the RSP.
 * @param isColour if true, inBuffer words represent 2 bit DMG pixels.  Otherwise they are 16bit GBC pixels
 */
void renderFrame(uintptr_t inBuffer, uintptr_t outBuffer, Rectangle* screen, bool isColour) {
}
