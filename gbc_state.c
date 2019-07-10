#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "state.h"
#include "hwdefs.h"
#include "logger.h"

static sByte setInfo(GbState* s, GameBoyCartridge* cartridge) {
    // Cart info from header
    if (cartridge->Rom.Size < 0x0150) {
        // Cartridge too small
        return -1;
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
            return -2;
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
    if (setInfo(s, cartridge)) {
        return -1;
    }

    s->Cartridge = cartridge;

    s->VramBanks = calloc(s->VramBankCount, VRAM_BANKSIZE);
    s->WramBanks = calloc(s->WramBankCount, WRAM_BANKSIZE);

    return 0;
}

/**
 * Adds BIOS to the state - should be called only after creating fresh state
 * from rom. Overwrites some state (such as PC) to facilitate running of BIOS.
 */
sByte applyBios(GbState* s, byte* bios) {
    memcpy(s->BIOS, bios, BIOS_SIZE);
    s->in_bios = 1;
    s->pc = 0;

    return 0;
}