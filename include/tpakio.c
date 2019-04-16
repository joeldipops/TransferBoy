#include "tpakio.h"
#include <libdragon.h>

/******************
 * command 00 - joystick status, if a transferpak exists
 * command 03-8001 - set: transfer pak power - on / off (0x84 = on) (0xFE = off)
 * command 02-8001 - query: transfer pak power - on / off (0x84 = on)
 * command 02-B010 - query enable state-Data (0x89 / 0x84) mode 1/0 - (0x00) gb cart missing?
 * command 03-B010 - set access mode 0x00 0x01? for what?
 * command 03-A00C - set: bankswitching via data area - first byte-> banknumber - better fill all
 * command 02-CXXX to FXXX-GB read ROM in 32kbyte chunks - XXX bank-offset
 ******************/

// For some
const byte BLOCK_SIZE = 32;

// According to the cen64 source, address is 0x8000 and the 1 is from a 5 bit cyclic redundancy check
const natural ENABLE_TPAK_ADDRESS = 0x8001; 
const byte ENABLE_TPAK = 0x84;
const byte DISABLE_TPAK = 0xFE;

// Likewise, address is 0xB000 + 5 bit CRC
const natural TPAK_MODE_ADDRESS = 0xB010;
// No-one in the homebrew community seems to have an explanation for what this mode
// is for, but can be used to check that everything is working properly.
const byte TPAK_MODE_SET_0 = 0x00;
const byte TPAK_MODE_SET_1 = 0x01;

const byte TPAK_MODE_UNCHANGED_0 = 0x80;
const byte TPAK_MODE_CHANGED_0 = 0x84;
const byte TPAK_MODE_UNCHANGED_1 = 0x89;
const byte TPAK_MODE_CHANGED_1 = 0x8D;

const byte TPAK_NO_CART_ERROR = 0x40;

// Address is 0xA000...
const natural TPAK_BANK_SWITCH_ADDRESS = 0xA00C;


const natural ROM_ADDRESS_OFFSET = 0xC000;

typedef enum { GBC_NOT_SUPPORTED, GBC_DMG_SUPPORTED, GBC_ONLY_SUPPORTED } GbcSupport;

typedef enum {
    // 32kB ROM
    ROM_ONLY,

    // MBC1 - max 2MB ROM and/or 32kB RAM)
    MBC1,
    MBC1_RAM,
    MBC1_BATTERY,
    
    // MBC2 - max 256kB ROM and 256B RAM
    MBC2 = 0x05,
    MBC2_BATTERY,
    MBC2_RAM = 0x08,
    MBC2_RAM_BATTERY,

    // MMO1 - max 8MB ROM and 128kB RAM
    MMM01 = 0x0b,
    MMM01_RAM,
    MMM01_RAM_BATTERY,

    // MBC3 - max 2MB ROM and/or 32kB RAM and Timer
    MBC3_TIMER_BATTERY = 0x0f,
    MBC3_TIMER_RAM_BATTERY,
    MBC3,
    MBC3_RAM,
    MBC3_RAM_BATTERY,

    // MBC5 - max 8MB ROM and/or 128kB RAM
    MBC5 = 0x19,
    MBC5_RAM,
    MBC5_RAM_BATTERY,
    MBC5_RUMBLE,
    MBC5_RUMBLE_RAM,
    MBC5_RUMBLE_RAM_BATTER,

    // MBC6 - who knows?
    MBC6 = 0x20,

    // MBC7 - max 8MB ROM or 256kB RAM and Accelerometer
    MBC7_SENSOR_RUMBLE_RAM_BATTERY = 0x22    
} CartridgeType;

typedef struct {
    byte entryScript[4];
    byte logo[48];
    union {
        byte DMGTitle[16];
        struct {
            byte DMGTitle[15];            
            GbcSupport gbcSupport;
        } OldCGBTitle;
        struct {
            byte CGBTitle[11];
            byte ManufacturerCode[4];
            GbcSupport gbcSupport;            
        } CGBTitle;
    };
    natural NewLicenseeCode;
    bool IsSgbSupported;
    CartridgeType CartridgeType;
    byte RomSizeCode;
    byte RamSizeCode;
    byte Destination;
    byte OldLicenseeCode;
    byte VersionNumber;
    byte HeaderChecksum;
    natural GlobalChecksum;
    byte overflow[16];
} CartridgeHeader;

/**
 * Puts Tpak/Cartridge in to a state where it's ready to be read from.
 * @param controllerNumber controller slot the Tpak is plugged in to.
 * @returns Error Code
 *  0 - Success
 * -1 - Controller not plugged in.
 * -2 - Transfer pak not detected.
 * -3 - Data corruption.
 * -4 - No cartridge inserted into TPAK.
 * -5 - Tpak not behaving as expected.
 */
sByte initialiseTPak(const u8 controllerNumber) {
    int accessory = identify_accessory(controllerNumber);

    if (accessory != ACCESSORY_MEMPAK) {
        return -2;
    }

    byte block[32];
    sByte result = 0;    

    // Wake up the transfer pak
    memset(block, ENABLE_TPAK, BLOCK_SIZE);
    result = write_mempak_address(controllerNumber, ENABLE_TPAK_ADDRESS, block);
    if (result) {
        return result;
    }
    read_mempak_address(controllerNumber, ENABLE_TPAK_ADDRESS, block);
    if (block[0] != 0x84) {
        return -2;
    }

    // And enable cart mode 1 (which doesn't do anything much according to the cen64 source.)
    memset(block, TPAK_MODE_SET_1, BLOCK_SIZE);
    write_mempak_address(controllerNumber, TPAK_MODE_ADDRESS, block);

    // Do some sanity checks to make sure the tpak is responding as expected.
    memset(block, 0, BLOCK_SIZE);
    read_mempak_address(controllerNumber, TPAK_MODE_ADDRESS, block);
    if (block[0] == TPAK_NO_CART_ERROR) {
        return -4;
    }

    if (block[0] != TPAK_MODE_CHANGED_1) {
        return -5;
    }

    memset(block, 0, BLOCK_SIZE);
    read_mempak_address(controllerNumber, TPAK_MODE_ADDRESS, block);
    if (block[0] != TPAK_MODE_UNCHANGED_1) {
        return -5;
    }    

    return 0;
}



/**
 * Calculates and checks the gameboy header checksum.
 * @param header The header to check.
 * @returns true if checksum passes, false otherwise.
 */
bool checkHeader(CartridgeHeader* header) {
    byte sum = 0;
    byte* data = (byte*) header;

    // sum values from 0x0134 (title) to 0x014C (version number)
    const byte start = 0x34;
    const byte end = 0x4C;
    for(byte i = start; i <= end; i++) {
        sum = sum - data[i] - 1;
    }
    
    return sum == header->HeaderChecksum;
}

/**
 * Reads the gameboy header so we know what banks to switch etc.
 * Call after initialise TPak
 * @param controllerNumber controller slot the T-Pak is plugge in to.
 * @out header header data will be populated here.
 * @returns ErrorCode
 ** 0 - Successful
 */
sByte getHeader(const byte controllerNumber, CartridgeHeader* header) {
    // Set bank 0
    byte block[BLOCK_SIZE];
    memset(block, 0, BLOCK_SIZE);
    write_mempak_address(controllerNumber, TPAK_BANK_SWITCH_ADDRESS, block);
    
    // Header starts at 0x0100 in bank 0 and goes for 80 bytes (rounded up to 96)
    natural address = ROM_ADDRESS_OFFSET + 0x0100;
    
    byte offset = 0;
    byte* headerData = (byte*) header;

    for(byte i = 0; i < 3; i++) {
        sByte result = read_mempak_address(controllerNumber, address + offset, headerData + offset);
        if (result) {
            return result;
        }
        offset += BLOCK_SIZE;
    }

    return 0;
}

/**
 * Determines the number of rom banks from the Rom Size Code
 * in the cartridge header.
 * @param header The header.
 * @returns Number of 16kB rom banks (including the first, fixed bank)
 * or -1 if unknown code.
 */
sShort getNumberOfRomBanks(CartridgeHeader* header) {
    if (header->RomSizeCode <= 8) {
        return 2 ^ (header->RomSizeCode + 1);
    } else {
        switch(header->RomSizeCode) {
            case 0x52: return 72;
            case 0x53: return 80;
            case 0x54: return 96;
        }
    }

    return -1;
}

 /*********************************
  * Bank size codes.
  * 00h - None
  * 01h - 2 KBytes
  * 02h - 8 Kbytes
  * 03h - 32 KBytes (4 banks of 8KBytes each)
  * 04h - 128 KBytes (16 banks of 8KBytes each)
  * 05h - 64 KBytes (8 banks of 8KBytes each)
  **********************************/

/**
 * Determines the number of RAM banks from the Ram Size code in the cartridge header. 
 * @param header the header.
 * @returns Number of banks, or -1 if unknown code.
 */
sByte getNumberOfRamBanks(CartridgeHeader* header) {
    switch(header->RamSizeCode) {
        case 0: return 0;
        case 1: return 1;
        case 2: return 1;
        case 3: return 4;
        case 4: return 16;
        case 5: return 8;
    }

    return -1;
}

/**
 * Determines the size of each RAM bank from the Ram size code in the cartridge header.
 * @param header the header.
 * @returns the size of each ram bank.
 */
natural getRamBankSize(CartridgeHeader* header) {
    switch(header->RamSizeCode) {
        case 0: return 0;
        case 1: return 2 * 1024;
        default: return 8 * 1024;
    }
}

/**
 * Imports the entire cartridge in to RAM as CartridgeData
 * @param controllerNumber get from T-Pak plugged in to this controller slot.
 * @out catridge GB/GBC catridge rom/ram 
 * @returns Error Code
 **   0 - Successful
 **  -1 - Invalid controller slot (must be 1-4)
 **  -2 - Cartridge header contains invalid values.
 **  -3 - Header failed checksum
 ** -10 - Controller not plugged in.
 ** -20 - Transfer pak not detected.
 ** -30 - Data corruption.
 ** -40 - No cartridge in Tpak.
 ** -50 - Tpak not behaving as expected.
 */
sByte importCartridge(byte controllerNumber, CartridgeData* cartridge) {
    if (controllerNumber <= 0 || controllerNumber > 4) {
        return -1;
    }

    controllerNumber -= 1;

    sByte result = initialiseTPak(controllerNumber);
    if (result) {
        return result * 10;
    }

    CartridgeHeader header;
    if (getHeader(controllerNumber, &header)) {
        return result * 10;
    }

    if (!checkHeader(&header)) {
        return -3;
    }

    sShort romBanks = getNumberOfRomBanks(&header);
    sByte ramBanks = getNumberOfRamBanks(&header);
    natural ramBankSize = getRamBankSize(&header); 

    if (romBanks < 0 || ramBanks < 0 || ramBankSize < 0) {
        return -2; 
    }
    return 0;
}


/**
 * Gets the complete ROM data from the cartridge a transfer pak.
 * @param controllerNumber T-Pak plugged in to this controller slot.
 * @param romData ROM will be copied to this address.
 * @returns Error codes
 ** 0  - Successful
 ** -1 - Error
 */
sByte getRom(const byte controllerNumber, ByteArray* romData) {
    return -1;
}

/**
 * Gets the complete ROM data from the cartridge in a transfer pak.
 * @param controllerNumber T-Pak plugged in to this controller slot.
 * @param ramData RAM will be copied to this address.
 * @returns Error codes
 **  0 - Successful
 ** -1 - Error
 */
sByte getRam(const byte controllerNumber, ByteArray* ramData) {
    return -1;
}

/**
 * Sets the cartridge RAM with the data in ramData.
 * @param controllerNumber T-Pak plugged in to this controller slot.
 * @param ramData RAM to copy in to the cartridge.
 * @returns Error codes
 **  0 - Successful
 ** -1 - Error
 ** -2 - Invalid controller slot (must be 1-4) 
 */
sByte setRam(const u8 controllerNumber, ByteArray* ramData) {
    if (controllerNumber <= 0 || controllerNumber > 4) {
        return -2;
    }
    return -1;
}
