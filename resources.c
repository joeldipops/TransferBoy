#include <libdragon.h>
#include "resources.h"
#include <math.h>

static sprite_t* _textMap = 0;
static sprite_t* _spriteSheet = 0;

static sprite_t* _transformCache[8] = {0};
static byte cacheTop = 0;
static byte _index[2][128][6] = {0}; // Two sprite sheets, Max 16*8 sprites per sheet, 6 possible rotations.

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
 * Takes an existing sprite, transforms it, and stashes it in a cache for next time it's needed.
 * @param sheet Source sprite sheet.
 * @param spriteCode Identifies source sprite on the sheet.
 * @param rotation How the sprite should be transformed.
 * @return Pointer to the new transformed sprite.
 */
sprite_t* rotateSprite(const sprite_t* sheet, const byte spriteCode, const sByte rotation) {
    byte sheetIndex, rotationIndex = 0;
    sheetIndex = (sheet == _textMap) ? 0 : 1;
    switch (rotation) {
        case ROTATE_90:
            rotationIndex = 0; break;
        case ROTATE_180:
            rotationIndex = 1; break;
        case ROTATE_270:
            rotationIndex = 2; break;
        case FLIP_HORIZONTAL:
            rotationIndex = 3; break;
        case FLIP_VERTICAL:
            rotationIndex = 4; break;
        case FLIP_BOTH:
            rotationIndex = 5; break;
        default:
            return null;
    }

    // Get from cache if we already have it.
    if (_transformCache[_index[sheetIndex][spriteCode][rotationIndex]]) {
        return _transformCache[_index[sheetIndex][spriteCode][rotationIndex]];
    }

    natural spriteWidth = sheet->width / sheet->hslices;
    natural spriteHeight = sheet->height / sheet->vslices;

    byte* data = calloc(spriteHeight * spriteWidth, sheet->bitdepth);

    byte x = spriteCode % sheet->hslices;
    byte y = floor(spriteCode / sheet->vslices);

    natural index = 0;

    // This loop doesn't work at all, but I think I am close.
    for (natural row = y; row < y + spriteHeight; row++) {
        for (natural column = x; column < x + spriteWidth; column++) {
            // 360 degrees
            memcpy(data + index, sheet->data + (row * sheet->width + column) * sheet->bitdepth, sheet->bitdepth);
            index += sheet->bitdepth;
        }
    }

    sprite_t* result = calloc(1, sizeof(sprite_t*));
    result->width = spriteWidth;
    result->height = spriteHeight;
    result->vslices = 1;
    result->hslices = 1;
    result->bitdepth = sheet->bitdepth;
    result->format = sheet->format;
    memcpy(result->data, data, index);

    // Put the newly generated sprite in to the cache.

    // If the cache is full, free up some memory
    if (_transformCache[cacheTop]) {
        free(_transformCache[cacheTop]->data);
        free(_transformCache[cacheTop]);
    }
    _transformCache[cacheTop] = result;
    _index[sheetIndex][spriteCode][rotationIndex] = cacheTop;

    // TODO Make sure we free the least recently used sprite instead of just going from 0 -7
    if (cacheTop < 7) {
        cacheTop++;
    } else {
        cacheTop = 0;
    }

    return result;
}

/**
 * Releases memory held by all resources. only rotate
 */
void freeResources() {
    free(_textMap);
    free(_spriteSheet);
    for (byte i = 0; i < 8; i++) {
        free(_transformCache[i]->data);
        free(_transformCache[i]);
    }
}
