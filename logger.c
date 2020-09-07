#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "config.h"
#include "state.h"
#include "logger.h"
#include "core.h"
#include "eeprom.h"
#include <libdragon.h>

const byte VERTICAL_MARGIN = 30;
const byte HORIZONTAL_MARGIN = 30;

/**
 * Prints the pre-sprintf'd text to the display.
 * @private
 * @param text The text ready to be displayed.
 */
static void printLog(const string text, display_context_t frame) {
    if (!frame) {
        while(!(frame = display_lock()));
    }

    graphics_draw_box(frame, 0, 0, 640, 40, GLOBAL_BACKGROUND_COLOUR);
    graphics_set_color(GLOBAL_TEXT_COLOUR, 0x0);

    graphics_draw_text(frame, HORIZONTAL_MARGIN, VERTICAL_MARGIN, text);

    display_show(frame);
}

/**
 * Print the next 32 bytes of memory from a given address and wait for "start".
 * @param caption Describes what memory is being displayed.
 * @param start the starting memory address.
 * @param frame display buffer to print to.
 */
void printSegmentToFrame(display_context_t frame, const string caption, const byte* start) {
    if (!frame) {
        while(!(frame = display_lock()));
    }    

    graphics_fill_screen(frame, GLOBAL_BACKGROUND_COLOUR);
    graphics_draw_text(frame, 30, 10, caption);
    
    string text = "";
    sprintf(
        text,
        "%02x %02x %02x %02x %02x %02x %02x %02x  %02x %02x %02x %02x %02x %02x %02x %02x",
        *start, *(start + 1), *(start + 2), *(start + 3), *(start + 4), *(start + 5), *(start + 6), *(start + 7),
        *(start + 8), *(start + 9), *(start + 10), *(start + 11), *(start + 12), *(start + 13), *(start + 14), *(start + 15)
    );
    graphics_draw_text(frame, 30, 20, text);
    start += 16;
    sprintf(
        text,
        "%02x %02x %02x %02x %02x %02x %02x %02x  %02x %02x %02x %02x %02x %02x %02x %02x",
        *start, *(start + 1), *(start + 2), *(start + 3), *(start + 4), *(start + 5), *(start + 6), *(start + 7),
        *(start + 8), *(start + 9), *(start + 10), *(start + 11), *(start + 12), *(start + 13), *(start + 14), *(start + 15)
    );
    graphics_draw_text(frame, 30, 30, text);
    start += 16;
    sprintf(
        text,
        "%02x %02x %02x %02x %02x %02x %02x %02x  %02x %02x %02x %02x %02x %02x %02x %02x",
        *start, *(start + 1), *(start + 2), *(start + 3), *(start + 4), *(start + 5), *(start + 6), *(start + 7),
        *(start + 8), *(start + 9), *(start + 10), *(start + 11), *(start + 12), *(start + 13), *(start + 14), *(start + 15)
    );
    graphics_draw_text(frame, 30, 40, text);
    start += 16;
    sprintf(
        text,
        "%02x %02x %02x %02x %02x %02x %02x %02x  %02x %02x %02x %02x %02x %02x %02x %02x",
        *start, *(start + 1), *(start + 2), *(start + 3), *(start + 4), *(start + 5), *(start + 6), *(start + 7),
        *(start + 8), *(start + 9), *(start + 10), *(start + 11), *(start + 12), *(start + 13), *(start + 14), *(start + 15)
    );
    graphics_draw_text(frame, 30, 50, text);

    bool isPaused = true;
    while(isPaused) {
        controller_scan();
        N64ControllerState input = get_keys_pressed();
        if (input.c[0].start) {
            isPaused = false;
        }
    } 
}


/**
 * Throws a line of text up on to the screen a la printf and then stalls execution so you can read it.
 * @param text The text or format string.
 * @param ... Parameters for the format string.
 */
void logAndPause(const string text, ...) {
    va_list args;
    va_start(args, text);

    string output = "";
    vsprintf(output, text, args);
    printLog(output, null);

    va_end(args);

    bool isPaused = true;
    while(isPaused) {
        controller_scan();
        N64ControllerState input = get_keys_pressed();
        if (input.c[0].start) {
            isPaused = false;
        }
    }
}

/**
 * Displays the current state of the Gameboy emulator registers.
 * @param s The emulator state structure.
 */
void printRegisters(GbState* s) {
    logAndPauseFrame(
        0,
        "A=%02x F=%02x B=%02x C=%02x D=%02x E=%02x H=%02x L=%02x Z=%d N=%d HF=%d C=%d sp=%04x pc=%04x",
        s->reg8.A, s->reg8.F, s->reg8.B, s->reg8.C,
        s->reg8.D, s->reg8.E, s->reg8.H, s->reg8.L,
        s->flags.ZF, s->flags.NF, s->flags.HF, s->flags.CF,
        s->sp, PC//, s->LcdControl, s->LcdStatus
    );
}

void logAndPauseFrame(display_context_t frame, const string text, ...) {
    va_list args;
    va_start(args, text);

    string output = "";
    vsprintf(output, text, args);
    printLog(output, frame);

    va_end(args);

    bool isPaused = true;
    while(isPaused) {
        controller_scan();
        N64ControllerState input = get_keys_pressed();
        if (input.c[0].start) {
            isPaused = false;
        }
    }
}

/**
 * Throws a line of text up on to the screen a la printf.
 * @param text The text or format string.
 * @param ... Parameters for the format string.
 */
void logInfo(const string text, ... ) {
    va_list args;
    va_start(args, text);

    string formatted = "";
    string tmp = "";

    vsprintf(formatted, text, args);
    va_end(args);

    byte blockNumber = getEepromCursorPosition();

    // Since it's a log file, whack a new line on the end.
    // TODO - timestamp.
    sprintf(tmp, "%s\n", formatted);
    strcpy(formatted, tmp);

    ByteArray textString;
    textString.Size = strlen(formatted);

    textString.Data = calloc(1, textString.Size + 1);

    memcpy(textString.Data, formatted, textString.Size);

    if(writeToEeprom(blockNumber, &textString)) {
        logAndPause("logging failed.  Message was: %s", formatted);
    }

    free(textString.Data);
    textString.Data = null;
}

