#include "core.h"
#include "types.h"
#include "screen.h"
#include "text.h"
#include "resources.h"
#include <stdio.h>
#include <libdragon.h>

static bool _isLoading[MAX_PLAYERS] = {0};
static timer_link_t* _timer;
static const int UPDATE_TIME = 500000;

/**
 * Fired periodically as we load a cartridge.  It updates the display so we know
 * how much longer to wait (and that we haven't crashed.)
 * @param controllerNumber controller we are loading from.
 */
static void updateProgressIndicator(const byte controllerNumber) {
    byte loadPercent = getLoadProgress(controllerNumber);
    string text;
    getText(TextLoadingCartridge, text);
    sprintf(text, "%s - %d%%", text, loadPercent);

    Rectangle screen = {};
    getScreenPosition(controllerNumber, &screen);

    natural textTop = screen.Top - TEXT_HEIGHT + (screen.Width / 2);

    while(!(rootState.Frame = display_lock()));

    // Hide the previous text.
    prepareRdpForSprite(rootState.Frame);
    loadSprite(getSpriteSheet(), GB_BG_TEXTURE, MIRROR_DISABLED);
    rdp_draw_textured_rectangle(0, screen.Left - 1, screen.Top, screen.Left + screen.Width, screen.Top + screen.Height, MIRROR_XY);
    drawTextParagraph(rootState.Frame, text, screen.Left + TEXT_WIDTH, textTop, 0.8, screen.Width - TEXT_WIDTH);

    display_show(rootState.Frame);
}

/**
 * Timer callback, it updates the progress bar of any player that is needs one.
 * @param ovfl
 */
static void onLoadProgressTimer(int ovfl) {
    for (byte i = 0; i < MAX_PLAYERS; i++) {
        if (_isLoading[i]) {
            updateProgressIndicator(i);
        }
    }
}

/**
 * Sets up handlers to display a progress bar for the cartridge load.
 * @param playerNumber identifies player loading a cartridge.
 */
void startLoadProgressTimer(const byte playerNumber) {
    // The main loop is locked up with loading the rom, so we need to have an internal
    // display lock/show loop.
    // This needs to be balanced with the main loop or things get out of whack.
    display_show(rootState.Frame);
    _isLoading[playerNumber] = true;

    if (!_timer) {
        _timer = new_timer(TIMER_TICKS(UPDATE_TIME), TF_CONTINUOUS, onLoadProgressTimer);
    }
}

/**
 * Cleans up timer once a cartridge is loaded
 * @param playerNumber player no longer using the progress bar
 */
void closeLoadProgressTimer(const byte playerNumber) {
    _isLoading[playerNumber] = false;

    // Check if there are any other players still using the timer.
    bool isDone = true;
    for (byte i = 0; i < MAX_PLAYERS; i++) {
        if (_isLoading[i]) {
            isDone = false;
        }
    }

    // If this was the last one, clean up.
    if (isDone) {
        delete_timer(_timer);
        _timer = null;
    }

    // Balance with the main loop.
    while(!(rootState.Frame = display_lock()));
}


