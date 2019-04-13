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

/**
 * 
 * @returns Error Code
 *  0 - Success
 * -1 - Controller not plugged in.
 * -2 - Transfer pak not detected.
 * -3 - Data corruption.
 */
s8 initialiseTPak(const u8 controllerNumber) {
    int accessory = identify_accessory(controllerNumber);

    if (accessory != ACCESSORY_MEMPAK) {
        return -2;
    }

    u8 block[32];
    memset(block, ENABLE_TPAK, BLOCK_SIZE);

    // Wake up the transfer pak
    s8 result = 0;
    result = write_mempak_address(controllerNumber, ENABLE_TPAK_ADDRESS, block);
    if (result) {
        return result;
    }
    read_mempak_address(controllerNumber, ENABLE_TPAK_ADDRESS, block);
    if (block[0] != 0x84) {
        return -2;
    }

    // And supply power to the cartridge
    memset(block, ENABLED, BLOCK_SIZE);
    write_mempak_address(controllerNumber, TPAK_CART_ON_ADDRESS, block);

    // Set bank 1
    memset(block, 0, BLOCK_SIZE);
    write_mempak_address(controllerNumber, TPAK_BANK_SWITCH_ADDRESS, block);

    return 0;

}
 

/**
 * Gets the complete ROM data from the cartridge a transfer pak.
 * @param controllerNumber T-Pak plugged in to this controller slot.
 * @param romData ROM will be copied to this address.
 * @returns Error codes
 ** 0  - Successful
 ** -1 - Error
 ** -2 - Invalid controller slot (must be 1-4)
 */
s8 getRom(const u8 controllerNumber, Buffer romData) {
    if (controllerNumber <= 0 || controllerNumber > 4) {
        return -2;
    }

    s8 result = initialiseTPak(controllerNumber - 1);
    if (result) {
        return result;
    }

    return 0;
}

/**
 * Gets the complete ROM data from the cartridge in a transfer pak.
 * @param controllerNumber T-Pak plugged in to this controller slot.
 * @param ramData RAM will be copied to this address.
 * @returns Error codes
 **  0 - Successful
 ** -1 - Error
 ** -2 - Invalid controller slot (must be 1-4) 
 */
s8 getRam(const u8 controllerNumber, Buffer ramData) {
    if (controllerNumber <= 0 || controllerNumber > 4) {
        return -2;
    }
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
s8 setRam(const u8 controllerNumber, Buffer ramData) {
    if (controllerNumber <= 0 || controllerNumber > 4) {
        return -2;
    }
    return -1;
}