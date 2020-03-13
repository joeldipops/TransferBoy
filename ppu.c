#include "global.h"
#include "rsp.h"
#include "hwdefs.h"
#include "config.h"

#include "logger.h"

typedef struct { // Extends ppuInterface
    u32 Status;
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

    ppuInterface->VRamAddress = (uintptr_t) state->EmulationState.VRAM;
    ppuInterface->HRamAddress = (uintptr_t) state->EmulationState.HRAM;
    ppuInterface->OAMAddress = (uintptr_t) state->EmulationState.OAM;
    ppuInterface->OutBuffer = (uintptr_t)state->EmulationState.TextureBuffer;

    data_cache_hit_writeback(ppuInterface, sizeof(PpuInterface));

    prepareMicrocode(UCODE_DMG_PPU);

    setDataReady(false);
    run_ucode();
}

void ppuStep(PlayerState* state) {
    if (state->EmulationState.lcd_entered_hblank) {

        if (state->EmulationState.CurrentLine >= GB_LCD_HEIGHT) { /* VBlank */
            return;
        }

        if (FRAMES_TO_SKIP && ((state->Meta.FrameCount + 1) % (FRAMES_TO_SKIP + 1))) {
            return;
        }

        // Let the RSP finish its current job.
        while(isRspBusy());

        Rectangle screen;
        getScreenPosition(0, &screen);
        ppuInterface->Screen.Top = screen.Top + (state->EmulationState.CurrentLine * (screen.Height / GB_LCD_HEIGHT));

        data_cache_hit_writeback(ppuInterface, sizeof(PpuInterface));
        data_cache_hit_writeback(state->EmulationState.OAM, OAM_SIZE);
        data_cache_hit_writeback(state->EmulationState.HRAM, HRAM_SIZE);
        if (state->EmulationState.isVramDirty) {
            data_cache_hit_writeback(state->EmulationState.VRAM, VRAM_BANK_SIZE);
            state->EmulationState.isVramDirty = false;
        }

        setDataReady(true);
    }
}