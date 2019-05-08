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
const natural ENABLE_TPAK_ADDRESS = 0x8000; 
const byte ENABLE_TPAK = 0x84;
const byte DISABLE_TPAK = 0xFE;

// Likewise, address is 0xB010 including 5 bit CRC
const natural TPAK_MODE_ADDRESS = 0xB000;
// No-one in the homebrew community seems to have an explanation for what this mode
// is for, but can be used to check that everything is working properly.
const byte TPAK_MODE_SET_0 = 0x00;
const byte TPAK_MODE_SET_1 = 0x01;

const byte TPAK_MODE_UNCHANGED_0 = 0x80;
const byte TPAK_MODE_CHANGED_0 = 0x84;
const byte TPAK_MODE_UNCHANGED_1 = 0x89;
const byte TPAK_MODE_CHANGED_1 = 0x8D;

const byte TPAK_NO_CART_ERROR = 0x40;

// The transfer pak has 4 16kB banks in its address space (0xC000 - 0xFFFF)
// for four chunks of the gameboy address space.
const natural TPAK_BANK_ADDRESS = 0xA000; // 0xA00C?

// And the cartridge also has banks switchable at 0x4000-0x8000 of gb address space.
// So to access mbc1 bank 2:
// 1) set TPAK_BANK to 0 ("ROM0"), so we can access gb 0x2000
// 2) set GB_BANK to 2
// 3) set TPAK_BANK to 1 so we can access "ROMX" (ie 0x4000 - 0x8000, the banked rom section)
// 4) read from 0xC000 - 0xFFFF, where 0xC000 is equivalent to 0x4000 of the gb memory map

// GB_ROM_BANK_ADDRESS is only a 5bit register.  For banks higher than 1F we also
// need to set 0x4000 for the upper two bits.
typedef enum {
    ROM0 = 0,
    ROMX = 1,
    SRAM = 2,
    WRAM = 3
} TpakBanks;

const natural ENABLE_GB_RAM_ADDRESS = 0x0000;
const natural ENABLE_GB_RAM = 0x000A;

const natural GB_ROM_BANK_ADDRESS = 0x2000;
const natural GB_RAM_BANK_ADDRESS = 0x4000;
const natural GB_BANK_MODE_ADDRESS = 0x6000;
const byte GB_ROM_BANK_MODE = 0;
const byte GB_RAM_BANK_MODE = 1;

const natural SRAM_ADDRESS = 0xA000;

const natural ROM_ADDRESS_OFFSET = 0xC000;

/**
 * Helper for when we just want to configure some single value, such as enabling the tpak, or switching banks.
 * Don't use this if you actually want to write to cartridge RAM.
 * @param controllerNumber Identifies the tpak to configure
 * @param address Address of the setting to update.
 * @param value New value to set.
 * @returns Error Code from write_mempak_address.
 */
sByte setTpakValue(const byte controllerNumber, const natural address, const byte value) {
    byte block[BLOCK_SIZE];
    memset(block, value, BLOCK_SIZE);
    return write_mempak_address(controllerNumber, address, block);
}

/**
 * For a given gameboy memory space address, determines the respective TPAK memory space address.
 * @param address An address in gameboy memory space.
 * @returns The corresponding tpak memory space address.
 */
natural mapAddress(const natural address) {
    natural offset = ROM_ADDRESS_OFFSET;
    if (address < 0x4000) {
        return address + offset;
    } else if (address < 0x8000) {
        offset -= 0x4000;
    } else if (address < 0xC000) {
        offset -= 0x8000;
    } else {
        offset = 0;
    }
    return address + offset;
}

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
    result = setTpakValue(controllerNumber, ENABLE_TPAK_ADDRESS, ENABLE_TPAK);
    if (result) {
        return TPAK_ERR_SYSTEM_ERROR;
    }
    read_mempak_address(controllerNumber, ENABLE_TPAK_ADDRESS, block);
    if (block[0] != 0x84) {
        return TPAK_ERR_NO_TPAK;
    }

    // And enable cart mode 1 (which doesn't do anything much according to the cen64 source.)
    setTpakValue(controllerNumber, TPAK_MODE_ADDRESS, TPAK_MODE_SET_1);

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

    // Enable Cartridge ram
    setTpakValue(controllerNumber, mapAddress(ENABLE_GB_RAM_ADDRESS), ENABLE_GB_RAM);

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
 * Switches what cartridge SRAM bank appears at 0xA000 - 0xBFFF of cart space
 * and switches the TPAK to map that location.
 * @param controllerNumber Controller slot tpak is plugged in to.
 * @param bank Bank number to switch to. 
 * @returns Error code.
 */
sByte switchRamBank(const byte controllerNumber, const natural bank) {
    // Make sure we're in ROMX to access 0x4000 and 0x6000
    setTpakValue(controllerNumber, TPAK_BANK_ADDRESS, ROMX);

    // Switch to RAM mode except for 0 bank which is accesible in ROM mode.
    if (bank > 0) {
        setTpakValue(controllerNumber, mapAddress(GB_BANK_MODE_ADDRESS), GB_RAM_BANK_MODE);
    }

    // Set the bank
    setTpakValue(controllerNumber, mapAddress(GB_RAM_BANK_ADDRESS), bank);

    // Switch the TPAK bank so the SRAM location is available.
    setTpakValue(controllerNumber, TPAK_BANK_ADDRESS, SRAM);

    return 0;
}

/**
 * Switches what cartridge ROM bank appears at 0x4000 - 0x7FFF of cart space
 * and switches the TPAK to map that location.
 * @param controllerNumber Controller slot tpak is plugged in to.
 * @param bank Bank number to switch to. 
 * @returns Error code.
 */
sByte switchBank(const byte controllerNumber, const natural bank) {
    if (bank == 0) {
        // Bank 0 is always ROM0
        setTpakValue(controllerNumber, TPAK_BANK_ADDRESS, ROM0);
    } else {
        // Tpak needs to map to ROMX for the following.
        setTpakValue(controllerNumber, TPAK_BANK_ADDRESS, ROMX);

        if (bank >= 32) {
            // Ensure we are in ROM bank mode.
            setTpakValue(controllerNumber, mapAddress(GB_BANK_MODE_ADDRESS), GB_ROM_BANK_MODE);
        }

        // Set upper two bits of the bank number at this address.
        setTpakValue(controllerNumber, mapAddress(GB_RAM_BANK_ADDRESS), (bank & 0x60) >> 5);

        // Switch back to ROM0
        setTpakValue(controllerNumber, TPAK_BANK_ADDRESS, ROM0);

        // And set lower five bits of the bank number.
        setTpakValue(controllerNumber, mapAddress(GB_ROM_BANK_ADDRESS), bank & 0x1F);        

        // GB Bank should have switched now, so switch the TPAK bank so we can access it.
        setTpakValue(controllerNumber, TPAK_BANK_ADDRESS, ROMX);        
    }

    return 0;
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
    // Set tpak bank to 0 since the header is at 0x0100
    switchBank(controllerNumber, 0);
    
    // Header starts at 0x0100 and goes for 80 bytes (rounded up to 96)
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
        switchBank(controllerNumber, bank);

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
sByte importCartridgeRam(const byte controllerNumber, GameBoyCartridge* cartridge) {
    cartridge->Ram.Size = cartridge->RamBankSize * cartridge->RamBankCount;
    if (cartridge->Ram.Data) {
        free(cartridge->Ram.Data);
    }
    cartridge->Ram.Data = calloc(cartridge->Ram.Size, 1);

    // Loop through each bank
    for (byte bank = 0; bank < cartridge->RamBankCount; bank++) {
        switchRamBank(controllerNumber, bank);

        // Read into memory 32bytes at a time.
        for (natural address = 0; address < cartridge->RamBankSize; address += BLOCK_SIZE) {
            read_mempak_address(controllerNumber, mapAddress(SRAM_ADDRESS) + address, cartridge->Ram.Data + (bank * cartridge->RamBankSize) + address);
        }
    }

    return TPAK_SUCCESS;
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
sByte exportCartridgeRam(const byte controllerNumber, GameBoyCartridge* ramData) {
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

    result = importCartridgeRam(controllerNumber, cartridge);
    if (result) {
        return result;
    }

    return TPAK_SUCCESS;
}