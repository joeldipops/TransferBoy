#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "state.h"
#include "hwdefs.h"
#include "gbc_state.h"

static sByte setInfo(GbState* s, const GameBoyCartridge* cartridge) {
    // Cart info from header
    if (cartridge->Rom.Size < 0x0150) {
        // Cartridge too small
        return LOAD_ERR_TOO_SMALL;
    }

    switch (cartridge->Header.CartridgeType) {
        case 0x00:                                                                  break;
        case 0x01: s->mbc = 1;                                                     break;
        case 0x02: s->mbc = 1; s->hasSRAM = 1;                                     break;
        case 0x03: s->mbc = 1; s->hasSRAM = 1; s->hasBattery = 1;                  break;
        case 0x05: s->mbc = 2; s->hasSRAM = 1;                                     break;
        case 0x06: s->mbc = 2; s->hasSRAM = 1; s->hasBattery = 1;                  break;
        case 0x08:              s->hasSRAM = 1;                                     break;
        case 0x09:              s->hasSRAM = 1; s->hasBattery = 1;                  break;
        case 0x0f: s->mbc = 3;                  s->hasBattery = 1; s->hasRtc = 1;   break;
        case 0x10: s->mbc = 3;  s->hasSRAM = 1; s->hasBattery = 1; s->hasRtc = 1;   break;
        case 0x11: s->mbc = 3;                                                      break;
        case 0x12: s->mbc = 3;  s->hasSRAM = 1;                                     break;
        case 0x13: s->mbc = 3;  s->hasSRAM = 1; s->hasBattery = 1;                  break;
        //case 0x15: s->mbc = 4;                                                      break;
        //case 0x16: s->mbc = 4;  s->hasSRAM = 1;                                     break;
        //case 0x17: s->mbc = 4;  s->hasSRAM = 1; s->hasBattery = 1;                  break;
        case 0x19: s->mbc = 5;                                                      break;
        case 0x1a: s->mbc = 5;  s->hasSRAM = 1;                                     break;
        case 0x1b: s->mbc = 5;  s->hasSRAM = 1; s->hasBattery = 1;                  break;
        case 0x1c: s->mbc = 5;                                                      break; /* rumble */
        case 0x1d: s->mbc = 5;  s->hasSRAM = 1;                                     break; /* rumble */
        case 0x1e: s->mbc = 5;  s->hasSRAM = 1; s->hasBattery = 1;                  break; /* rumble */
        case 0x20: s->mbc = 6;                                                      break;
        /* MMM01 unsupported */
        /* MBC7 Sensor not supported */
        /* Camera not supported */
        /* Bandai TAMA5 not supported */
        /* HuCn not supported */
        default:
            // Unsupported cartridge.
            return LOAD_ERR_UNSUPPORTED;
    }

    if (cartridge->IsGbcSupported) {
        s->WRAMBankCount = 8;
        s->VRAMBankCount = 2;
    } else  {
        s->WRAMBankCount = 2;
        s->VRAMBankCount = 1;
    }

    return 0;
}

sByte loadCartridge(GbState *s, GameBoyCartridge* cartridge) {
    sByte result = setInfo(s, cartridge);
    if (result) {
        return result;
    }

    s->Cartridge = cartridge;

    // Initialise cartridge memory.
    s->ROM0 = cartridge->Rom.Data;
    s->ROMX = cartridge->Rom.Data + ROM_BANK_SIZE;

    if (s->hasSRAM) {
        s->SRAM = cartridge->Ram.Data;
    } else {
        s->SRAM = malloc(0x2000);
        memset(s->SRAM, 0xFF, 0x2000);
    }

    s->VRAMBanks = calloc(s->VRAMBankCount, VRAM_BANK_SIZE);
    s->VRAM = s->VRAMBanks;
    s->WRAMBanks = calloc(s->WRAMBankCount, WRAM_BANK_SIZE);
    s->WRAM0 = s->WRAMBanks;
    s->WRAMX = s->WRAMBanks + 0x1000;

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