#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "core.h"
#include "text.h"
#include "screen.h"

#include <libdragon.h>

static string _strings[TextEnd] = {"", "", "", ""};
static bool textInitted = false;

/**
 * Initialises text subsystem by loading sprites etc.
 * @return 0 result code
 ** 0   success
 ** -1  expected file not present
 */
sByte initText() {
    if (textInitted) {
        return 0;
    }

    // Set up string resources
    strcpy(_strings[TextEmpty], "");
    strcpy(_strings[TextLoadCartridge], "Press $02 to load cartridge.");
    strcpy(_strings[TextNoTpak], "Please insert a Transfer Pak.");
    strcpy(_strings[TextNoCartridge], "Please insert a Game Boy cartridge.");
    strcpy(_strings[TextLoadingCartridge], "Loading cartridge, please wait.");
    //strcpy(_strings[TextLoadingCartridge], "$02 $v02 $>02");
    strcpy(_strings[TextExpansionPakRequired], "This cartridge cannot be loaded without an Expansion Pak.");
    strcpy(_strings[TextMenuResume], "Resume");
    strcpy(_strings[TextMenuReset], "Reset");
    strcpy(_strings[TextMenuChangeCart], "Switch");
    strcpy(_strings[TextMenuOptions], "Options");
    strcpy(_strings[TextMenuAddPlayer], "Add Player");
    strcpy(_strings[TextMenuAddGame], "Add Game");
    strcpy(_strings[TextSplash], "~TRANSFER BOY~");
    strcpy(_strings[TextAudioOff], "Audio : Off");
    strcpy(_strings[TextAudioOn], "Audio : On");

    sByte result = initResources();
    if (result) {
        return result;
    } else {
        textInitted = true;
        return result;
    }
}

/**
 * Frees resources used by the text subsystem when done.
 */
void freeText() {
    //free(_strings);
    textInitted = false;
}

static const byte CHARACTER_SIZE = 24;
static const byte SPRITE_SIZE = 32;

/**
 * Gets the text string with the given id code, and loads it into output.
 * @param textId The text id code.
 * @out output the destination string.
 */
void getText(const TextId textId, string output) {
    if (!textInitted) {
        initText();
    }
    sprintf(output, _strings[textId]);
}

/**
 * Draws a sprite from a sheet at the given location & scale.
 * @param spriteCode id of the sprite on the sheet.
 * @param spriteSheet the sheet of sprites.
 * @param x The x screen position.
 * @param y The y screen position.
 * @param scale Scale the sprite.
 * @private
 */
void drawSprite(const byte spriteCode, sprite_t* spriteSheet, const natural x, const natural y, const float scale) {
    rdp_sync(SYNC_PIPE);
    rdp_load_texture_stride(0, 1, MIRROR_DISABLED, spriteSheet, spriteCode);
    rdp_draw_sprite_scaled(0, x, y, scale, scale);
}



/**
 * Draws a text character from the sprite sheet at a given location.
 * @param character the ASCII character to draw
 * @param x The x co-ordinate to draw at.
 * @param y The y co-ordinate to draw at.
 * @param scale size of the image
 * @private
 */
void drawCharacter(const char character, const natural x, const natural y, const float scale) {
    // Avoid printing any control characters, we don't at this point know what
    // wackiness will ensue.
    if (character <= 0x20) {
        return;
    }

    byte offset = character - 0x20;
    drawSprite(offset, getCharacterSheet(), x, y, scale);
}

/**
 * Draws an image from a sprite sheet, as specified by a token in the string, rotating if requested.
 * @param text the string containing the image to draw.
 * @param textIndex the position in text that of the token.
 * @param length Length of the string to avoid recalculating each iteration.
 * @param x The x co-ordinate to draw at.
 * @param y The y co-ordinate to draw at.
 * @param scale size of the image
 * @return
 ** next index of the string after the token if positive.
 ** error code if negative
 *** -1 token is not complete.
 *** -2 badly formatted token.
 * @private
 */
sShort drawImage(const string text, const byte textIndex, const byte length, const natural x, const natural y, const float scale) {
    byte i = textIndex;
    char rotation = text[i+1];
    if (length <= i + 2) {
        return -1;
    } else if (
        rotation == ROTATE_90
        || rotation == ROTATE_180
        || rotation == ROTATE_270
        || rotation == FLIP_HORIZONTAL
        || rotation == FLIP_VERTICAL
        || rotation == FLIP_BOTH
    ) {
        // Flip/Rotation specifier optionally follows the $ sign.
        if (length <= i + 3) {
            return -1;
        }
        i++;
    } else {
        rotation = 0;
    }

    // sprite is 2 digit hex, we need to parse it from the string
    byte spriteCode = parseByte(&text[i+1], 2, 16);

    sprite_t* sheet = getSpriteSheet();
    if (rotation) {
        sheet = rotateSprite(sheet, spriteCode, rotation);
        spriteCode = 0;
    }

    drawSprite(spriteCode, sheet, x, y, ceil(scale));

    i += 2;
    return i;
}

/**
 * Draws characters in a line.
 * @param x The x co-ordinate to start the string at.
 * @param y The y co-ordinate to start the string at.
 * @param scale size of the text sprites.
 * @return result code
 ** 0   success
 ** -1  token is not complete.
 * @private
 */
sByte drawTextLine(const string text, const natural x, const natural y, const float scale) {
    byte length = strlen(text);
    natural left = x;
    for (byte i = 0; i < length; i++) {
        // $ token means draw a sprite instead of a text character.
        if (text[i] == '$') {
            sByte result = drawImage(text, i, length, left, y, scale);
            if (result < 0) {
                return result;
            } else {
                i = result;
            }
            left += SPRITE_SIZE * ceil(scale);
            continue;
        }
        // Do literal draw of whatever follows slash.
        if (text[i] == '\\') {
            i++;
        }
        drawCharacter(text[i], left, y, scale);
        left += CHARACTER_SIZE * scale;
    }
    return 0;
}


/**
 * Draws a horizontal string of text starting at the given location.
 * @param frame The id of the frame to draw on.
 * @param x The x co-ordinate to start the string at.
 * @param y The y co-ordinate to start the string at.
 * @param scale size of the text sprites.
 */
void drawText(const display_context_t frame, const string text, const natural x, const natural y, const float scale) {
    if (!textInitted) {
        initText();
    }
    prepareRdpForSprite(frame);
    drawTextLine(text, x, y, scale);

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
    prepareRdpForSprite(frame);

    natural top = y;

    byte stringLength = strlen(text);
    byte maxLength = width / (CHARACTER_SIZE * scale);

    byte i = 0;
    while(i < stringLength) {
        byte lineAvailable = 0;
        lineAvailable = (stringLength - i < maxLength) ? stringLength - i : maxLength;

        // Split on spaces.
        byte lineBreak = lineAvailable;
        for(byte j = 0; j < lineAvailable; j++) {
            if (text[i + j] == '$') {
                // 3 characters, but only takes up 1 space.
                j += 2;
                lineAvailable += 2;
            } else if (text[i + j] == '\\') {
                // 2 characters, 1 space.
                j += 1;
                lineAvailable += 1;
            }
            if (0x20 == *(text + i + j)) {
                lineBreak = j;
            }
        }

        // if we're don't need any more lines, just run to the end.
        if (i + lineAvailable >= stringLength) {
            lineBreak = lineAvailable;
        } else {
            // make sure if we end on one of these, we keep the whole token.
            if (text[lineBreak] == '$') {
                lineBreak += 2;
            } else if (text[lineBreak-1] == '$') {
                lineBreak += 1;
            } else if (text[lineBreak] == '\\') {
                lineBreak += 1;
            }

            // absorb spaces into the newline
            while(lineBreak < stringLength && *(text + i + lineBreak) == 0x20) {
                lineBreak++;
            }
        }

        string line = "";
        memcpy(line, text + i, lineBreak);

        drawTextLine(line, x, top, scale);

        i += lineBreak;
        top += CHARACTER_SIZE;
    }
    rdp_detach_display();
}
