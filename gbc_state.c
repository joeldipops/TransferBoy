#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "state.h"
#include "mmu.h"
#include "hwdefs.h"
#include "gbc_state.h"

static sByte setInfo(GbState* s) {
    // Cart info from header
    if (s->Cartridge.Rom.Size < 0x0150) {
        // Cartridge too small
        return LOAD_ERR_TOO_SMALL;
    }

    switch (s->Cartridge.Header.CartridgeType) {
        case 0x00: s->mbc = 0;                                                     break;
        case 0x01: s->mbc = 1;                                                     break;
        case 0x02: s->mbc = 1; s->hasSRAM = 1;                                     break;
        case 0x03: s->mbc = 1; s->hasSRAM = 1; s->hasBattery = 1;                  break;
        case 0x05: s->mbc = 2; s->hasSRAM = 1;                                     break;
        case 0x06: s->mbc = 2; s->hasSRAM = 1; s->hasBattery = 1;                  break;
        case 0x08: s->mbc = 0; s->hasSRAM = 1;                                     break;
        case 0x09: s->mbc = 0; s->hasSRAM = 1; s->hasBattery = 1;                  break;
        case 0x0f: s->mbc = 3;                 s->hasBattery = 1; s->hasRTC = 1;   break;
        case 0x10: s->mbc = 3; s->hasSRAM = 1; s->hasBattery = 1; s->hasRTC = 1;   break;
        case 0x11: s->mbc = 3;                                                     break;
        case 0x12: s->mbc = 3; s->hasSRAM = 1;                                     break;
        case 0x13: s->mbc = 3; s->hasSRAM = 1; s->hasBattery = 1;                  break;
        //case 0x15: s->mbc = 4;                                                      break;
        //case 0x16: s->mbc = 4;  s->hasSRAM = 1;                                     break;
        //case 0x17: s->mbc = 4;  s->hasSRAM = 1; s->hasBattery = 1;                  break;
        case 0x19: s->mbc = 5;                                                     break;
        case 0x1a: s->mbc = 5; s->hasSRAM = 1;                                     break;
        case 0x1b: s->mbc = 5; s->hasSRAM = 1; s->hasBattery = 1;                  break;
        case 0x1c: s->mbc = 5;                                                     break; /* rumble */
        case 0x1d: s->mbc = 5; s->hasSRAM = 1;                                     break; /* rumble */
        case 0x1e: s->mbc = 5; s->hasSRAM = 1; s->hasBattery = 1;                  break; /* rumble */
        case 0x20: s->mbc = 6;                                                     break;
        /* MMM01 unsupported */
        /* MBC7 Sensor not supported */
        /* Camera not supported */
        /* Bandai TAMA5 not supported */
        /* HuCn not supported */
        default:
            // Unsupported cartridge.
            return LOAD_ERR_UNSUPPORTED;
    }

    if (s->Cartridge.IsGbcSupported) {
        s->WRAMBankCount = 8;
        s->VRAMBankCount = 2;
    } else  {
        s->WRAMBankCount = 2;
        s->VRAMBankCount = 1;
    }

    return 0;
}

/**
 * Initialises all the memory to emulate a gameboy cartridge.
 */
sByte loadCartridge(GbState* s) {
    sByte result = setInfo(s);
    if (result) {
        return result;
    }

    // Initialise cartridge memory.
    s->ROM0 = s->Cartridge.Rom.Data;
    s->ROMX = s->Cartridge.Rom.Data + ROM_BANK_SIZE;

    // SRAM starts out disabled.
    s->SRAM = (byte*) disabledRAMPage;

    if (s->hasRTC) {
        s->Cartridge.RTCTimeStopped = 0;
        s->Cartridge.RTCStopTime = 0;
    }

    s->VRAMBanks = calloc(s->VRAMBankCount, VRAM_BANK_SIZE);
    s->VRAM = s->VRAMBanks;
    s->WRAMBanks = calloc(s->WRAMBankCount, WRAM_BANK_SIZE);
    s->WRAM0 = s->WRAMBanks;
    s->WRAMX = s->WRAMBanks + 0x1000;

    mmu_install_mbc(s);

    return 0;
}

/**
 * Adds BIOS to the state - should be called only after creating fresh state
 * from rom. Overwrites some state (such as PC) to facilitate running of BIOS.
 *
 */
sByte applyBios(GbState* s, ByteArray* bios) {
    if (bios->Size > ROM_BANK_SIZE) {
        return LOAD_ERR_TOO_LARGE;
    }

    s->BiosFile = bios->Data;
    // Overlay the BIOS data on top of the ROM0 data.
    s->BIOS = malloc(ROM_BANK_SIZE);
    memcpy(s->BIOS, s->ROM0, ROM_BANK_SIZE);
    memcpy(s->BIOS, bios->Data, bios->Size);

    s->ROM0 = s->BIOS;

    s->in_bios = 1;
    s->pc = 0;

    return 0;
}