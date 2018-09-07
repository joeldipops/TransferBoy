#include <stdio.h>
#include <string.h>
#include "text.h"
static string _textMap[TextEnd] = {"", "", "", ""};
static bool initted = false;

/**
 * @private
 */
void initText() {
    strcpy(_textMap[TextEmpty], "");
    strcpy(_textMap[TextLoadCartridge], "Press (Start) to load cartridge.");
    strcpy(_textMap[TextNoTpak], "Please insert a Transfer Pak.");
    strcpy(_textMap[TextNoCartridge], "Please insert a Game Boy cartridge.");
    strcpy(_textMap[TextLoadingCartridge], "Loading Cartridge, Please Wait");
    initted = true;
}


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
