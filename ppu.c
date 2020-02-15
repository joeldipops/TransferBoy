#include "rsp.h"
#include "logger.h"

typedef struct { // Extends ppuInterface
    u32 IsBusy;
    union {
        u32 Settings[8];
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

PpuInterface* ppuInterface;

void ppuInit(PlayerState* state) {
    Rectangle screen = {};
    getScreenPosition(0, &screen);

    ppuInterface = allocRspInterface(sizeof(PpuInterface));

    ppuInterface->Screen = screen;
    ppuInterface->Line = (Rectangle) { screen.Left, screen.Top, 320, 4 };

    ppuInterface->IsBusy = false;
    ppuInterface->VRamAddress = (uintptr_t) state->EmulationState.VRAM;
    ppuInterface->HRamAddress = (uintptr_t) state->EmulationState.HRAM;
    ppuInterface->OAMAddress = (uintptr_t) state->EmulationState.OAM;
    ppuInterface->OutBuffer = (uintptr_t)state->EmulationState.TextureBuffer;

    data_cache_hit_writeback(ppuInterface, sizeof(PpuInterface));

    prepareMicrocode(UCODE_DMG_PPU);
}

void ppuStep(PlayerState* state) {
    if (state->EmulationState.lcd_entered_hblank) {
        // Let the RSP finish its current job & skip this one.
        if (isRspBusy()) {
            return;
        }

        haltRsp();
        ppuInterface->IsBusy = true;
        //logAndPauseFrame(2, "LY Val = %08x", state->EmulationState.BackgroundPalette);
        data_cache_hit_writeback(ppuInterface, sizeof(PpuInterface));
        run_ucode();
    }
}