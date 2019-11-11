#include <libdragon.h>
#include <stdlib.h>
#include "core.h"
#include "logger.h"

extern const char rsp_code_start __attribute((section(".data")));
extern const char rsp_code_end __attribute((section(".data")));
extern const char rsp_code_size __attribute((section(".data")));

/********************************
 * 
 * The below defintions (up to SP_regs) were copied out of libdragon (regsinternal.h & rsp.h)
 */
 
#define RSP_DMA_BUSY ( 1 << 2 )
#define RSP_IO_BUSY ( 1 << 4 )
#define RSP_DMEM_ADDRESS 0x04000000

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

static volatile struct SP_regs_s * const SP_regs = (struct SP_regs_s *)0xa4040000;

/**
 * @brief Wait until the SI is finished with a DMA request
 */
static void DMAWait(void) {
    while (SP_regs->status & (RSP_DMA_BUSY | RSP_IO_BUSY));
}

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

/**
 * DMAs a block of memory to DMEM where it can be accessed by the RSP
 * @param source Address of memory to transfer.
 * @param size size of memory to transfer.
 */
void rspDMAWrite(void* source, size_t size) {
    disable_interrupts();
    DMAWait();

    SP_regs->DRAM_addr = source;
    MEMORY_BARRIER();
    SP_regs->RSP_addr = (void*)RSP_DMEM_ADDRESS;
    MEMORY_BARRIER();
    SP_regs->rsp_read_length = size - 1;
    MEMORY_BARRIER();

    DMAWait();
    enable_interrupts();
}

/**
 * DMAs a block of memory from RSP DMEM back to main memory.
 * @param destination address of memory to transfer to.
 * @param size size of memory to transfer.
 */
void rspDMARead(void* destination, size_t size) {
    disable_interrupts();
    DMAWait();

    SP_regs->DRAM_addr = destination;
    MEMORY_BARRIER();
    SP_regs->RSP_addr = (void*)RSP_DMEM_ADDRESS;
    MEMORY_BARRIER();
    SP_regs->rsp_write_length = size - 1;
    MEMORY_BARRIER();
    DMAWait();

    enable_interrupts();
}