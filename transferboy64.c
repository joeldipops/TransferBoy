#include "config.h"
#include "core.h"
#include "play.h"
#include "menu.h"
#include "options.h"
#include "logger.h"
#include "text.h"
#include "init.h"
#include "controller.h"
#include "state.h"
#include "screen.h"
#include "resources.h"
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <libdragon.h>

/**
 * Sets oft-use colours since we can't assign graphics_make_color to a constant.
 * @private
 */
void setGlobalConstants() {
    GLOBAL_BACKGROUND_COLOUR = graphics_make_color(220, 220, 150, 255);
    GLOBAL_TEXT_COLOUR = graphics_make_color(255, 0, 0, 255);
    SELECTED_MENU_ITEM_COLOUR = graphics_make_color(0x28, 0x6E, 0x31, 255);
    SELECTED_OPTIONS_ITEM_COLOUR = graphics_make_color(0xCF, 0x48, 0x2C, 255);
}

/**
 * Inititalises all required libdragon subsystems.
 */
void initialiseSubsystems() {
    getMemoryLimit();
    init_interrupts();
    timer_init();
    controller_init();
    dfs_init(DFS_DEFAULT_LOCATION);
    display_init(RESOLUTION_640x480, DEPTH_16_BPP, 2, GAMMA_NONE, ANTIALIAS_OFF);
    rdp_init();
    graphics_set_color(GLOBAL_TEXT_COLOUR, 0x0);
    audio_init(AUDIO_SAMPLE_RATE, 4);
    initResources();
    initText();
}

/**
 * Sets up initial state object that will be used throughout the program.
 * @out state the state object.
 */
void generateState(RootState* state) {
    state->PlayerCount = 1;
    generatePlayerState(&state->Players[0]);
}

/**
 * The mainloop of the program.  Has three phases and executes for all players at each phase
 * phase 1 read input
 * phase 2 carry out program logic
 * phase 3 paint the screen if required.
 * todo - audio.
 */
void mainLoop(RootState* state) {
    bool allQuit = false;
    state->RequiresRepaint = true;
    state->RequiresControllerRead = true;
    uLong iterations = 0;

    while(!(state->Frame = display_lock()));

    while (!allQuit) {
        allQuit = true;

        // Read controller about once per frame.
        if (state->RequiresControllerRead || iterations > GB_LCD_FRAME_CLKS / 2) {
            controller_scan();
            state->KeysPressed = get_keys_pressed();
            state->KeysReleased = get_keys_up();
            state->RequiresControllerRead = false;
            iterations = 0;
        } else {
            iterations++;
        }

        Mode modes[MAX_PLAYERS] = {0, 0};

        // Cache this as it may change in the below calls.
        byte playerCount = state->PlayerCount;

        for (byte i = 0; i < playerCount; i++) {
            if (allQuit && state->Players[i].ActiveMode != Quit) {
                allQuit = false;
            }

            modes[i] = state->Players[i].ActiveMode;

            switch(modes[i]) {
                case Quit:
                    allQuit &= true;
                    break;
                case Init:
                    initLogic(state, i);
                    break;
                case Play:
                    playLogic(state, i);
                    break;
                case Menu:
                    menuLogic(state, i);
                    break;
                case Options:
                    optionsLogic(state, i);
                    break;
                default: break;
            }
        }

        if (state->RequiresRepaint) {
            state->RequiresRepaint = false;

            for (byte i = 0; i < state->PlayerCount; i++) {
                switch(modes[i]) {
                    case Init:
                        initDraw(state, i);
                        break;
                    case Play:
                        playDraw(state, i);
                        playAfter(state, i);
                        break;
                    case Menu:
                        menuDraw(state, i);
                        break;
                    case Options:
                        optionsDraw(state, i);
                        break;
                    default: break;
                }
            }
            display_show(state->Frame);
        }
    }
}

/**
 * Initialise all then enter main loop.
 */
int main(void) {
    initialiseSubsystems();
    setGlobalConstants();
    RootState state;
    generateState(&state);
    flushScreen(&state);

    mainLoop(&state);

    freeText();
    freeResources();
    display_close();
    rdp_close();
    audio_close();
    return 0;
}


#include "core.c"
#include "resources.c"
#include "init.c"
#include "play.c"
#include "menu.c"
#include "options.c"
#include "logger.c"
#include "text.c"
#include "controller.c"
#include "screen.c"
#include "state.c"
#include "eeprom.c"
#include "link.c"
#include "sound.c"
#include "tpakio.c"
#include "superGameboy.c"
#include "include/gbc_bundle.c"