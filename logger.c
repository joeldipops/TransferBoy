#include "logger.h"
#include "constants.h"
#include <stdarg.h>
#include <libdragon.h>

void logInfo(const string text, ...) {
    va_list args;
    va_start(args, text);

    static display_context_t frame = null;
    string output = "";

    vsprintf(output, text, args);
    va_end(args);

    while (!(frame = display_lock()));

    graphics_fill_screen(frame, GLOBAL_BACKGROUND_COLOUR);
    graphics_set_color(GLOBAL_TEXT_COLOUR, 0x0);

    graphics_draw_text(frame, HORIZONTAL_MARGIN, VERTICAL_MARGIN, output);

    display_show(frame);
}

