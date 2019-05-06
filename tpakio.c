#include "core.h"
#include "tpakio.h"
#include <libdragon.h>
#include <math.h>

/******************
 * command 00 - joystick status, if a transferpak exists
 * command 03-8001 - set: transfer pak power - on / off (0x84 = on) (0xFE = off)
 * command 02-8001 - query: transfer pak power - on / off (0x84 = on)
 * command 02-B010 - query enable state-Data (0x89 / 0x84) mode 1/0 - (0x00) gb cart missing?
 * command 03-B010 - set access mode 0x00 0x01? for what?
 * command 03-A00C - set: bankswitching via data area - first byte-> banknumber - better fill all
 * command 02-CXXX to FXXX-GB read ROM in 32kbyte chunks - XXX bank-offset
 ******************/

const byte BLOCK_SIZE = 32;
const natural BANK_SIZE = 16 * 1024; // 16kB banks.
const byte HEADER_SIZE = 80;

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

/**
 * Puts Tpak/Cartridge in to a state where it's ready to be read from.
 * @param controllerNumber controller slot the Tpak is plugged in to.
 * @returns Error Code
 */
sByte initialiseTPak(const byte controllerNumber) {
    int accessory = identify_accessory(controllerNumber);

    if (accessory != ACCESSORY_MEMPAK) {
        return TPAK_ERR_NO_TPAK;
    }

    byte block[BLOCK_SIZE];
    sByte result = 0;    

    // Wake up the transfer pak
    memset(block, ENABLE_TPAK, BLOCK_SIZE);
    result = write_mempak_address(controllerNumber, ENABLE_TPAK_ADDRESS, block);
    if (result) {
        return TPAK_ERR_SYSTEM_ERROR;
    }
    read_mempak_address(controllerNumber, ENABLE_TPAK_ADDRESS, block);
    if (block[0] != 0x84) {
        return TPAK_ERR_NO_TPAK;
    }

    // And enable cart mode 1 (which doesn't do anything much according to the cen64 source.)
    memset(block, TPAK_MODE_SET_1, BLOCK_SIZE);
    write_mempak_address(controllerNumber, TPAK_MODE_ADDRESS, block);

    // Do some sanity checks to make sure the tpak is responding as expected.
    memset(block, 0, BLOCK_SIZE);
    read_mempak_address(controllerNumber, TPAK_MODE_ADDRESS, block);
    if (block[0] == TPAK_NO_CART_ERROR) {
        return TPAK_ERR_NO_CARTRIDGE;
    }

    if (block[0] != TPAK_MODE_CHANGED_1) {
        return TPAK_ERR_UNKNOWN_BEHAVIOUR;
    }

    memset(block, 0, BLOCK_SIZE);
    read_mempak_address(controllerNumber, TPAK_MODE_ADDRESS, block);
    if (block[0] != TPAK_MODE_UNCHANGED_1) {
        return TPAK_ERR_UNKNOWN_BEHAVIOUR;
    }    

    return TPAK_SUCCESS;
}

/**
 * Calculates and checks the gameboy header checksum.
 * @param header The header to check.
 * @returns true if checksum passes, false otherwise.
 */
bool checkHeader(const CartridgeHeader* header) {
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
 * Calculates and checks the gameboy global checksum.
 * @param expected The expected result of the check.
 * @param data The data to check.
 * @returns true if values match, false otherwise.
 */
bool checkRom(const natural expected, const ByteArray* data) {
    natural sum = 0;
    for (uLong i = 0; i < data->Size; i++) {
        sum += data->Data[i];
    }

    // Bytes of checksum ignored by algorithm.
    sum -= expected & 0x00FF;
    sum -= expected >> 8;

    return expected == sum;
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
            return TPAK_ERR_SYSTEM_ERROR;
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
 * 
 ** Bank size codes.
 ** 00h - None
 ** 01h - 2 KBytes
 ** 02h - 8 Kbytes
 ** 03h - 32 KBytes (4 banks of 8KBytes each)
 ** 04h - 128 KBytes (16 banks of 8KBytes each)
 ** 05h - 64 KBytes (8 banks of 8KBytes each)
 */
sShort getNumberOfRomBanks(const CartridgeHeader* header) {
    if (header->RomSizeCode <= 8) {
        return pow(2, header->RomSizeCode + 1);
    } else {
        switch(header->RomSizeCode) {
            case 0x52: return 72;
            case 0x53: return 80;
            case 0x54: return 96;
        }
    }

    return -1;
}

/**
 * Determines the number of RAM banks from the Ram Size code in the cartridge header. 
 * @param header the header.
 * @returns Number of banks, or -1 if unknown code.
 */
sByte getNumberOfRamBanks(const CartridgeHeader* header) {
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
natural getRamBankSize(const CartridgeHeader* header) {
    switch(header->RamSizeCode) {
        case 0: return 0;
        case 1: return 2 * 1024;
        default: return 8 * 1024;
    }
}

/**
 * Gets the complete ROM data from the cartridge a transfer pak.
 * @param controllerNumber T-Pak plugged in to this controller slot.
 * @param cartridge Structure to copy ROM to.
 * @returns Error codes
 ** 0  - Successful
 ** -1 - Error
 */
sByte importRom(const byte controllerNumber, GameBoyCartridge* cartridge) {
    cartridge->Rom.Size = BANK_SIZE * cartridge->RomBankCount;
    if (cartridge->Rom.Data) {
        free(cartridge->Rom.Data);
    }
    cartridge->Rom.Data = calloc(cartridge->Rom.Size, 1);

    // Loop through each bank
    for (natural bank = 0; bank < cartridge->RomBankCount; bank++) {
        // Set bank
        byte block[BLOCK_SIZE];
        memset(block, bank, BLOCK_SIZE);
        write_mempak_address(controllerNumber, TPAK_BANK_SWITCH_ADDRESS, block);

        // Read into memory 32bytes at a time.
        for (natural address = 0; address < BANK_SIZE; address += BLOCK_SIZE) {
            read_mempak_address(controllerNumber, ROM_ADDRESS_OFFSET + address, cartridge->Rom.Data + (bank * BANK_SIZE) + address);
        }
    }

    return TPAK_SUCCESS;
}

/**
 * Gets the complete ROM data from the cartridge in a transfer pak.
 * @param controllerNumber T-Pak plugged in to this controller slot.
 * @param cartridge Structure to copy RAM to.
 * @returns Error codes
 **  0 - Successful
 ** -1 - Error
 */
sByte importCartridgeRam(const byte controllerNumber, ByteArray* cartridge) {
    return -1;
}
/**
 * Sets the cartridge RAM with the data in ramData.
 * @param controllerNumber T-Pak plugged in to this controller slot.
 * @param ramData RAM to copy in to the cartridge.
 * @returns Error codes
 **  0 - Successful
 ** -1 - Error
 ** -2 - Invalid controller slot (must be 0-3) 
 */
sByte exportCartridgeRam(const byte controllerNumber, ByteArray* ramData) {
    if (controllerNumber > 3) {
        return TPAK_ERR_NO_SLOT;
    }
    return -1;
}

/**
 * Imports a cartridge header from the TPAK as well as some derived metadata values.
 * @param controllerNumber number of controller slot cartridge is plugged in to.
 * @out cartridge metadata will be set on the object.
 * @returns Error Code
 */
sByte getCartridgeMetadata(const byte controllerNumber, GameBoyCartridge* cartridge) {
    if (controllerNumber > 3) {
        return TPAK_ERR_NO_SLOT;
    }

    sByte result = initialiseTPak(controllerNumber);
    if (result) {
        return result;
    }

    CartridgeHeader header;
    result = getHeader(controllerNumber, &header);
    if (result) {
        return result;
    }

    if (!checkHeader(&header)) {
        return TPAK_ERR_CORRUPT_HEADER;
    }

    cartridge->IsGbcSupported = header.CGBTitle.GbcSupport == GBC_DMG_SUPPORTED || header.CGBTitle.GbcSupport == GBC_ONLY_SUPPORTED;        

    sShort romBanks = getNumberOfRomBanks(&header);
    sByte ramBanks = getNumberOfRamBanks(&header);
    sInt ramBankSize = getRamBankSize(&header); 

    if (romBanks * BANK_SIZE * 3 >= getMemoryLimit()) {
        return TPAK_ERR_INSUFFICIENT_MEMORY;
    }

    if (romBanks < 0 || ramBanks < 0 || ramBankSize < 0) {
        return TPAK_ERR_INVALID_HEADER; 
    }

    cartridge->RomBankCount = romBanks;
    cartridge->RamBankCount = ramBanks;
    cartridge->RamBankSize = (natural) ramBankSize;     

    memcpy(&cartridge->Header, &header, HEADER_SIZE);

    return TPAK_SUCCESS;
}

/**
 * Imports the entire cartridge in to RAM as CartridgeData
 * @param controllerNumber get from T-Pak plugged in to this controller slot.
 * @out catridge GB/GBC catridge rom/ram 
 * @returns Error Code
 */
sByte importCartridge(const byte controllerNumber, GameBoyCartridge* cartridge) {
    sByte result = getCartridgeMetadata(controllerNumber, cartridge);
    if (result) {
        return result;
    }

    result = importRom(controllerNumber, cartridge);
    if (result) {
        return result;
    }

    if (!checkRom(cartridge->Header.GlobalChecksum, &cartridge->Rom)) 
    {
        return TPAK_ERR_CORRUPT_DATA;
    }

    return TPAK_SUCCESS;
}