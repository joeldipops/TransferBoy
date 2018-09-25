#include <stdio.h>
#include <string.h>
#include "core.h"
#include "text.h"
#include "screen.h"

#include <libdragon.h>

static string _strings[TextEnd] = {"", "", "", ""};
static sprite_t* _textMap = 0;
static sprite_t* _spriteSheet = 0;
static bool initted = false;

/**
 * Initialises text subsystem by loading sprites etc.
 * @return 0 result code
 ** 0   success
 ** -1  expected file not present
 */
sByte initText() {
    if (initted) {
        return 0;
    }

    // Set up string resources
    strcpy(_strings[TextEmpty], "");
    //strcpy(_strings[TextLoadCartridge], "Press $02 to load cartridge.");
    strcpy(_strings[TextLoadCartridge], "$00 $01 $02 $03");
    strcpy(_strings[TextNoTpak], "Please insert a Transfer Pak.");
    strcpy(_strings[TextNoCartridge], "Please insert a Game Boy cartridge.");
    strcpy(_strings[TextLoadingCartridge], "Loading cartridge, please wait.");
    strcpy(_strings[TextExpansionPakRequired], "This cartridge cannot be loaded without an Expansion Pak.");
    strcpy(_strings[TextMenuResume], "Resume");
    strcpy(_strings[TextMenuReset], "Reset");
    strcpy(_strings[TextMenuChangeCart], "Switch");
    strcpy(_strings[TextMenuOptions], "Options");
    strcpy(_strings[TextMenuAddPlayer], "Add Player");
    strcpy(_strings[TextMenuAddGame], "Add Game");
    strcpy(_strings[TextSplash], "~TRANSFER BOY~");

    // Read in character sprite sheet.
    sInt textMapPointer = dfs_open("/textMap.sprite");
    if (!textMapPointer) {
        return -1;
    }
    _textMap = malloc(dfs_size(textMapPointer));
    dfs_read(_textMap, 1, dfs_size(textMapPointer), textMapPointer);
    dfs_close(textMapPointer);

    // Read in other sprites.
    sInt spriteSheetPointer = dfs_open("/spriteSheet.sprite");
    if (!spriteSheetPointer) {
        return -1;
    }
    _spriteSheet = malloc(dfs_size(spriteSheetPointer));
    dfs_read(_spriteSheet, 1, dfs_size(spriteSheetPointer), spriteSheetPointer);
    dfs_close(spriteSheetPointer);

    initted = true;
    return 0;
}

/**
 * Frees resources used by the text subsystem when done.
 */
void freeText() {
    //free(_strings);
    free(_textMap);
    initted = false;
}

static const byte CHARACTER_SIZE = 24;
static const byte SPRITE_SIZE = 32;

/**
 * Gets the text string with the given id code, and loads it into output.
 * @param textId The text id code.
 * @out output the destination string.
 */
void getText(const TextId textId, string output) {
    if (!initted) {
        initText();
    }
    sprintf(output, _strings[textId]);
}

/**
 * Draws a text character from the sprite sheet at a given location.
 * @param character the ASCII character to draw
 * @param x The x co-ordinate to draw at.
 * @param y The y co-ordinate to draw at.
 * @private
 */
void drawCharacter(const char character, const natural x, const natural y, const float scale) {
    // Avoid printing any control characters, we don't at this point know what
    // wackiness will ensue.
    if (character <= 0x20) {
        return;
    }

    byte offset = character - 0x21;
    rdp_sync(SYNC_PIPE);
    rdp_load_texture_stride(0, 0, MIRROR_DISABLED, _textMap, offset);
    rdp_draw_sprite_scaled(0, x, y, scale, scale);
}

/**
 * Draws characters in a line.
 * @param frame The id of the frame to draw on.
 * @param x The x co-ordinate to start the string at.
 * @param y The y co-ordinate to start the string at.
 * @param scale size of the text sprites.
 * @return result code
 ** 0   success
 ** -1  badly formatted token.
 * @private
 */
sByte drawTextLine(const display_context_t frame, const string text, const natural x, const natural y, const float scale) {
    byte length = strlen(text);
    natural left = x;
    for (byte i = 0; i < length; i++) {
        // $ token means draw a sprite instead of a text character.
        if (text[i] == '$') {
            if (length <= i + 2) {
                return -1;
            }
            // sprite is 2 digit hex
            byte spriteCode = 16 * (text[i + 1] - '0') + (text[i + 2] - '0');
            rdp_sync(SYNC_PIPE);
            rdp_load_texture_stride(0, 0, MIRROR_DISABLED, _spriteSheet, spriteCode);
            rdp_draw_sprite_scaled(0, left, y, ceil(scale), ceil(scale));
            i += 2;
            left += SPRITE_SIZE * scale;
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
    if (!initted) {
        initText();
    }
    prepareRdpForTexture(frame);
    drawTextLine(frame, text, x, y, scale);

    rdp_detach_display();
}

/**
 * Gets string length, accounting for tokens.
 */
byte getTextLength(const string text) {
    byte length = strlen(text);
    byte result = 0;
    for (byte i = 0; i < length; i++) {
        if (text[i] == '$') {
            i += 2; // skip next two characters after insert-sprite token.
        }
        if (text[i] == '\\') {
            i += 1; // don't count escape token
        }
        result++;
    }

    return result;
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

        string line = "";
        memcpy(line, text + i, lineBreak);

        drawTextLine(frame, line, x, top, scale);

        i += lineBreak;
        top += CHARACTER_SIZE;
    }
    rdp_detach_display();
}
