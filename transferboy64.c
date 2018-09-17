#include "core.h"
#include "play.h"
#include "menu.h"
#include "options.h"
#include "logger.h"
#include "text.h"
#include "tpakio.h"
#include "init.h"
#include "controller.h"
#include "state.h"
#include "screen.h"
#include <string.h>
#include <stdlib.h>
#include <libdragon.h>

#include <pthread.h>

/**
 * Sets oft-use colours since we can't assign graphics_make_color to a constant.
 * @private
 */
void setGlobalConstants() {
    GLOBAL_BACKGROUND_COLOUR = graphics_make_color(220, 220, 150, 255);
    BLANK_SCREEN_COLOUR = graphics_make_color(0xBB, 0xBB, 0xBB, 255);
    GLOBAL_TEXT_COLOUR = graphics_make_color(0, 0, 0, 255);
}

/**
 * Inititalises all required libdragon subsystems.
 */
void initialiseSubsystems() {
    getMemoryLimit();
    init_interrupts();
    controller_init();
    display_init(RESOLUTION_640x480, DEPTH_16_BPP, 2, GAMMA_NONE, ANTIALIAS_OFF);
    rdp_init();
    graphics_set_color(GLOBAL_TEXT_COLOUR, 0x0);
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

    while (!allQuit) {
        allQuit = true;

        controller_scan();
        state->ControllerState = get_keys_pressed();

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
            while(!(state->Frame = display_lock()));
        }

        for (int i = 0; i < state->PlayerCount; i++) {
            switch(modes[i]) {
                case Init:
                    initDraw(state, i);
                    break;
                case Play:
                    playDraw(state, i);
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

        if (state->RequiresRepaint) {
            display_show(state->Frame);
            state->Frame = 0;
            state->RequiresRepaint = false;
        }
    }
}

/**
 * Initialise all then enter main loop.
 */
int main(void) {
    initialiseSubsystems();
    setGlobalConstants();
    flushScreen();
    RootState state;
    generateState(&state);
    mainLoop(&state);

    display_close();
}


// cheating because the makefile doesn't work.
void _exit(int status)
{
  (void) status;
  abort();
}

#include "core.c"
#include "init.c"
#include "play.c"
#include "menu.c"
#include "options.c"
#include "tpakio.c"
#include "logger.c"
#include "text.c"
#include "controller.c"
#include "screen.c"
#include "state.c"
#include "eeprom.c"
#include "include/libgbpak.c"
#include "include/gbc_bundle.c"
