#include "logger.h"
#include "utils.h"
#include <stdarg.h>
#include <libdragon.h>

void logAndPause(const string format, ...) {
    va_list args;
    va_start(args, format);

    logInfo(format, args);

    va_end(args);

    while(true) {;}
}

void logInfo(const string format, ...) {
    va_list args;
    va_start(args, format);

    static display_context_t frame = null;
    string output = "";

    vsprintf(output, format, args);
    va_end(args);

    while (!(frame = display_lock()));

    graphics_fill_screen(frame, GLOBAL_BACKGROUND_COLOUR);
    graphics_set_color(GLOBAL_TEXT_COLOUR, 0x0);

    graphics_draw_text(frame, HORIZONTAL_MARGIN, VERTICAL_MARGIN, output);

    display_show(frame);
}

