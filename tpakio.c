#include "core.h"
#include "tpakio.h"
#include "config.h"
#include "hwdefs.h"
#include "state.h"

#include <libdragon.h>
#include <math.h>

const byte BLOCK_SIZE = 32;
const natural BANK_SIZE = 16 * 1024; // 16kB banks.
const byte HEADER_SIZE = 80;

const natural DETECT_DEVICE_ADDRESS = 0x8000;
const natural RUMBLE_ADDRESS = 0xC000;
const byte RUMBLE_PAK = 0x80;
const natural CARTRIDGE_POWER_ADDRESS = 0x8000;
const byte CARTRIDGE_POWER_ON = 0x84;
const byte CARTRIDGE_POWER_OFF = 0xFE;

// The transfer pak has 4 16kB banks in its address space (0xC000 - 0xFFFF)
// for four chunks of the gameboy address space.
const natural TPAK_BANK_ADDRESS = 0xA000;

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

const natural GB_ROM_BANK_ADDRESS = 0x2100;
const natural GB_UPPER_ROM_BANK_ADDRESS = 0x3000;
const natural GB_RAM_BANK_ADDRESS = 0x4000;
const natural GB_BANK_MODE_ADDRESS = 0x6000;
const natural GB_RTC_LATCH_ADDRESS = 0x6000;
const byte GB_ROM_BANK_MODE = 0;
const byte GB_RAM_BANK_MODE = 1;

const natural SRAM_ADDRESS = 0xA000;

const natural TPAK_STATUS_ADDRESS = 0xB000;

#define OS_GBPAK_RSTB_DETECTION 0x04
#define OS_GBPAK_RSTB_STATUS    0x08

const byte TPAK_STATUS_ACCESS_ON = 0x01; // bit 0
const byte TPAK_STATUS_RESET_DETECTED = 0x04; // bit 2
const byte TPAK_STATUS_RESET_STATUS = 0x08; // bit 3
const byte TPAK_STATUS_CARTRIDGE_ABSENT = 0x40; // bit 6
const byte TPAK_STATUS_CARTRIDGE_POWERED = 0x80; // bit 7

const natural ROM_ADDRESS_OFFSET = 0xC000;

byte _progress[MAX_PLAYERS] = {0};

/**
 * Gets whether this cartridge has an RTC installed on it.
 * @param header Cartridge metadata.
 * @returns true if RTC present, false otherwise.
 */
static bool hasRealTimeClock(CartridgeHeader* header) {
    switch (header->cartridge_type) {
        case MBC3_TIMER_BATTERY:
        case MBC3_TIMER_RAM_BATTERY:
            return true;
        default:
            return false;
    }
}

/**
 * Helper for when we just want to configure some single value, such as enabling the tpak, or switching banks.
 * Don't use this if you actually want to write to cartridge RAM.
 * @param controllerNumber Identifies the tpak to configure
 * @param address Address of the setting to update.
 * @param value New value to set.
 * @returns Error Code from write_mempak_address.
 */
static sByte setTpakValue(const byte controllerNumber, const natural address, const byte value) {
    byte block[BLOCK_SIZE];
    memset(block, value, BLOCK_SIZE);
    return write_mempak_address(controllerNumber, address, block);
}

/**
 * For a given gameboy memory space address, determines the respective TPAK memory space address.
 * @param address An address in gameboy memory space.
 * @returns The corresponding tpak memory space address.
 */
static natural mapAddress(const natural address) {
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
static sByte initialiseTPak(const byte controllerNumber) {
    sByte result = tpak_init(controllerNumber);

    // Enable Cartridge ram
    setTpakValue(controllerNumber, mapAddress(ENABLE_GB_RAM_ADDRESS), ENABLE_GB_RAM);

    return result ? result : TPAK_SUCCESS;
}

/**
 * Calculates the checksum of an address and assigns it to that address.
 * @param address The address
 * @returns The address OR'd with the checksum.
 */
static natural writeCRC(natural address) {
    byte values[] = { 0x15, 0x1F, 0x0B, 0x16, 0x19, 0x07, 0x0E, 0x1C, 0x0D, 0x1A, 0x01 };
    address &= 0xFFE0;
    byte checksum = 0;
    for (byte i = 15; i > 4; i--) {
        if ((address >> i) & 1) {
            checksum ^= values[i - 5];
        }
    }
    return address | checksum;
}

/**
 * Calculates and checks the gameboy global checksum.
 * @param expected The expected result of the check.
 * @param data The data to check.
 * @returns true if values match, false otherwise.
 */
static bool checkRom(const natural expected, const ByteArray* data) {
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
static sByte switchRamBank(const byte controllerNumber, const natural bank, const CartridgeType type) {
    // Make sure we're in ROMX to access 0x4000 and 0x6000
    setTpakValue(controllerNumber, TPAK_BANK_ADDRESS, ROMX);

    // Switch to RAM mode except for 0 bank which is accesible in ROM mode.
    if (type == MBC1 && bank > 0) {
        setTpakValue(controllerNumber, mapAddress(GB_BANK_MODE_ADDRESS), GB_RAM_BANK_MODE);
    }

    // Set the bank
    setTpakValue(controllerNumber, mapAddress(GB_RAM_BANK_ADDRESS), bank);

    // Switch the TPAK bank so the SRAM location is available.
    setTpakValue(controllerNumber, TPAK_BANK_ADDRESS, SRAM);

    return 0;
}

/**
 * Getting banks 0x20, 0x40 and 0x60 on an MBC1 requires some voodoo.
 * This switches us to ROM0 so it's immediately accessible.
 * @param controllerNumber Controller slot tpak is plugged in to.
 * @param bank Bank number to switch to.
 * @returns Error code.
 */
static sByte switchROM0Bank(const byte controllerNumber, const natural bank, const CartridgeType type) {
    // If not one of those three banks, don't try this.
    if (bank & 0x9F) {
        return TPAK_ERR_INVALID_ROM_BANK;
    }    

    if (type != MBC1) {
        return TPAK_ERR_UNSUPPORTED_CARTRIDGE;
    }

    // Make sure we're in ROMX space
    setTpakValue(controllerNumber, TPAK_BANK_ADDRESS, ROMX);
    // Switch to 'RAM' mode
    setTpakValue(controllerNumber, mapAddress(GB_BANK_MODE_ADDRESS), GB_RAM_BANK_MODE);
    // Write to upper bank address, bank is the two bits << 5
    setTpakValue(controllerNumber, mapAddress(GB_RAM_BANK_ADDRESS), bank >> 5);
    // And move TPAK bank back to ROM0, since that's where these banks get put.
    // GO FIGURE
    setTpakValue(controllerNumber, TPAK_BANK_ADDRESS, ROM0);

    return 0;
}

/**
 * Switches what cartridge ROM bank appears at 0xC000 - 0xFFFF of address space
 * and switches the TPAK to map that location.
 * @param controllerNumber Controller slot tpak is plugged in to.
 * @param bank Bank number to switch to. 
 * @returns Error code.
 */
static sByte switchBank(const byte controllerNumber, const natural bank, const CartridgeType type) {
    if (bank == 0) {
        setTpakValue(controllerNumber, TPAK_BANK_ADDRESS, ROM0);
    } else {
        if (type == MBC1) {
            // Tpak needs to map to ROMX for the following.
            setTpakValue(controllerNumber, TPAK_BANK_ADDRESS, ROMX);
            if (bank >= 32) {
                // Special voodoo required for these three banks.
                if (bank == 0x20 || bank == 0x40 || bank == 0x60) {
                    return switchROM0Bank(controllerNumber, bank, type);
                } else {
                    // Ensure we are in ROM bank mode.
                    setTpakValue(controllerNumber, mapAddress(GB_BANK_MODE_ADDRESS), GB_ROM_BANK_MODE);
                }
            }

            // Set upper two bits of the bank number at this address.
            setTpakValue(controllerNumber, mapAddress(GB_RAM_BANK_ADDRESS), (bank & 0x60) >> 5);  
        } else if (type == MBC5) {
            // The upper bit is set at 0x3000
            setTpakValue(controllerNumber, mapAddress(GB_UPPER_ROM_BANK_ADDRESS), (bank & 0x0100) >> 8);
        }

        // Ensure we are in ROM0
        setTpakValue(controllerNumber, TPAK_BANK_ADDRESS, ROM0);          

        byte mask = 0;
        switch (type) {
            case MBC1: mask = 0x1F;
                break;
            case MBC2: mask = 0x0F;
                break;
            case MBC3: mask = 0x7F;
                break;
            case MBC5: mask = 0xFF;
                break;
            default:
                break;
        }

        // And set lower bits of the bank number.
        setTpakValue(controllerNumber, mapAddress(GB_ROM_BANK_ADDRESS), bank & mask);        

        // GB Bank should have switched now, so switch the TPAK bank so we can access it.
        setTpakValue(controllerNumber, TPAK_BANK_ADDRESS, ROMX);        
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
static sShort getNumberOfRomBanks(const CartridgeHeader* header) {
    if (header->rom_size_code <= 8) {
        return pow(2, header->rom_size_code + 1);
    } else {
        switch(header->rom_size_code) {
            case 0x52: return 72;
            case 0x53: return 80;
            case 0x54: return 96;
        }
    }

    return -1;
}

/**
 * Determines the number of RAM banks from the Ram Size code in the cartridge header. 
 * @param cartridge the cartridge.
 * @returns Number of banks, or -1 if unknown code.
 */
static sByte getNumberOfRamBanks(const CartridgeHeader* header, const CartridgeType type) {
    // MBC2s do have RAM, but it doesn't show up in the cartridge header.
    if (type == MBC2) {
        return 1;
    }
    switch(header->ram_size_code) {
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
static natural getRamBankSize(const CartridgeHeader* header, const CartridgeType type) {
    switch(header->ram_size_code) {
        case 0: 
            if (type == MBC2) {
                return 512;
            } else {
                return 0;
            }
        case 1: return 2 * 1024;
        default: return 8 * 1024;
    }
}

/**
 * Given the specific type of a cartridge eg MBC1+RAM+BATTERY and returns the general MBC type eg MBC1.
 * @param fullType A cartridge type.
 * @returns The Memory Bank Controller type.
 */
static CartridgeType getPrimaryType(const CartridgeType fullType) {
    switch(fullType) {
        case ROM_ONLY:
            return ROM_ONLY;

        case MBC1:
        case MBC1_BATTERY:
        case MBC1_RAM:
            return MBC1;

        case MBC2:
        case MBC2_BATTERY:
        case MBC2_RAM:
        case MBC2_RAM_BATTERY:
            return MBC2;

        case MBC3:
        case MBC3_RAM:
        case MBC3_RAM_BATTERY:
        case MBC3_TIMER_BATTERY:
        case MBC3_TIMER_RAM_BATTERY:
            return MBC3;

        case MBC5:
        case MBC5_RAM:
        case MBC5_RAM_BATTERY:
        case MBC5_RUMBLE:
        case MBC5_RUMBLE_RAM:
        case MBC5_RUMBLE_RAM_BATTERY:
            return MBC5;

        case HUC1_RAM_BATTERY:
        case MMM01:
        case MMM01_RAM:
        case MMM01_RAM_BATTERY:
        default:
            return UNKNOWN_CARTRIDGE_TYPE;


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
static sByte importRom(const byte controllerNumber, GameBoyCartridge* cartridge) {
    _progress[controllerNumber] = 0;

    cartridge->Rom.Size = BANK_SIZE * cartridge->RomBankCount;
    if (cartridge->Rom.Data) {
        free(cartridge->Rom.Data);
    }
    cartridge->Rom.Data = calloc(cartridge->Rom.Size, 1);

    // Loop through each bank
    for (natural bank = 0; bank < cartridge->RomBankCount; bank++) {
        switchBank(controllerNumber, bank, cartridge->Type);

        // Read into memory 32bytes at a time.
        for (natural address = 0; address < BANK_SIZE; address += BLOCK_SIZE) {
            read_mempak_address(controllerNumber, ROM_ADDRESS_OFFSET + address, cartridge->Rom.Data + (bank * BANK_SIZE) + address);

            _progress[controllerNumber] = ((float)(bank * BANK_SIZE + address) / (float)cartridge->Rom.Size) * 100;
        }


    }

    return TPAK_SUCCESS;
}

static sByte exportRTCData(const byte controllerNumber, GameBoyCartridge* cartridge) {
    // Change to ROM0 and ensure SRAM is enabled.
    setTpakValue(controllerNumber, TPAK_BANK_ADDRESS, ROM0);
    setTpakValue(controllerNumber, mapAddress(ENABLE_GB_RAM_ADDRESS), ENABLE_GB_RAM);

    for (byte bank = 0x08; bank < 0x0D; bank++) {
        // Update the bank
        setTpakValue(controllerNumber, TPAK_BANK_ADDRESS, ROMX);
        setTpakValue(controllerNumber, mapAddress(GB_RAM_BANK_ADDRESS), bank);
        // Update the register.
        setTpakValue(controllerNumber, mapAddress(SRAM_ADDRESS), cartridge->Ram.Data[SRAM_BANK_SIZE * bank]);
    }

    return TPAK_SUCCESS;
}

/**
 * Reads the five RealTickClock registers from a cartridge into where they will sit in SRAM
 * @param controllerNumber controller slot of the cartridge.
 * @param cartridge In-memory representation of the cartridge.
 * @returns Error Code
 */
static sByte importRTCData(const byte controllerNumber, GameBoyCartridge* cartridge) {
    // Change to ROM0 and ensure SRAM is enabled.
    setTpakValue(controllerNumber, TPAK_BANK_ADDRESS, ROM0);
    setTpakValue(controllerNumber, mapAddress(ENABLE_GB_RAM_ADDRESS), ENABLE_GB_RAM);

    // Prepare to latch clock (0x6000)
    setTpakValue(controllerNumber, TPAK_BANK_ADDRESS, ROMX);
    setTpakValue(controllerNumber, mapAddress(GB_RTC_LATCH_ADDRESS), 0x00);

    // Record when we do this.
    cartridge->LastRTCTicks = get_ticks_ms();
    // Update the clock.
    setTpakValue(controllerNumber, mapAddress(GB_RTC_LATCH_ADDRESS), 0x01);

    // Smash the 5 RTC registers into memory.
    byte block[32];
    for (byte bank = 0x08; bank < 0x0D; bank++) {
        // Update the bank
        setTpakValue(controllerNumber, TPAK_BANK_ADDRESS, ROMX);
        setTpakValue(controllerNumber, mapAddress(GB_RAM_BANK_ADDRESS), bank);
        // Read the register.
        setTpakValue(controllerNumber, TPAK_BANK_ADDRESS, SRAM);
        read_mempak_address(controllerNumber, mapAddress(SRAM_ADDRESS), block);
        memset(cartridge->Ram.Data + SRAM_BANK_SIZE * bank, block[0], SRAM_BANK_SIZE);
    }

    return TPAK_SUCCESS;
}

/**
 * Starts or stops the rumble motor for cartridges that have one.
 * @param controllerNumber Slot that rumble-capable cartridge is using.
 * @param isRumbleStart true to start the motor, false to stop it.
 * @returns Error Code
 * @note For now this will switch the transfer pak bank to romx and the ram bank to bank 0.
 *       I think this will be ok because I usually switch to whatever bank I need before any transfer.
 */
sByte toggleRumble(const byte controllerNumber, const bool isRumbleStart) {
    byte block[BLOCK_SIZE];

    read_mempak_address(controllerNumber, DETECT_DEVICE_ADDRESS, block);

    if (block[0] == RUMBLE_PAK) {
        // Use regular rumble pak if detected.
        setTpakValue(controllerNumber, RUMBLE_ADDRESS, isRumbleStart);
    } else {
        // Wake up the transfer pak and send power to the cartridge.
        setTpakValue(controllerNumber, CARTRIDGE_POWER_ADDRESS, CARTRIDGE_POWER_ON);
        setTpakValue(controllerNumber, TPAK_STATUS_ADDRESS, TPAK_STATUS_ACCESS_ON);

        // Make sure the bank with the rumble register is selected.
        setTpakValue(controllerNumber, TPAK_BANK_ADDRESS, ROMX);

        const byte RUMBLE_START = 8;
        const byte RUMBLE_STOP = 0;

        if (isRumbleStart) {
            setTpakValue(controllerNumber, mapAddress(GB_RAM_BANK_ADDRESS), RUMBLE_START);
        } else {
            setTpakValue(controllerNumber, mapAddress(GB_RAM_BANK_ADDRESS), RUMBLE_STOP);

            // Since we're not rumbling, cartridge can go back to sleep.
            setTpakValue(controllerNumber, CARTRIDGE_POWER_ADDRESS, CARTRIDGE_POWER_OFF);
        }
    }


    return TPAK_SUCCESS;
}

/**
 * Gets the complete ROM data from the cartridge in a transfer pak.
 * @param controllerNumber T-Pak plugged in to this controller slot.
 * @param cartridge Structure to copy RAM to.
 * @returns Error code
 */
sByte importCartridgeRam(const byte controllerNumber, GameBoyCartridge* cartridge) {
    if (controllerNumber > 3) {
        return TPAK_ERR_NO_SLOT;
    }

    initialiseTPak(controllerNumber);

    if (hasRealTimeClock(&cartridge->Header)) {
        cartridge->Ram.Size = SRAM_BANK_SIZE * 0x0D; // RTC goes up to bank 0x0C
    } else {
        cartridge->Ram.Size = cartridge->RamBankSize * cartridge->RamBankCount;
    }

    if (cartridge->Ram.Data) {
        free(cartridge->Ram.Data);
    }
    cartridge->Ram.Data = calloc(cartridge->Ram.Size, 1);

    // Loop through each bank
    for (byte bank = 0; bank < cartridge->RamBankCount; bank++) {
        switchRamBank(controllerNumber, bank, cartridge->Type);

        // Read into memory 32bytes at a time.
        for (natural address = 0; address < cartridge->RamBankSize; address += BLOCK_SIZE) {
            read_mempak_address(controllerNumber, mapAddress(SRAM_ADDRESS) + address, cartridge->Ram.Data + (bank * cartridge->RamBankSize) + address);
        }
    }

    if (hasRealTimeClock(&cartridge->Header)) {
        sByte result = importRTCData(controllerNumber, cartridge);
        if (result) {
            return result;
        }
    }

    return TPAK_SUCCESS;
}

/**
 * Sets the cartridge RAM with the data in ramData.
 * @param controllerNumber T-Pak plugged in to this controller slot.
 * @param ramData RAM to copy in to the cartridge.
 * @returns Error code
 */
sByte exportCartridgeRam(const byte controllerNumber, GameBoyCartridge* cartridge) {
    if (controllerNumber > 3) {
        return TPAK_ERR_NO_SLOT;
    }

    initialiseTPak(controllerNumber);

    // Loop through each bank
    for (byte bank = 0; bank < cartridge->RamBankCount; bank++) {
        switchRamBank(controllerNumber, bank, cartridge->Type);

        // Read into memory 32bytes at a time.
        for (natural address = 0; address < cartridge->RamBankSize; address += BLOCK_SIZE) {
            write_mempak_address(controllerNumber, mapAddress(SRAM_ADDRESS) + address, cartridge->Ram.Data + (bank * cartridge->RamBankSize) + address);
        }
    }

    if (hasRealTimeClock(&cartridge->Header)) {
        sByte result = exportRTCData(controllerNumber, cartridge);
        if (result) {
            return result;
        }
    }

    // Turn off the lights when we're done.
    setTpakValue(controllerNumber, CARTRIDGE_POWER_ADDRESS, CARTRIDGE_POWER_OFF);

    return TPAK_SUCCESS;
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
    result = tpak_get_cartridge_header(controllerNumber, &header);
    if (result) {
        return result;
    }

    if (VALIDATE_CHECKSUMS && !tpak_check_header(&header)) {
        return TPAK_ERR_CORRUPT_HEADER;
    }

    cartridge->IsGbcSupported = header.new_title.gbc_support == GBC_DMG_SUPPORTED || header.new_title.gbc_support == GBC_ONLY_SUPPORTED;
    cartridge->Type = getPrimaryType(header.cartridge_type);

    if (cartridge->Type == UNKNOWN_CARTRIDGE_TYPE) {
        return TPAK_ERR_UNSUPPORTED_CARTRIDGE;
    }

    sShort romBanks = getNumberOfRomBanks(&header);
    sByte ramBanks = getNumberOfRamBanks(&header, cartridge->Type);
    sInt ramBankSize = getRamBankSize(&header, cartridge->Type); 

    if (romBanks * BANK_SIZE * 3 >= getMemoryLimit() && RESERVE_WORKING_MEMORY) {
        return TPAK_ERR_INSUFFICIENT_MEMORY;
    }

    if (romBanks < 0 || ramBanks < 0 || ramBankSize < 0) {
        return TPAK_ERR_INVALID_HEADER; 
    }

    cartridge->RomBankCount = romBanks;
    cartridge->RamBankCount = ramBanks;
    cartridge->RamBankSize = (natural) ramBankSize;

    // TODO - Fix Broken
    /*
    if (!header.CGBTitle.GbcSupport) {
        cartridge->Title = malloc(16);
        memcpy(cartridge->Title, header.Title, 16);
    } else {
        cartridge->Title = malloc(11);
        memcpy(cartridge->Title, header.CGBTitle.Title, 11);
    } 
    */

    memcpy(&cartridge->Header, &header, HEADER_SIZE);

    return TPAK_SUCCESS;
}

/**
 * Imports the entire cartridge in to RAM as CartridgeData
 * @param controllerNumber get from T-Pak plugged in to this controller slot.
 * @out cartridge GB/GBC cartridge rom/ram 
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

    if (!checkRom(cartridge->Header.global_checksum, &cartridge->Rom) && VALIDATE_CHECKSUMS) 
    {
        return TPAK_ERR_CORRUPT_DATA;
    }

    result = importCartridgeRam(controllerNumber, cartridge);
    if (result) {
        return result;
    }

    // Turn off the lights when we're done.
    setTpakValue(controllerNumber, CARTRIDGE_POWER_ADDRESS, CARTRIDGE_POWER_OFF);

    return TPAK_SUCCESS;
}

/**
 * Gets the percentage complete that a given TPak cartridge load is.
 * @param controllerNumber controller port we are loading from.
 */
byte getLoadProgress(const byte controllerNumber) {
    return _progress[controllerNumber];
}
