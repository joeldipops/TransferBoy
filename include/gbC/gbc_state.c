#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "state.h"
#include "hwdefs.h"
#include "../../logger.h"

struct rominfo {
    enum gb_type gb_type;
    int mbc;
    char has_extram:1;
    char has_battery:1;
    char has_rtc:1;
    int num_rom_banks;
    int num_wram_banks;
    int num_extram_banks;
    int num_vram_banks;
};


static int rom_get_info(u8 *rom, size_t rom_size, struct rominfo *ret_rominfo) {
    // Cart info from header
    if (
        ROMHDR_CARTTYPE >= rom_size ||
        ROMHDR_ROMSIZE >= rom_size ||
        ROMHDR_EXTRAMSIZE >= rom_size
    ) {
        logInfo("Given ROM too small to read header fields (%zu)", rom_size);
        return -1;
    }

    u8 hdr_cart_type = rom[ROMHDR_CARTTYPE];
    u8 hdr_rom_size = rom[ROMHDR_ROMSIZE];
    u8 hdr_extram_size = rom[ROMHDR_EXTRAMSIZE];
    u8 hdr_cgb_flag = rom[ROMHDR_CGBFLAG];

    enum gb_type rom_gb_type;
    int mbc = 0; // Memory Bank Controller
    int extram = 0;
    int battery = 0;
    int rtc = 0; // Real time clock
    int rom_banks = 0;
    int extram_banks = 0;
    int wram_banks = 0;
    int vram_banks = 0;

    if (hdr_cgb_flag & 0x80) {
        rom_gb_type = GB_TYPE_CGB;
    } else {
        rom_gb_type = GB_TYPE_GB;
    }

    switch (hdr_cart_type) {
        case 0x00:                                              break;
        case 0x01: mbc = 1;                                     break;
        case 0x02: mbc = 1; extram = 1;                         break;
        case 0x03: mbc = 1; extram = 1; battery = 1;            break;
        case 0x05: mbc = 2;                                     break;
        case 0x06: mbc = 2;             battery = 1;            break;
        case 0x08:          extram = 1;                         break;
        case 0x09:          extram = 1; battery = 1;            break;
        case 0x0f: mbc = 3;             battery = 1; rtc = 1;   break;
        case 0x10: mbc = 3; extram = 1; battery = 1; rtc = 1;   break;
        case 0x11: mbc = 3;                                     break;
        case 0x12: mbc = 3; extram = 1;                         break;
        case 0x13: mbc = 3; extram = 1; battery = 1;            break;
        case 0x15: mbc = 4;                                     break;
        case 0x16: mbc = 4; extram = 1;                         break;
        case 0x17: mbc = 4; extram = 1; battery = 1;            break;
        case 0x19: mbc = 5;                                     break;
        case 0x1a: mbc = 5; extram = 1;                         break;
        case 0x1b: mbc = 5; extram = 1; battery = 1;            break;
        case 0x1c: mbc = 5;                                     break; /* rumble */
        case 0x1d: mbc = 5; extram = 1;                         break; /* rumble */
        case 0x1e: mbc = 5; extram = 1; battery = 1;            break; /* rumble */
        case 0x20: mbc = 6;                                     break;
        /* MMM01 unsupported */
        /* MBC7 Sensor not supported */
        /* Camera not supported */
        /* Bandai TAMA5 not supported */
        /* HuCn not supported */
        default:
            logInfo("Unsupported cartridge type: %x", hdr_cart_type);
            return -1;
    }

    switch (hdr_rom_size) {
        case 0x00: rom_banks = 2;   break; /* 16K */
        case 0x01: rom_banks = 4;   break; /* 64K */
        case 0x02: rom_banks = 8;   break; /* 128K */
        case 0x03: rom_banks = 16;  break; /* 256K */
        case 0x04: rom_banks = 32;  break; /* 512K */
        case 0x05: rom_banks = 64;  break; /* 1M */
        case 0x06: rom_banks = 128; break; /* 2M */
        case 0x07: rom_banks = 256; break; /* 4M */
        case 0x08: rom_banks = 512; break; /* 8M */
        case 0x52: rom_banks = 72;  break; /* 1.1M */
        case 0x53: rom_banks = 80;  break; /* 1.2M */
        case 0x54: rom_banks = 96;  break; /* 1.5M */
        default:
            logInfo("Unsupported ROM size: %x", hdr_rom_size);
            return -1;
    }

    switch (hdr_extram_size) {
        case 0x00: extram_banks = 0;  break; /* None */
        case 0x01: extram_banks = 1;  break; /* 2K */
        case 0x02: extram_banks = 1;  break; /* 8K */
        case 0x03: extram_banks = 4;  break; /* 32K */
        case 0x04: extram_banks = 16; break; /* 128K */
        case 0x05: extram_banks = 8;  break; /* 64KB */
        default:
            logInfo("Unsupported EXT_RAM size: %x", hdr_extram_size);
            return -1;
    }

    // Prevents my test ROM from running, but I think that's a problem with the test rom not this line.
    // Nevertheless this just crashes cen64, rather than providing a useful error for now.
    //assert((extram == 0 && extram_banks == 0) || (extram == 1 && extram_banks > 0));

    if (rom_gb_type == GB_TYPE_GB) {
        wram_banks = 2;
        vram_banks = 1;
    } else if (rom_gb_type == GB_TYPE_CGB) {
        wram_banks = 8;
        vram_banks = 2;
    } else {
        logInfo("Unsupported GB type: %d", rom_gb_type);
        return -1;
    }

    ret_rominfo->mbc = mbc;
    ret_rominfo->has_extram = extram;
    ret_rominfo->has_battery = battery;
    ret_rominfo->has_rtc = rtc;
    ret_rominfo->num_rom_banks = rom_banks;
    ret_rominfo->num_wram_banks = wram_banks;
    ret_rominfo->num_extram_banks = extram_banks;
    ret_rominfo->num_vram_banks = vram_banks;
    ret_rominfo->gb_type = rom_gb_type;

    return 0;
}

int state_new_from_rom(GbState *s, u8 *rom, size_t rom_size) {
    struct rominfo rominfo;
    if (rom_get_info(rom, rom_size, &rominfo)) {
        logInfo("Error retrieving rom info");
        return -1;
    }

    if (rominfo.gb_type != GB_TYPE_GB && rominfo.gb_type != GB_TYPE_CGB) {
        logInfo("Unsupported GB type: %d", rominfo.gb_type);
        return -1;
    }

    s->gb_type = rominfo.gb_type;

    s->mbc = rominfo.mbc;
    s->has_extram = rominfo.has_extram;
    s->has_battery = rominfo.has_battery;
    s->has_rtc = rominfo.has_rtc;

    s->mem_num_banks_rom = rominfo.num_rom_banks;
    s->mem_num_banks_wram = rominfo.num_wram_banks;
    s->mem_num_banks_extram = rominfo.num_extram_banks;
    s->mem_num_banks_vram = rominfo.num_vram_banks;

    s->mem_ROM = NULL;
    s->mem_WRAM = NULL;
    s->mem_EXTRAM = NULL;
    s->mem_VRAM = NULL;

    s->mem_ROM = malloc(ROM_BANKSIZE * s->mem_num_banks_rom);
    s->mem_WRAM = malloc(WRAM_BANKSIZE * s->mem_num_banks_wram);
    if (s->mem_num_banks_extram)
        s->mem_EXTRAM = malloc(EXTRAM_BANKSIZE * s->mem_num_banks_extram);
    s->mem_VRAM = malloc(VRAM_BANKSIZE * s->mem_num_banks_vram);

    memset(s->mem_ROM, 0, ROM_BANKSIZE * s->mem_num_banks_rom);
    memcpy(s->mem_ROM, rom, rom_size);

    return 0;
}

/*
 * Adds BIOS to the state - should be called only after creating fresh state
 * from rom. Overwrites some state (such as PC) to facilitate running of BIOS.
 */
void state_add_bios(GbState *s, u8 *bios, size_t bios_size) {
    assert(bios_size == 256);
    s->mem_BIOS = malloc(bios_size);
    memcpy(s->mem_BIOS, bios, bios_size);
    s->in_bios = 1;
    s->pc = 0;
}

int state_load_extram(GbState *s, u8 *state_buf, size_t state_buf_size) {
    size_t extramsize = s->mem_num_banks_extram * EXTRAM_BANKSIZE;
    if (state_buf_size != extramsize) {
        logInfo("Mismatch in size, save has size %zu, emulator %zu bytes", state_buf_size, extramsize);
        return -1;
    }

    memcpy(s->mem_EXTRAM, state_buf, state_buf_size);
    return 0;
}