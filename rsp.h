#ifndef RSP_INCLUDED
#define RSP_INCLUDED

#include <libdragon.h>
#include "screen.h"

typedef enum {
    RSP_SUCCESS = 0,
    RSP_ERR_INVALID_UCODE = -127,
    RSP_ERR_UNIMPLEMENTED_UCODE
} RspError;

typedef enum { NONE, UCODE_RENDERER, UCODE_DMG_PPU, UCODE_GBC_PPU, UCODE_SGB_PPU } Microcode;

typedef void (*RspEventHandler)();

/**
 * Kicks off the RSP to render the next frame.
 * @param inBuffer gameboy screen buffer pixels are picked up by the RSP from here.
 * @param outBuffer after RSP generates a texture, it will DMA it back into DRAM at this address.
 * @param screen size and position of the textures drawn by the RSP.
 * @param isColour if true, inBuffer words represent 2 bit DMG pixels.  Otherwise they are 16bit GBC pixels
 */
void renderFrame(uintptr_t inBuffer, uintptr_t outBuffer, Rectangle* screen, bool isColour);

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
 * Checks the RSP interface to determine what it's doing.
 * @returns True if the RSP is working, false if it's idle.
 */
bool isRspBusy();


#endif