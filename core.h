#ifndef CORE_INCLUDED
#define CORE_INCLUDED

#define null 0
typedef enum { false, true } bool;

#include "include/gbc_bundle.h"
#include <libdragon.h>

typedef char string[1024];
typedef unsigned char byte;
typedef unsigned int natural;
typedef enum { NoButton, A, B, L, R, Z, Start, Up, Down, Left, Right, CUp, CDown, CLeft, CRight, Stick } N64Button;
typedef enum { GbNoButton, GbA, GbB, GbUp, GbDown, GbLeft, GbRight, GbStart, GbSelect, GbSystemMenu } GbButton;

const unsigned char VERTICAL_MARGIN = 30;
const unsigned char HORIZONTAL_MARGIN = 30;
const unsigned char VERTICAL_MENU_SPACING = 25;

const unsigned short SINGLE_PLAYER_SCREEN_TOP = 50;
const unsigned short SINGLE_PLAYER_SCREEN_LEFT = 140;
const unsigned short SINGLE_PLAYER_SCREEN_WIDTH = 360;
const unsigned short SINGLE_PLAYER_SCREEN_HEIGHT = 324;

const unsigned short PLAYER_1_SCREEN_TOP;
const unsigned short PLAYER_1_SCREEN_LEFT;
const unsigned short PLAYER_1_SCREEN_WIDTH;
const unsigned short PLAYER_1_SCREEN_HEIGHT;

const unsigned short PLAYER_2_SCREEN_TOP;
const unsigned short PLAYER_2_SCREEN_LEFT;
const unsigned short PLAYER_2_SCREEN_WIDTH;
const unsigned short PLAYER_2_SCREEN_HEIGHT;

natural GLOBAL_BACKGROUND_COLOUR = 0;
natural GLOBAL_TEXT_COLOUR = 0;
natural BLANK_SCREEN_COLOUR = 0;


typedef struct {
    unsigned long Size;
    byte* Data;
} ByteArray;

typedef enum { Quit, Init, Play, Menu, Options } Mode;
typedef enum { BorderNone } Border;

typedef struct {
    ByteArray RomData;
    ByteArray SaveData;
    string Title;
    bool IsGbcCart;
    bool IsSuperGbCart;
} CartridgeData;

typedef struct {
    Mode ActiveMode;
    Border SelectedBorder;
    GbButton ButtonMap[16];
    CartridgeData Cartridge;
    bool AudioEnabled;
    struct gb_state EmulationState;
} PlayerState;

typedef struct {
    bool RequiresRepaint;
    char PlayerCount;
    struct controller_data ControllerState;
    display_context_t Frame;
    PlayerState Players[8]; // Really not aiming for anything other than 2, but you never know.
} RootState;

#endif
