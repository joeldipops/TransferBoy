#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "state.h"
#include "hwdefs.h"
#include "logger.h"
#include "gbc_state.h"

static sByte setInfo(GbState* s, const GameBoyCartridge* cartridge) {
    // Cart info from header
    if (cartridge->Rom.Size < 0x0150) {
        // Cartridge too small
        return LOAD_ERR_TOO_SMALL;
    }

    switch (cartridge->Type) {
        case 0x00:                                                                  break;
        case 0x01:  s->mbc = 1;                                                     break;
        case 0x02:  s->mbc = 1; s->hasSram = 1;                                     break;
        case 0x03:  s->mbc = 1; s->hasSram = 1; s->hasBattery = 1;                  break;
        case 0x05:  s->mbc = 2;                                                     break;
        case 0x06:  s->mbc = 2;                 s->hasBattery = 1;                  break;
        case 0x08:              s->hasSram = 1;                                     break;
        case 0x09:              s->hasSram = 1; s->hasBattery = 1;                  break;
        case 0x0f: s->mbc = 3;                  s->hasBattery = 1; s->hasRtc = 1;   break;
        case 0x10: s->mbc = 3;  s->hasSram = 1; s->hasBattery = 1; s->hasRtc = 1;   break;
        case 0x11: s->mbc = 3;                                                      break;
        case 0x12: s->mbc = 3;  s->hasSram = 1;                                     break;
        case 0x13: s->mbc = 3;  s->hasSram = 1; s->hasBattery = 1;                  break;
        //case 0x15: s->mbc = 4;                                                      break;
        //case 0x16: s->mbc = 4;  s->hasSram = 1;                                     break;
        //case 0x17: s->mbc = 4;  s->hasSram = 1; s->hasBattery = 1;                  break;
        case 0x19: s->mbc = 5;                                                      break;
        case 0x1a: s->mbc = 5;  s->hasSram = 1;                                     break;
        case 0x1b: s->mbc = 5;  s->hasSram = 1; s->hasBattery = 1;                  break;
        case 0x1c: s->mbc = 5;                                                      break; /* rumble */
        case 0x1d: s->mbc = 5;  s->hasSram = 1;                                     break; /* rumble */
        case 0x1e: s->mbc = 5;  s->hasSram = 1; s->hasBattery = 1;                  break; /* rumble */
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
        s->WramBankCount = 8;
        s->VramBankCount = 2;
    } else  {
        s->WramBankCount = 2;
        s->VramBankCount = 1;
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
    memcpy(s->ROM0, cartridge->Rom.Data, ROM_BANK_SIZE);

    if (cartridge->Rom.Size >= ROM_BANK_SIZE * 2) {
        memcpy(s->ROMX, cartridge->Rom.Data + ROM_BANK_SIZE, ROM_BANK_SIZE);
    } else if (cartridge->Rom.Size > ROM_BANK_SIZE) {
        // Ensure the blank portion returns 0xFF
        memset(s->ROMX, 0xFF, ROM_BANK_SIZE);
        memcpy(s->ROMX, cartridge->Rom.Data + ROM_BANK_SIZE, cartridge->Rom.Size - ROM_BANK_SIZE);
    } else {
        memset(s->ROMX, 0xFF, ROM_BANK_SIZE);        
    }

    memset(s->SRAM, 0xFF, 0x2000);
    if (s->hasSram) {
        memcpy(s->SRAM, cartridge->Ram.Data, cartridge->Ram.Size < 0x2000 ? cartridge->Ram.Size : 0x2000);
    }

    s->VramBanks = calloc(s->VramBankCount, VRAM_BANK_SIZE);
    s->WramBanks = calloc(s->WramBankCount, WRAM_BANK_SIZE);

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

    memcpy(s->BIOS, bios->Data, bios->Size);
    memcpy(s->ROM0, bios->Data, bios->Size);
    s->in_bios = 1;
    s->pc = 0;

    return 0;
}