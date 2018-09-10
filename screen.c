#include "state.h"
#include "screen.h"

const unsigned short SINGLE_PLAYER_SCREEN_TOP = 50;
const unsigned short SINGLE_PLAYER_SCREEN_LEFT = 140;
const unsigned short SINGLE_PLAYER_SCREEN_WIDTH = 360;
const unsigned short SINGLE_PLAYER_SCREEN_HEIGHT = 324;

const unsigned short PLAYER_1_SCREEN_TOP = 134;
const unsigned short PLAYER_1_SCREEN_LEFT = 27;
const unsigned short PLAYER_1_SCREEN_WIDTH = 267;
const unsigned short PLAYER_1_SCREEN_HEIGHT = 240;

const unsigned short PLAYER_2_SCREEN_TOP = 134;
const unsigned short PLAYER_2_SCREEN_LEFT = 347;
const unsigned short PLAYER_2_SCREEN_WIDTH = 267;
const unsigned short PLAYER_2_SCREEN_HEIGHT = 240;

/**
 * Get the gameboy screen rectangle based on player number.
 * @param state program state including number of players.
 * @param playerNumber number of a given player.
 * @out output The calculated screen size & position.
 */
void getScreenPosition(const RootState* state, const byte playerNumber, ScreenPosition* output) {
    if (state->PlayerCount == 1) {
        output->Top = SINGLE_PLAYER_SCREEN_TOP;
        output->Left = SINGLE_PLAYER_SCREEN_LEFT;
        output->Width = SINGLE_PLAYER_SCREEN_WIDTH;
        output->Height = SINGLE_PLAYER_SCREEN_HEIGHT;
    } else if (state->PlayerCount == 2) {
        if (playerNumber == 0) {
            output->Top = PLAYER_1_SCREEN_TOP;
            output->Left = PLAYER_1_SCREEN_LEFT;
            output->Width = PLAYER_1_SCREEN_WIDTH;
            output->Height = PLAYER_1_SCREEN_HEIGHT;
        } else if (playerNumber == 1) {
            output->Top = PLAYER_2_SCREEN_TOP;
            output->Left = PLAYER_2_SCREEN_LEFT;
            output->Width = PLAYER_2_SCREEN_WIDTH;
            output->Height = PLAYER_2_SCREEN_HEIGHT;
        } else {
            logAndPause("Unimplemented getScreenPosition");
        }
    } else {
        logAndPause("Unimplemented getScreenPosition");
    }
}


