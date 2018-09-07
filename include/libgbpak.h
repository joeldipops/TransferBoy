#ifndef LIBGBPAK_INCLUDED
#define LIBGBPAK_INCLUDED
/*
 * Completely bastardised from original code to fit my purpose and formatting standards.
 */

#define GB_NORM		0x00
#define GB_MBC1		0x01
#define GB_MBC2		0x02
#define GB_MMMO1	0x03
#define GB_MBC3		0x04
#define GB_MBC4		0x10 //test
#define GB_MBC5		0x05
#define GB_CAMERA	0x06
#define GB_TAMA5	0x07
#define GB_HUC3		0x08
#define GB_HUC1		0x09

#define FALSE 0
#define TRUE 1
#define BANKSIZE 16*1024

typedef struct {
    char errorCode;
    char mapper;
    char ram;
    char battery;
    char rtc;
    char rumble;
    char sgb;
    char gbc;
    char title[12];
    char _romsize;
    char _ramsize;
    int romsize;
    int ramsize;
    short rombanks;
    short rambanks;
    short bank;
    short cpld;
} GameboyCart;

int disable_gbRam(const GameboyCart gbcart);

/**
 * Determines essential data about the catridge in the given TPak - size, Memory Bank configuration, title etc and loads into memory.
 * @param controllerNumber The controller with a transfer pak.
 * @out outputCart Pointer to cartridge to populate with data.
 */
void initialiseCart(const char controllerNumber, GameboyCart* outputCart);

/**
 * Loads the ROM of the given cartridge in to memory.
 * @param controllerNumber controller the cartidge is plugged in to.
 * @param gbcart Basic information about the cartridge.
 * @out romData ROM of the cartridge is dumped in here.
 */
char importRom(const char controllerNumber, GameboyCart* cart, ByteArray* romData);

/**
 * Loads the Save RAM of the given cartridge in to memory.
 * @param controllerNumber controller the cartidge is plugged in to.
 * @param gbcart Basic information about the cartridge.
 * @out ramData RAM of the cartridge is dumped in here.
 */
char importSave(const char controllerNumber, const GameboyCart cart, ByteArray* saveData);

// TODO
char exportSave(const char controllerNumber, const GameboyCart cart, ByteArray* saveData);
#endif
