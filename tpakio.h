#ifndef TPAKIO_INCLUDED
#define TPAKIO_INCLUDED

#include <stdlib.h>
#include <string.h>

#include "core.h"


/*********************************************************************************************
 * Thanks to Saturnu's (translated) writeup from here:
 * https://circuit-board.de/forum/index.php/Thread/13481-N64-Transfer-Pak-PIF-commands-Reverse-Engineering/
 * *******************************************************************************************/

typedef enum {
    TPAK_SUCCESS = 0,
    // Invalid controller slot (must be 0-3)    
    TPAK_ERR_NO_SLOT = -127,
    // Cartridge header contains invalid/unknown values.    
    TPAK_ERR_INVALID_HEADER,
    // Header failed checksum
    TPAK_ERR_CORRUPT_HEADER,
    // Cartridge failed global checksum
    TPAK_ERR_CORRUPT_DATA,
    // If we loaded this cart we'd probably run out of memory.  An expansion pak may help here.
    TPAK_ERR_INSUFFICIENT_MEMORY,
    // Controller not plugged in.    
    TPAK_ERR_NO_CONTROLLER,
    // Controller detected but no transfer pak.
    TPAK_ERR_NO_TPAK,
    // Transfer pak detected, but no cartridge.
    TPAK_ERR_NO_CARTRIDGE,
    // Transfer pak isn't giving us the initialisation values we expect it to.
    TPAK_ERR_UNKNOWN_BEHAVIOUR,
    // We haven't been able to implement how to read the banks of this memory bank controller yet.
    TPAK_ERR_UNSUPPORTED_CARTRIDGE,
    // Tried to switch to a bank that is not available in this context.
    TPAK_ERR_INVALID_ROM_BANK,
    // libdragon read_mempak_address returned an error code
    // todo - break these down into useful errors we can respond to.
    TPAK_ERR_SYSTEM_ERROR,

} TpakError;

// Must be packed into a single byte to fit in the header
typedef enum __attribute__ ((packed)) {
    // Change this if we do find a use for 0x04
    UNKNOWN_CARTRIDGE_TYPE = 0x04,

    // 32kB ROM
    ROM_ONLY = 0x00,

    // MBC1 - max 2MB ROM and/or 32kB RAM)
    MBC1 = 0x01,
    MBC1_RAM = 0x02, 
    MBC1_BATTERY = 0x03,
    
    // MBC2 - max 256kB ROM and 256B RAM
    MBC2 = 0x05,
    MBC2_BATTERY = 0x06,
    MBC2_RAM = 0x08,
    MBC2_RAM_BATTERY = 0x09,

    // MMO1 - max 8MB ROM and 128kB RAM
    MMM01 = 0x0B,
    MMM01_RAM = 0x0C,
    MMM01_RAM_BATTERY = 0x0D,

    // MBC3 - max 2MB ROM and/or 32kB RAM and Timer
    MBC3_TIMER_BATTERY = 0x0f,
    MBC3_TIMER_RAM_BATTERY = 0x10,
    MBC3 = 0x11,
    MBC3_RAM = 0x012,
    MBC3_RAM_BATTERY = 0x013,

    // MBC5 - max 8MB ROM and/or 128kB RAM
    MBC5 = 0x19,
    MBC5_RAM = 0x1A,
    MBC5_RAM_BATTERY = 0x1B,
    MBC5_RUMBLE = 0x1C,
    MBC5_RUMBLE_RAM = 0x1D,
    MBC5_RUMBLE_RAM_BATTERY = 0x1E,

    // MBC6 - who knows?
    MBC6 = 0x20,
    MBC6_RAM_BATTERY = 0x20,

    // MBC7 - max 8MB ROM or 256kB RAM and Accelerometer
    MBC7_SENSOR_RUMBLE_RAM_BATTERY = 0x22,

    HUC3 = 0xFE,
    HUC1 = 0xFF,
    HUC1_RAM_BATTERY = 0xFF
} CartridgeType;

typedef struct gameboy_cartridge_header CartridgeHeader;

typedef struct {
    ByteArray Rom;
    ByteArray Ram;
    long LastRTCTicks;
    long RTCStopTime;
    long RTCTimeStopped;
    CartridgeHeader Header;
    CartridgeType Type;
    natural RomBankCount;
    byte RamBankCount;
    natural RamBankSize;
    bool IsGbcSupported;
    bool IsRumbling;
    char* Title;
} GameBoyCartridge;

/**
 * Gets the percentage complete that a given TPak cartridge load is.
 * @param controllerNumber controller port we are loading from.
 */
byte getLoadProgress(const byte controllerNumber);

/**
 * Starts or stops the rumble motor for cartridges that have one.
 * @param controllerNumber Slot that rumble-capable cartridge is using.
 * @param isRumbleStart true to start the motor, false to stop it.
 * @returns Error Code
 * @note For now this will switch the transfer pak bank to romx and the ram bank to bank 0.
 *       I think this will be ok because I usually switch to whatever bank I need before any transfer.
 */
sByte toggleRumble(const byte controllerNumber, const bool isRumbleStart);

/**
 * Imports the entire cartridge in to RAM as CartridgeData
 * @param controllerNumber get from T-Pak plugged in to this controller slot.
 * @out cartridge GB/GBC cartridge rom/ram 
 * @returns Error Code
 */
sByte importCartridge(const byte controllerNumber, GameBoyCartridge* cartridge);

/**
 * Imports a cartridge header from the TPAK as well as some derived metadata values.
 * @param controllerNumber number of controller slot cartridge is plugged in to.
 * @out cartridge metadata will be set on the object.
 * @returns Error Code
 */
sByte getCartridgeMetadata(const byte controllerNumber, GameBoyCartridge* cartridge);

/**
 * Sets the cartridge RAM with the data in ramData.
 * @param controllerNumber T-Pak plugged in to this controller slot.
 * @param ramData RAM to copy in to the cartridge.
 * @returns Error code
 */
sByte exportCartridgeRam(const byte controllerNumber, GameBoyCartridge* cartridge);

/**
 * Gets the complete ROM data from the cartridge in a transfer pak.
 * @param controllerNumber T-Pak plugged in to this controller slot.
 * @param cartridge Structure to copy RAM to.
 * @returns Error code
 */
sByte importCartridgeRam(const byte controllerNumber, GameBoyCartridge* cartridge);

#endif