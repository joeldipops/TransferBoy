#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "core.h"
#include "text.h"
#include "resources.h"
#include "screen.h"

#include <libdragon.h>

static string _strings[TextEnd] = {"", "", "", ""};
static bool textInitted = false;

static const byte CHARACTER_SIZE = 24;
static const byte SPRITE_SIZE = 32;

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
    strcpy(_strings[TextLoadingCartridge], "Loading cartridge, please wait...");
    strcpy(_strings[TextExpansionPakRequired], "This cartridge cannot be loaded without an Expansion Pak.");
    strcpy(_strings[TextChecksumFailed], "The cartridge could not be read correctly. Error %d.");
    strcpy(_strings[TextUnsupportedCartridge], "This cartridge is not yet supported");
    strcpy(_strings[TextLoadCartridgePrompt], "Press A to retry.");
    strcpy(_strings[TextMenuResume], "Resume");
    strcpy(_strings[TextMenuReset], "Reset");
    strcpy(_strings[TextMenuChangeCart], "Switch");
    strcpy(_strings[TextMenuOptions], "Options");
    strcpy(_strings[TextMenuAddPlayer], "Add Player");
    strcpy(_strings[TextMenuAddGame], "Add Game");
    strcpy(_strings[TextSplash], "\\~TRANSFER BOY~");
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

/**
 * Gets the text string with the given id code, and loads it into output.
 * @param textId The text id code.
 * @out output the destination string.
 */
void getText(const TextId textId, string output) {
    if (!textInitted) {
        initText();
    }
    strcpy(output, _strings[textId]);
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
static void drawSprite(const byte spriteCode, sprite_t* spriteSheet, const natural x, const natural y, const float scale) {
    rdp_load_texture_stride(0, 1, MIRROR_DISABLED, spriteSheet, spriteCode);
    rdp_draw_sprite_scaled(0, x, y, scale, scale, MIRROR_XY);
}

/**
 * Draws a text character from the sprite sheet at a given location with the given transformation.
 * @param character the ASCII character to draw
 * @param x The x co-ordinate to draw at.
 * @param y The y co-ordinate to draw at.
 * @param scale size of the image
 * @param transformation Flip/Fade/Shift etc the character.
 * @private
 */
static void drawTransformedCharacter(const char character, const natural x, const natural y, const float scale, const Transformation transformation) {
    // Avoid printing any control characters, we don't at this point know what
    // wackiness will ensue.
    if (character <= 0x20) {
        return;
    }

    sprite_t* sheet = getCharacterSheet();

    byte offset = character - 0x20;
    if (transformation) {
        sheet = transformSprite(sheet, offset, transformation);
        offset = 0;
    }

    drawSprite(offset, sheet, x, y, scale);
}

/**
 * Draws a text character from the sprite sheet at a given location.
 * @param character the ASCII character to draw
 * @param x The x co-ordinate to draw at.
 * @param y The y co-ordinate to draw at.
 * @param scale size of the image
 * @private
 */
static void drawCharacter(const char character, const natural x, const natural y, const float scale) {
    drawTransformedCharacter(character, x, y, scale, 0);
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
static sShort drawImage(const string text, const byte textIndex, const byte length, const natural x, const natural y, const float scale) {
    byte i = textIndex;
    char transformation = text[i+1];
    if (length <= i + 2) {
        return -1;
    } else if (
        transformation == ROTATE_90
        || transformation == ROTATE_180
        || transformation == ROTATE_270
        || transformation == FLIP_HORIZONTAL
        || transformation == FLIP_VERTICAL
    ) {
        // Flip/Rotation specifier optionally follows the $ sign.
        if (length <= i + 3) {
            return -1;
        }
        i++;
    } else {
        transformation = 0;
    }

    // sprite is 2 digit hex, we need to parse it from the string
    byte spriteCode = parseByte(&text[i+1], 2, 16);

    sprite_t* sheet = getSpriteSheet();
    if (transformation) {
        sheet = transformSprite(sheet, spriteCode, transformation);
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
static sByte drawTextLine(const string text, const natural x, const natural y, const float scale) {
    byte length = strlen(text);
    natural left = x;
    Transformation transform = 0;

    byte i = 0;

    // If the first character is a ~, fade the whole line.
    if (text[0] == '~') {
        transform = FADE;
        i++;
    }

    for (; i < length; i++) {
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
        drawTransformedCharacter(text[i], left, y, scale, transform);
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

/**
 * Calculates the length in pixels of a given string, including any sprites.
 * @param text the string.
 * @return the length in pixels.
 */
natural getStringWidth(const string text) {
    // TODO Sprites.
    return strlen(text) * CHARACTER_SIZE;
}
