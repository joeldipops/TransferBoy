#include "rsp.h"
#include "logger.h"

typedef struct { // Extends ppuInterface
    u32 IsBusy;
    union {
        u32 Settings[5];
        struct {
            uintptr_t VRamAddress;
            uintptr_t HRamAddress;
            uintptr_t OAMAddress;
            uintptr_t OutBuffer;
            Rectangle Screen;
            Rectangle Line;
        };
    };
} PpuInterface;

volatile PpuInterface ppuInterface __attribute__ ((section (".rspInterface"))) __attribute__ ((__used__));

void ppuInit(PlayerState* state) {
    Rectangle screen = {};
    getScreenPosition(0, &screen);

    ppuInterface.Screen = screen;
    ppuInterface.Line = (Rectangle) { screen.Left, screen.Top, 320, 4 };

    ppuInterface.IsBusy = false;
    ppuInterface.VRamAddress = (uintptr_t) state->EmulationState.VRAM;
    ppuInterface.HRamAddress = (uintptr_t) state->EmulationState.HRAM;
    ppuInterface.OAMAddress = (uintptr_t) state->EmulationState.OAM;
    ppuInterface.OutBuffer = (uintptr_t)state->EmulationState.TextureBuffer;

    data_cache_hit_writeback(&ppuInterface, sizeof(PpuInterface));

    prepareMicrocode(UCODE_DMG_PPU);
}

void ppuStep(PlayerState* state) {
    // Let the RSP finish its current job & skip this one.
    //if (isRspBusy()) {
        //return;
    //}

    haltRsp();
    ppuInterface.IsBusy = true;
    logAndPauseFrame(2, "LCDC Val = %08x", state->EmulationState.LcdControl);
    data_cache_hit_writeback(&ppuInterface, sizeof(PpuInterface));
    run_ucode();
}