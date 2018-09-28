#include <libdragon.h>
#include "resources.h"

static sprite_t* _textMap = 0;
static sprite_t* _spriteSheet = 0;
static bool resourcesInitted = false;

/**
 * Loads oft-used resources in to memory.
 * @return success/error code
 ** 0   initted successfully
 ** -1  could not find expected resource file.
 */
sByte initResources() {
    if (resourcesInitted) {
        return 0;
    }

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

    resourcesInitted = true;
    return 0;
}

/**
 * Gets sprite sheet of textual characters.
 * @return pointer to sprite sheet.
 */
sprite_t* getCharacterSheet() {
    return _textMap;
}

/**
 * Gets sprite sheet of icons and textures.
 * @return pointer to sprite sheet.
 */
sprite_t* getSpriteSheet() {
    return _spriteSheet;
}

/**
 * Releases memory held by all resources.
 */
void freeResources() {
    free(_textMap);
    free(_spriteSheet);
}
