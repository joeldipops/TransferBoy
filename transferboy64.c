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

void initialiseSubsystems() {
    init_interrupts();
    controller_init();
    display_init(RESOLUTION_640x480, DEPTH_16_BPP, 2, GAMMA_NONE, ANTIALIAS_OFF);
    rdp_init();
}

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

void generateState(RootState* state) {
    state->PlayerCount = 1;
    state->Players[0].SelectedBorder = BorderNone;
    state->Players[0].AudioEnabled = true;
    state->Players[0].ActiveMode = Init;
    initialiseButtonMap(state->Players[0].ButtonMap);
}

void mainLoop(RootState* state) {
    bool allQuit = false;

    while(!(state->Frame = display_lock()))
    state->RequiresRepaint = true;

    while (!allQuit) {
        allQuit = true;

        controller_scan();
        state->ControllerState = get_keys_pressed();

        for (int i = 0; i < state->PlayerCount; i++) {
            switch(state->Players[i].ActiveMode) {
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
                default: break;
            }
        }

        if (state->RequiresRepaint) {
            while(!(state->Frame = display_lock()));
        }

        for (int i = 0; i < state->PlayerCount; i++) {
            switch(state->Players[i].ActiveMode) {
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

/**
 * Main loop.
 */
 /*
int main(void)
{
    setColours();
    init_interrupts();
    display_init(res, bit, 2, GAMMA_NONE, ANTIALIAS_RESAMPLE);
    controller_init();

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
*/
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
