#include "logger.h"
#include "utils.h"
#include <stdarg.h>
#include <libdragon.h>

/**
 * Prints the pre-sprintf'd text to the display.
 * @private
 * @param text The text ready to be displayed.
 */
void printLog(const string text) {
    static display_context_t frame = null;

    while (!(frame = display_lock()));

    graphics_fill_screen(frame, GLOBAL_BACKGROUND_COLOUR);
    graphics_set_color(GLOBAL_TEXT_COLOUR, 0x0);

    graphics_draw_text(frame, HORIZONTAL_MARGIN, VERTICAL_MARGIN, text);

    display_show(frame);
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
    printLog(output);

    va_end(args);

    bool isPaused = true;
    while(isPaused) {
        controller_scan();
        struct controller_data input = get_keys_pressed();
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

    string output = "";

    vsprintf(output, text, args);
    va_end(args);

    printLog(output);
}

