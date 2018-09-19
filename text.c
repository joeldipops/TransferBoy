#include <stdio.h>
#include <string.h>
#include "core.h"
#include "text.h"

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
    strcpy(_textMap[TextLoadCartridge], "Press (Start) to load cartridge.");
    strcpy(_textMap[TextNoTpak], "Please insert a Transfer Pak.");
    strcpy(_textMap[TextNoCartridge], "Please insert a Game Boy cartridge.");
    strcpy(_textMap[TextLoadingCartridge], "Loading Cartridge, Please Wait.");
    strcpy(_textMap[TextExpansionPakRequired], "This Cartridge cannot be loaded without an Expansion Pak.");
    strcpy(_textMap[TextMenuResume], "RESUME");
    strcpy(_textMap[TextMenuReset], "RESET");
    strcpy(_textMap[TextMenuChangeCart], "CHANGE GAME");
    strcpy(_textMap[TextMenuOptions], "OPTIONS");
    strcpy(_textMap[TextMenuAddPlayer], "ADD PLAYER");
    strcpy(_textMap[TextMenuAddGame], "ADD GAME");

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

static const unsigned short DEFAULT_CHARACTER_SIZE = 20;


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
void drawCharacter(const char character, const unsigned short x, const unsigned short y) {
    byte offset = character - 0x21;
    rdp_sync(SYNC_PIPE);
    rdp_load_texture_stride(0, 0, MIRROR_DISABLED, textMap, offset);
    rdp_draw_sprite(0, x, y);
}

/**
 * Draws a horizontal string of text starting at the given location.
 * @param frame The id of the frame to draw on.
 * @param x The x co-ordinate to start the string at.
 * @param y The y co-ordinate to start the string at.
 */
void drawText(const display_context_t frame, const string text, const unsigned short x, const unsigned short y) {
    if (!initted) {
        initText();
    }

    // Assure RDP is ready for new commands
    rdp_sync( SYNC_PIPE );
    // Remove any clipping windows
    rdp_set_default_clipping();
    // Enable sprite display instead of solid color fill
    rdp_enable_texture_copy();
    // Attach RDP to display
    rdp_attach_display(frame);

    byte length = strlen(text);
    for (byte i = 0; i < length; i++) {
        drawCharacter(text[i], x + i * DEFAULT_CHARACTER_SIZE, y);
    }

    rdp_detach_display();
}
