#include "core.h"
#include "types.h"
#include "screen.h"
#include "text.h"
#include <stdio.h>
#include <libdragon.h>

static RootState* _state;
static bool _isLoading[MAX_PLAYERS] = {0};
static timer_link_t* _timer;

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
    getScreenPosition(_state, controllerNumber, &screen);

    natural textTop = screen.Top - TEXT_HEIGHT + (screen.Width / 2);

    // This is crashing us somehow :(
    //drawTextParagraph(_state->Frame, text, screen.Left + TEXT_WIDTH, textTop, 0.8, screen.Width - TEXT_WIDTH);
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
 * @param Address of the global state so it can be referenced in a callback.
 * @param Number identifying player that will load a cartridge.
 */
void startLoadProgressTimer(RootState* rootState, const byte playerNumber) {
    if (!_state) {
        _state = rootState;
    }

    _isLoading[playerNumber] = true;

    if (!_timer) {
        _timer = new_timer(TIMER_TICKS(1000000), TF_CONTINUOUS, onLoadProgressTimer);
    }
}

/**
 * Cleans up handlers for displaying the cartridge load progress bar.
 * @param timer timer to close.
 */
void closeLoadProgressTimer(const byte playerNumber) {
    _isLoading[playerNumber] = false;

    // Check if this was the last time.
    bool isDone = true;
    for (byte i = 0; i < MAX_PLAYERS; i++) {
        if (_isLoading[i]) {
            isDone = false;
        }
    }

    // If so, clean up.
    if (isDone) {
        delete_timer(_timer);
        _timer = null;
        _state = null;
    }
}


