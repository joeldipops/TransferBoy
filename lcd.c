#include <stdlib.h>
#include <string.h>

#include "core.h"
#include "state.h"
#include "config.h"
#include "lcd.h"
#include "hwdefs.h"
#include "ppu.h"

/**
 *
 * @return error code
 **  0 - Successful
 ** -1 - Out of memory.
 */
int lcd_init(GbState* s) {
    s->TextureBuffer = calloc(sizeof(byte), 0x1000); // Size of DMEM
    return 0;
}

void lcd_step(PlayerState* state) {
    /* The LCD goes through several states.
     * 0 = H-Blank, 1 = V-Blank, 2 = reading OAM, 3 = line render
     * For the first 144 (visible) lines the hardware first reads the OAM
     * (sprite data), then goes through each pixel on the line, and finally
     * H-Blanks. For the last few lines the screen is in V-Blank, where VRAM can
     * be freely accessed and we don't have the other 3 modes per line.
     * So the cycle goes like: 2330002330002330002330001111..1111233000...
     *                         OBBHHHOBBHHHOBBHHHOBBHHHVVVV..VVVVOBBHHH...
     * The entire cycle takes 70224 clks. (so that's about 60FPS)
     * H-Blank takes about 201-207 cycles. VBlank 4560 clks.
     * OAM reading takes about 77-83 and line rendering about 169-175 clks.
     */

    GbState* s = &state->EmulationState;

    s->lcd_entered_hblank = 0;
    s->lcd_entered_vblank = 0;

    s->io_lcd_mode_cycles_left -= s->last_op_cycles;

    if (s->io_lcd_mode_cycles_left < 0) {
        switch (s->LcdStatus & 3) {
            case 0: // H-Blank
                if (s->CurrentLine == 143) { // Go into V-Blank (1)
                    s->LcdStatus = (s->LcdStatus & 0xfc) | 1;
                    s->io_lcd_mode_cycles_left = GB_LCD_MODE_1_CLKS;
                    s->InterruptFlags |= 1 << 0;
                    s->lcd_entered_vblank = 1;
                } else { // Back into OAM (2)
                    s->LcdStatus = (s->LcdStatus & 0xfc) | 2;
                    s->io_lcd_mode_cycles_left = GB_LCD_MODE_2_CLKS;
                }
                s->CurrentLine = (s->CurrentLine + 1) % (GB_LCD_LY_MAX + 1);
                s->LcdStatus = (s->LcdStatus & 0xfb) | (s->CurrentLine == s->NextInterruptLine);

                // We incremented line, check LY=LYC and set interrupt if needed.
                if (s->LcdStatus & (1 << 6) && s->CurrentLine == s->NextInterruptLine) {
                    s->InterruptFlags |= 1 << 1;
                }
                break;
            case 1: // VBlank, Back to OAM (2)
                s->LcdStatus = (s->LcdStatus & 0xfc) | 2;
                s->io_lcd_mode_cycles_left = GB_LCD_MODE_2_CLKS;
                break;
            case 2: // OAM, onto line drawing (OAM+VRAM busy) (3)
                s->LcdStatus = (s->LcdStatus & 0xfc) | 3;
                s->io_lcd_mode_cycles_left = GB_LCD_MODE_3_CLKS;
                break;
            case 3: // Line render (OAM+VRAM), let's H-Blank (0)
                s->LcdStatus = (s->LcdStatus & 0xfc) | 0;
                s->io_lcd_mode_cycles_left = GB_LCD_MODE_0_CLKS;
                s->lcd_entered_hblank = 1;
                break;
        }

        if (s->IsInDoubleSpeedMode && s->Cartridge.IsGbcSupported) {
            s->io_lcd_mode_cycles_left *= 2;
        }

        // We switched mode, trigger interrupt if requested.
        u8 newmode = s->LcdStatus & 3;
        if (s->LcdStatus & (1 << 5) && newmode == 2) { // OAM (2) int
            s->InterruptFlags |= 1 << 1;
        } else if (s->LcdStatus & (1 << 4) && newmode == 1) { // V-Blank (1) int
            s->InterruptFlags |= 1 << 1;
        } else if (s->LcdStatus & (1 << 3) && newmode == 0) { // H-Blank (0) int
            s->InterruptFlags |= 1 << 1;
        }
    }

    s->IsCurrentLineLYC = (s->CurrentLine == s->NextInterruptLine);

    if (s->lcd_entered_hblank) {
        ppuStep(state);
    }
}