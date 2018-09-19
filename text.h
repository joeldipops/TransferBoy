#ifndef TEXT_INCLUDED
#define TEXT_INCLUDED

#include "core.h"
#include <libdragon.h>

typedef enum {
    TextEmpty,
    TextNoTpak,
    TextNoCartridge,
    TextLoadCartridge,
    TextLoadingCartridge,
    TextExpansionPakRequired,
    TextMenuResume,
    TextMenuReset,
    TextMenuChangeCart,
    TextMenuOptions,
    TextMenuAddPlayer,
    TextMenuAddGame,
    TextEnd
} TextId;

/**
 * Gets the text string with the given id code, and loads it into output.
 * @param textId The text id code.
 * @out output the destination string.
 */
void getText(TextId textId, string output);

/**
 * Initialises text subsystem by loading sprites etc.
 */
void initText();

/**
 * Draws a horizontal string of text starting at the given location.
 * @param frame The id of the frame to draw on.
 * @param x The x co-ordinate to start the string at.
 * @param y The y co-ordinate to start the string at.
 */
void drawText(const display_context_t frame, const string text, const unsigned short x, const unsigned short y);

#endif
