#ifndef SCREEN_INCLUDED
#define SCREEN_INCLUDED

#include "core.h"
#include "state.h"

static const short RESOLUTION_X = 640;
static const short RESOLUTION_Y = 480;

static const float TEXT_SCALE_FACTOR = 0.210 / 100.0;

typedef struct {
    natural Top;
    natural Left;
    natural Width;
    natural Height;
} Rectangle;

// 16 bit colours are are 5 bits per colour and a transparency flag
static const uInt MONOCHROME_PALETTE[] = {
    0xffffffff, 0xA529A529, 0x52955295, 0x00010001
};

/**
 * Resets all screen buffers to a known state.
 * @param state program state.
 */
void flushScreen(RootState* state);

/**
 * Gets the RDP module ready to render a new texture.
 * @param frame identifies frame to render to.
 */
void prepareRdpForSprite(const display_context_t frame);

/**
 * Loads a single sprite sheet in to the primary texture slot
 * @param spriteSheet sheet the sprite is on.
 * @param spriteCode position of the sprite in the sheet.
 * @param mirrorSetting how the sprite should behave when mirroring.
 */
void loadSprite(sprite_t* spriteSheet, const byte spriteCode, const mirror_t mirrorSetting);

/**
 * Draws an unfilled rectangle of a certain thickness.
 * @param frame frame to draw the border on.
 * @param position Where to draw the border on the screeen.
 * @param thickness How thick the border is.
 * @param colour The colour of the border.
 */
void drawSolidBorder(const display_context_t frame, const Rectangle* position, const natural width, const natural colour);

/**
 * Get the gameboy screen rectangle based on player number.
 * @param state program state including number of players.
 * @param playerNumber number of a given player.
 * @out output The calculated screen size & position.
 */
void getScreenPosition(const RootState* state, const byte playerNumber, Rectangle* output);

/**
 * Takes a 16bit gameboy colour and returns a 32 bit libdragon colour
 * @param colour The colour from the gameboy
 * @return The colour that libdragon can render.
 */
uInt massageColour(const natural colour);

/**
 * Fade/Brighten a colour by a given amount.
 * @param colour The 16bit colour
 * @param fadeAmount This amount will be added to each component.
 * @returns The new faded colour.
 */
u16 fadeColour(const natural colour, const byte fadeAmount);

#endif
