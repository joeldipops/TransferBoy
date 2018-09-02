#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <stdint.h>
#include <unistd.h>

#include "utils.h"
#include "options.h"
#include "menu.h"
#include "game.h"
#include "logger.h"
#include <libdragon.h>

static resolution_t res = RESOLUTION_320x240;
static bitdepth_t bit = DEPTH_32_BPP;

/**
 * Sets oft-use colours since we can't assign graphics_make_color to a constant.
 * @private
 */
void setColours() {
    GLOBAL_BACKGROUND_COLOUR = graphics_make_color(200, 200, 220, 255);
    GLOBAL_TEXT_COLOUR = graphics_make_color(0, 0, 0, 255);
}

/**
 * Main loop.
 */
int main(void)
{
    setColours();
    init_interrupts();
    display_init(res, bit, 2, GAMMA_NONE, ANTIALIAS_RESAMPLE);
    console_init();
    controller_init();
    console_set_render_mode(1);

    // initialize options hash
    OptionsHash * options = malloc(sizeof *options);
    initialiseOptions(options);

    bool isDone = false;

    while(!isDone) {
        MenuState menuState = menuLoop();

        switch(menuState) {
            case MenuStateExit:
                isDone = true;
                break;
            case MenuStateStart:
                gameLoop(options);
                break;
            case MenuStateOptions:
                optionsLoop(options);
            default: break;
        }
    }
}

// cheating because the makefile doesn't work.
void _exit(int status)
{
  (void) status;
  abort();
}

#include "utils.c"
#include "game.c"
#include "menu.c"
#include "options.c"
#include "tpakio.c"
#include "logger.c"
#include "include/libgbpak.c"
#include "include/gbc_bundle.c"
