#include "core.h"
#include "play.h"
#include "menu.h"
#include "options.h"
#include "logger.h"
#include "text.h"
#include "tpakio.h"
#include "init.h"
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
    init_interrupts();
    controller_init();
    display_init(RESOLUTION_640x480, DEPTH_16_BPP, 2, GAMMA_NONE, ANTIALIAS_OFF);
    rdp_init();
}

/**
 * Sets up initial configuration for N64 - Gameboy button map.
 * @out buttonMap map of N64 to Gameboy buttons.
 */
void initialiseButtonMap(GbButton* map) {
    GbButton buttons[16];
    buttons[NoButton] = GbNoButton;
    buttons[A] = GbA;
    buttons[B] = GbB;
    buttons[Up] = GbUp;
    buttons[Down] = GbDown;
    buttons[Left] = GbLeft;
    buttons[Right] = GbRight;

    // Start, Select and SystemMenu should be configurable
    buttons[Start] = GbStart;
    buttons[R] = GbSelect;
    buttons[L] = GbSystemMenu;

    // TODO: Implement stick control.
    buttons[Stick] = GbNoButton;

    buttons[Z] = GbNoButton;
    buttons[CUp] = GbNoButton;
    buttons[CDown] = GbNoButton;
    buttons[CLeft] = GbNoButton;
    buttons[CRight] = GbNoButton;

    memcpy(map, &buttons, sizeof(GbButton) * 16);
}

/**
 * Sets up initial state object that will be used throughout the program.
 * @out state the state object.
 */
void generateState(RootState* state) {
    state->PlayerCount = 1;
    state->Players[0].SelectedBorder = BorderNone;
    state->Players[0].AudioEnabled = true;
    state->Players[0].ActiveMode = Init;
    initialiseButtonMap(state->Players[0].ButtonMap);
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

        Mode modes[4] = {0, 0, 0, 0};

        for (unsigned char i = 0; i < state->PlayerCount; i++) {
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

    RootState state;
    generateState(&state);
    mainLoop(&state);



    //display_close();
}


// cheating because the makefile doesn't work.
void _exit(int status)
{
  (void) status;
  abort();
}

#include "init.c"
#include "play.c"
#include "menu.c"
#include "options.c"
#include "tpakio.c"
#include "logger.c"
#include "text.c"
#include "controller.c"
#include "include/libgbpak.c"
#include "include/gbc_bundle.c"
