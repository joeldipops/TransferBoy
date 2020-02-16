#include "global.h"
#include "rsp.h"

#include "logger.h"

typedef struct { // Extends ppuInterface
    u32 IsBusy;
    union {
        u32 Settings[6];
        struct {
            uintptr_t VRamAddress;
            uintptr_t HRamAddress;
            uintptr_t OAMAddress;
            uintptr_t OutBuffer;
            Rectangle Screen;
        };
    };
} PpuInterface;

PpuInterface* ppuInterface;

void ppuInit(PlayerState* state) {
    Rectangle screen = {};
    getScreenPosition(0, &screen);

    ppuInterface = allocRspInterface(sizeof(PpuInterface));

    ppuInterface->Screen = (Rectangle) { screen.Left, screen.Top, 320, 2 };

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
        Rectangle screen;
        getScreenPosition(0, &screen);
        ppuInterface->Screen.Top = screen.Top + (state->EmulationState.CurrentLine * (screen.Height / GB_LCD_HEIGHT));
        //logAndPauseFrame(2, "LY Val = %08x", state->EmulationState.BackgroundPalette);
        data_cache_hit_writeback(ppuInterface, sizeof(PpuInterface));
        run_ucode();
    }
}