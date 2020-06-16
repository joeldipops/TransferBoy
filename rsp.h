#ifndef RSP_INCLUDED
#define RSP_INCLUDED

#include <libdragon.h>
#include "screen.h"

typedef enum {
    RSP_SUCCESS = 0,
    RSP_ERR_INVALID_UCODE = -127,
    RSP_ERR_UNIMPLEMENTED_UCODE
} RspError;

typedef struct {
    u32 Settings[8];
} RspInterface;

typedef enum { NONE, UCODE_DMG_PPU, UCODE_GBC_PPU, UCODE_SGB_PPU, FRAME_RENDERER } Microcode;

typedef void (*RspEventHandler)();

/**
 * DMAs a fixed set of instructions to the RSP ready to be run when we call run_ucode()
 * @returns Error Code
 **  0 Success
 ** -1 Invalid microcode
 */
s8 prepareMicrocode(const Microcode code);

/**
 * Sets the RSP halt status so that it stops executing while we reload IMEM/DMEM
 */
void haltRsp();

/**
 * Extend interface
 */
void* allocRspInterface(size_t size);

/**
 * Checks the RSP interface to determine what it's doing.
 * @returns True if the RSP is working, false if it's idle.
 */
bool isRspBusy();

/**
 * Signals to the RSP whether there is more data to be processed.
 * @param value to set.
 */
void setDataReady(bool value);

/**
 * Sets which frame buffer the RSP should be targetting.
 * @param id identifies the buffer (1 or 2)
 */
void setFrameBufferId(display_context_t id);

/**
 * Gets whether the RSP has more data to be processed.
 */
bool getDataReady();


#endif