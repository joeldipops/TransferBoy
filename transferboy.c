#include "config.h"
#include "core.h"
#include "play.h"
#include "menu.h"
#include "options.h"
#include "text.h"
#include "init.h"
#include "controller.h"
#include "state.h"
#include "screen.h"
#include "resources.h"
#include "hwdefs.h"
#include "rsp.h"
#include "fps.h"
#include "debug.h"
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <libdragon.h>

#include "rsp.h"

/**
 * Sets oft-use colours since we can't assign graphics_make_color to a constant.
 * @private
 */
static void setGlobalConstants() {
    GLOBAL_BACKGROUND_COLOUR = graphics_make_color(220, 220, 150, 255);
    GLOBAL_TEXT_COLOUR = graphics_make_color(255, 0, 0, 255);
    SELECTED_MENU_ITEM_COLOUR = graphics_make_color(0x28, 0x6E, 0x31, 255);
    SELECTED_OPTIONS_ITEM_COLOUR = graphics_make_color(0xCF, 0x48, 0x2C, 255);
}

/**
 * Inititalises all required libdragon subsystems.
 */
static void initialiseSubsystems() {
    getMemoryLimit();
    init_interrupts();
    timer_init();

    #ifdef SHOW_FRAME_COUNT
        new_timer(TIMER_TICKS(1000000), TF_CONTINUOUS, fps_timer);
    #endif

    controller_init();
    dfs_init(DFS_DEFAULT_LOCATION);

    if (USE_ANTIALIASING) {
        // Slightly better picture, but frame-rate drop on my N64
        display_init(RESOLUTION_640x480, DEPTH_16_BPP, 2, GAMMA_NONE, ANTIALIAS_RESAMPLE);
    } else {
        display_init(RESOLUTION_640x480, DEPTH_16_BPP, 2, GAMMA_NONE, ANTIALIAS_OFF);
    }
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
static void generateState() {
    rootState.PlayerCount = 1;
    generatePlayerState(&rootState.Players[0]);
}

/**
 * The mainloop of the program.  Has three phases and executes for all players at each phase
 * phase 1 read input
 * phase 2 carry out program logic
 * phase 3 paint the screen if required.
 * todo - audio.
 */
static void mainLoop() {
    rootState.RequiresRepaint = true;
    rootState.RequiresControllerRead = true;
    uLong iterations = 0;

    Mode modes[MAX_PLAYERS];
    for(byte i = 0; i < MAX_PLAYERS; i++) {
        modes[i] = 0;
    }

    while(!(rootState.Frame = display_lock())); 

    while (true) {
        // Read controller about once per frame.
        if (rootState.RequiresControllerRead || iterations > GB_LCD_FRAME_CLKS / 2) {
            controller_scan();
            natural previous = rootState.ControllersPresent;
            rootState.ControllersPresent = get_controllers_present();
            
            // If a new controller is plugged in, we may need to indicate that.
            if (previous != rootState.ControllersPresent) {
                rootState.RequiresRepaint = true;
            }

            rootState.KeysPressed = get_keys_pressed();
            rootState.KeysReleased = get_keys_up();
            rootState.RequiresControllerRead = false;
            iterations = 0;
        } else {
            iterations++;
        }

        UPDATE_PROFILE(PROFILE_C_SCAN);

        #if MAX_PLAYERS >= 2
        for (byte i = 0; i < rootState.PlayerCount; i++) {
            modes[i] = rootState.Players[i].ActiveMode;

            switch(modes[i]) {
                case Init:
                    initLogic(i);
                    break;
                case Play:
                    playLogic(i);
                    break;
                case Menu:
                    menuLogic(i);
                    break;
                case Options:
                    optionsLogic(i);
                    break;
                default: break;
            }
        }

        if (rootState.RequiresRepaint) {
            rootState.RequiresRepaint = false;

            for (byte i = 0; i < rootState.PlayerCount; i++) {
                switch(modes[i]) {
                    case Init:
                        initDraw(i);
                        break;
                    case Play:
                        playDraw(i);
                        break;
                    case Menu:
                        menuDraw(i);
                        break;
                    case Options:
                        optionsDraw(i);
                        break;
                    default: break;
                }
            }

            display_show(rootState.Frame);
            while(!(rootState.Frame = display_lock()));
            setFrameBufferId(rootState.Frame);
        }
        #else
            modes[0] = rootState.Players[0].ActiveMode;

            switch(modes[0]) {
                case Init:
                    initLogic(0);
                    break;
                case Play:
                    playLogic(0);
                    break;
                case Menu:
                    menuLogic(0);
                    break;
                case Options:
                    optionsLogic(0);
                    break;
                default: break;
            }

        if (rootState.RequiresRepaint) {
            rootState.RequiresRepaint = false;

                switch(modes[0]) {
                    case Init:
                        initDraw(0);
                        break;
                    case Play:
                        playDraw(0);
                        break;
                    case Menu:
                        menuDraw(0);
                        break;
                    case Options:
                        optionsDraw(0);
                        break;
                    default: break;
            }

            display_show(rootState.Frame);
            #ifdef IS_DOUBLE_BUFFERED
                while(!(rootState.Frame = display_lock()));
                setFrameBufferId(rootState.Frame);
            #endif
        }


        UPDATE_PROFILE(PROFILE_DRAW);

        #ifdef IS_PROFILING
        if (rootState.Players[0].Meta.FrameCount >= 50) {
            displayProfile();
        }
        #endif

        #endif
    }
}

/**
 * Initialise all then enter main loop.
 */
int main(void) {
    initialiseSubsystems();
    setGlobalConstants();
    generateState();
    flushScreen();

    rsp_init();

    mainLoop();

    freeText();
    freeResources();
    display_close();
    rdp_close();
    audio_close();
    return 0;
}
