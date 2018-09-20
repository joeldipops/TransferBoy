#include <stdio.h>
#include <string.h>
#include "core.h"
#include "text.h"
#include "screen.h"

#include <libdragon.h>

static string _textMap[TextEnd] = {"", "", "", ""};
static bool initted = false;
static sprite_t* textMap = 0;

/**
 * Initialises text subsystem by loading sprites etc.
 */
void initText() {
    if (initted) {
        return;
    }

    // Set up string resources
    strcpy(_textMap[TextEmpty], "");
    strcpy(_textMap[TextLoadCartridge], "Press (S) to load cartridge.");
    strcpy(_textMap[TextNoTpak], "Please insert a Transfer Pak.");
    strcpy(_textMap[TextNoCartridge], "Please insert a Game Boy cartridge.");
    strcpy(_textMap[TextLoadingCartridge], "Loading Cartridge, Please Wait.");
    strcpy(_textMap[TextExpansionPakRequired], "This Cartridge cannot be loaded without an Expansion Pak.");
    strcpy(_textMap[TextMenuResume], "Resume");
    strcpy(_textMap[TextMenuReset], "Reset");
    strcpy(_textMap[TextMenuChangeCart], "Change Game");
    strcpy(_textMap[TextMenuOptions], "Options");
    strcpy(_textMap[TextMenuAddPlayer], "Add Player");
    strcpy(_textMap[TextMenuAddGame], "Add Game");
    strcpy(_textMap[TextSplash], "~TRANSFER BOY~");

    // Read in character sprite sheet.
    int textMapPointer = dfs_open("/textMap.sprite");
    if (!textMapPointer) {
        return;
    }

    textMap = malloc(dfs_size(textMapPointer));
    dfs_read(textMap, 1, dfs_size(textMapPointer), textMapPointer);
    dfs_close(textMapPointer);

    initted = true;
}

static const natural DEFAULT_CHARACTER_SPACING = 20;
static const natural DEFAULT_CHARACTER_SIZE = 20;


/**
 * Gets the text string with the given id code, and loads it into output.
 * @param textId The text id code.
 * @out output the destination string.
 */
void getText(TextId textId, string output) {
    if (!initted) {
        initText();
    }
    sprintf(output, _textMap[textId]);
}

/**
 * Draws a text character from the sprite sheet at a given location.
 * @param character the ASCII character to draw
 * @param x The x co-ordinate to draw at.
 * @param y The y co-ordinate to draw at.
 * @private
 */
void drawCharacter(const char character, const natural x, const natural y, const float scale) {
    byte offset = character - 0x21;
    rdp_sync(SYNC_PIPE);
    rdp_load_texture_stride(0, 0, MIRROR_DISABLED, textMap, offset);
    rdp_draw_sprite_scaled(0, x, y, scale, scale);
}

/**
 * Draws characters in a line.
 * @param frame The id of the frame to draw on.
 * @param x The x co-ordinate to start the string at.
 * @param y The y co-ordinate to start the string at.
 * @param scale size of the text sprites.
 * @private
 */
void drawTextLine(const display_context_t frame, const string text, const natural x, const natural y, const float scale) {
    byte length = strlen(text);
    for (byte i = 0; i < length; i++) {
        drawCharacter(text[i], x + i * DEFAULT_CHARACTER_SPACING * scale, y, scale);
    }
}


/**
 * Draws a horizontal string of text starting at the given location.
 * @param frame The id of the frame to draw on.
 * @param x The x co-ordinate to start the string at.
 * @param y The y co-ordinate to start the string at.
 * @param scale size of the text sprites.
 */
void drawText(const display_context_t frame, const string text, const natural x, const natural y, const float scale) {
    if (!initted) {
        initText();
    }
    prepareRdpForTexture(frame);
    drawTextLine(frame, text, x, y, scale);

    rdp_detach_display();
}

/**
 * Draws a horizontal string of text starting at the given location.
 * @param frame The id of the frame to draw on.
 * @param x The x co-ordinate to start the string at.
 * @param y The y co-ordinate to start the string at.
 * @param scale size of the text sprites.
 * @param width width of the area available for text.
 */
void drawTextParagraph(
    const display_context_t frame,
    const string text,
    const natural x,
    const natural y,
    const float scale,
    const natural width
) {
    prepareRdpForTexture(frame);

    natural top = y;
    byte length = strlen(text);
    byte maxLength = width / (DEFAULT_CHARACTER_SIZE * scale);

    byte i = 0;
    while(i < length) {
        byte lineLength = 0;
        lineLength = (length - i < maxLength) ? length - i : maxLength;

        // Split on spaces.
        byte spacePos = lineLength;
        for(byte j = i; j < lineLength; j++) {
            if (0x20 == *(text + j)) {
                spacePos = j;
            }
        }

        string line = "";
        memcpy(line, text + i, spacePos);

        drawTextLine(frame, line, x, top, scale);

        i += spacePos;
        top += DEFAULT_CHARACTER_SIZE;
    }

    rdp_detach_display();
}
