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
//int copy_gbRam_toRAM(const GameboyCart gbcart, uint8_t *ram_data);

GameboyCart initialiseCart(const char controllerNumber);
char importRom(const char controllerNumber, const GameboyCart cart, ByteArray* romData);
char importSave(const char controllerNumber, const GameboyCart cart, ByteArray* saveData);
char exportSave(const char controllerNumber, const GameboyCart cart, ByteArray* saveData);
#endif
