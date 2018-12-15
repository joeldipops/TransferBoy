#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "state.h"
#include "hwdefs.h"

#define err(fmt, ...) \
    do { \
        fprintf(stderr, "[State] " fmt "\n", ##__VA_ARGS__); \
        return 1; \
    } while (0)

void print_rom_header_info(u8* rom) {
    printf("Title: %s\n", &rom[ROMHDR_TITLE]);

    u8 cart_cgb_flag = rom[ROMHDR_CGBFLAG];
    if (cart_cgb_flag & 0x80) {
        if ((cart_cgb_flag & 0xc0) == 0xc0)
            printf("CGB: yes (needed)\n");
        else
            printf("CGB: yes (optional)\n");
    } else
        printf("CGB: no\n");

    printf("Cart type: ");
    u8 cart_type = rom[ROMHDR_CARTTYPE];
    switch(cart_type) {
    case 0x00: printf("ROM ONLY\n"); break;
    case 0x01: printf("MBC1\n"); break;
    case 0x02: printf("MBC1+RAM\n"); break;
    case 0x03: printf("MBC1+RAM+BATTERY\n"); break;
    case 0x05: printf("MBC2\n"); break;
    case 0x06: printf("MBC2+BATTERY\n"); break;
    case 0x08: printf("ROM+RAM\n"); break;
    case 0x09: printf("ROM+RAM+BATTERY\n"); break;
    case 0x0b: printf("MMM01\n"); break;
    case 0x0c: printf("MMM01+RAM\n"); break;
    case 0x0d: printf("MMM01+RAM+BATTERY\n"); break;
    case 0x0f: printf("MBC3+TIMER+BATTERY\n"); break;
    case 0x10: printf("MBC3+TIMER+RAM+BATTERY\n"); break;
    case 0x11: printf("MBC3\n"); break;
    case 0x12: printf("MBC3+RAM\n"); break;
    case 0x13: printf("MBC3+RAM+BATTERY\n"); break;
    case 0x15: printf("MBC4\n"); break;
    case 0x16: printf("MBC4+RAM\n"); break;
    case 0x17: printf("MBC4+RAM+BATTERY\n"); break;
    case 0x19: printf("MBC5\n"); break;
    case 0x1a: printf("MBC5+RAM\n"); break;
    case 0x1b: printf("MBC5+RAM+BATTERY\n"); break;
    case 0x1c: printf("MBC5+RUMBLE\n"); break;
    case 0x1d: printf("MBC5+RUMBLE+RAM\n"); break;
    case 0x1e: printf("MBC5+RUMBLE+RAM+BATTERY\n"); break;
    case 0x20: printf("MBC6\n"); break;
    case 0x22: printf("MBC7+SENSOR+RUMBLE+RAM+BATTERY\n"); break;
    case 0xfc: printf("POCKET CAMERA\n"); break;
    case 0xfd: printf("BANDAI TAMA5\n"); break;
    case 0xfe: printf("HuC3\n"); break;
    case 0xff: printf("HuC1+RAM+BATTERY\n"); break;
    default:   printf("UNKNOWN (%x)\n", cart_type); break;
    }

    printf("ROM Size: ");
    u8 cart_ROM = rom[ROMHDR_ROMSIZE];
    switch(cart_ROM) {
    case 0x00: printf("32KB (no banks)\n"); break;
    case 0x01: printf("64KB (4 banks)\n"); break;
    case 0x02: printf("128KB (8 banks)\n"); break;
    case 0x03: printf("256KB (16 banks)\n"); break;
    case 0x04: printf("512KB (32 banks)\n"); break;
    case 0x05: printf("1MB (64 banks)\n"); break;
    case 0x06: printf("2MB (128 banks)\n"); break;
    case 0x07: printf("4MB (256 banks)\n"); break;
    case 0x52: printf("1.1MB (72 banks)\n"); break;
    case 0x53: printf("1.2MB (80 banks)\n"); break;
    case 0x54: printf("1.5MB (96 banks)\n"); break;
    default:   printf("UNKNOWN (%x)\n", cart_ROM);
    }

    printf("RAM Size: ");
    u8 cart_RAM = rom[ROMHDR_EXTRAMSIZE];
    switch(cart_RAM) {
    case 0x00: printf("None\n"); break;
    case 0x01: printf("2KB\n"); break;
    case 0x02: printf("8KB\n"); break;
    case 0x03: printf("32KB (4 banks)\n"); break;
    case 0x04: printf("128KB (16 banks)\n"); break;
    case 0x05: printf("64KB (8 banks)\n"); break;
    default:   printf("UNKNOWN (%x)\n", cart_RAM);
    }
}

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


int rom_get_info(u8 *rom, size_t rom_size, struct rominfo *ret_rominfo) {

    /* Cart info from header */
    if (ROMHDR_CARTTYPE >= rom_size ||
            ROMHDR_ROMSIZE >= rom_size ||
            ROMHDR_EXTRAMSIZE >= rom_size)
        err("Given ROM too small to read header fields (%zu)", rom_size);

    u8 hdr_cart_type = rom[ROMHDR_CARTTYPE];
    u8 hdr_rom_size = rom[ROMHDR_ROMSIZE];
    u8 hdr_extram_size = rom[ROMHDR_EXTRAMSIZE];
    u8 hdr_cgb_flag = rom[ROMHDR_CGBFLAG];

    enum gb_type rom_gb_type;
    int mbc = 0; /* Memory Bank Controller */
    int extram = 0;
    int battery = 0;
    int rtc = 0; /* Real time clock */
    int rom_banks = 0;
    int extram_banks = 0;
    int wram_banks = 0;
    int vram_banks = 0;

    if (hdr_cgb_flag & 0x80)
        rom_gb_type = GB_TYPE_CGB;
    else
        rom_gb_type = GB_TYPE_GB;

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
        err("Unsupported cartridge type: %x", hdr_cart_type);
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
        err("Unsupported ROM size: %x", hdr_rom_size);
    }

    switch (hdr_extram_size) {
    case 0x00: extram_banks = 0;  break; /* None */
    case 0x01: extram_banks = 1;  break; /* 2K */
    case 0x02: extram_banks = 1;  break; /* 8K */
    case 0x03: extram_banks = 4;  break; /* 32K */
    case 0x04: extram_banks = 16; break; /* 128K */
    case 0x05: extram_banks = 8;  break; /* 64KB */
    default:
        err("Unsupported EXT_RAM size: %x", hdr_extram_size);
    }

    assert((extram == 0 && extram_banks == 0) ||
           (extram == 1 && extram_banks > 0));

    if (rom_gb_type == GB_TYPE_GB) {
        wram_banks = 2;
        vram_banks = 1;
    } else if (rom_gb_type == GB_TYPE_CGB) {
        wram_banks = 8;
        vram_banks = 2;
    } else {
        err("Unsupported GB type: %d", rom_gb_type);
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

int state_new_from_rom(struct gb_state *s, u8 *rom, size_t rom_size) {


    struct rominfo rominfo;
    if (rom_get_info(rom, rom_size, &rominfo))
        err("Error retrieving rom info");

    if (rominfo.gb_type != GB_TYPE_GB && rominfo.gb_type != GB_TYPE_CGB)
        err("Unsupported GB type: %d", rominfo.gb_type);
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
void state_add_bios(struct gb_state *s, u8 *bios, size_t bios_size) {
    assert(bios_size == 256);
    s->mem_BIOS = malloc(bios_size);
    memcpy(s->mem_BIOS, bios, bios_size);
    s->in_bios = 1;
    s->pc = 0;
}

/*
 * Initialize the emulator state of the gameboy. This state belongs to the
 * emulator, not the state of the emulated hardware.
 */
void init_emu_state(struct gb_state *s) {
    s->emu_state = calloc(1, sizeof(struct emu_state));
    s->emu_state->dbg_breakpoint = 0xffff;
}

/*
 * Dump the current state of the gameboy into a buffer. This function allocates
 * a buffer large enough to hold `struct gb_state` and all the memory of the
 * gameboy (ROM, WRAM, EXT_RAM, VRAM).
 */
int state_save(struct gb_state *s, u8 **ret_state_buf, size_t *ret_state_size) {
    size_t rom_size = ROM_BANKSIZE * s->mem_num_banks_rom;
    size_t wram_size = WRAM_BANKSIZE * s->mem_num_banks_wram;
    size_t extram_size = EXTRAM_BANKSIZE * s->mem_num_banks_extram;
    size_t vram_size = VRAM_BANKSIZE * s->mem_num_banks_vram;

    if (s->in_bios) {
        fprintf(stderr, "Cannot dump state while in bios\n");
        return 1;
    }

    size_t state_size = sizeof(u32) + sizeof(struct gb_state) + rom_size +
        wram_size + extram_size + vram_size;
    u8 *state_buf = malloc(state_size);

    u32 *hdr = (u32*)state_buf;
    hdr[0] = sizeof(struct gb_state);
    struct gb_state *ts = (struct gb_state*)(&hdr[1]);
    *ts = *s;
    u8 *rom_start = (u8*)(ts + 1);
    u8 *wram_start = rom_start + rom_size;
    u8 *extram_start = wram_start + wram_size;
    u8 *vram_start = extram_start + extram_size;

    memcpy(rom_start, s->mem_ROM, rom_size);
    memcpy(wram_start, s->mem_WRAM, wram_size);
    memcpy(extram_start, s->mem_EXTRAM, extram_size);
    memcpy(vram_start, s->mem_VRAM, vram_size);

    *ret_state_size = state_size;
    *ret_state_buf = state_buf;
    return 0;
}

/*
 * Load the state from the given buffer. This should be a state buffer generated
 * previously by `state_save`.
 *
 * A non-zero return value indicates an error. An error occurs when the header
 * of the state is not compatible with the program (different sizes of `struct
 * gb_state`).
 *
 * This function allocates memory for the underlying buffers of the `struct
 * gb_state` that form the memory of the gameboy (ROM, WRAM, EXT_RAM, VRAM).
 */
 /*
int state_load(struct gb_state *s, u8 *state_buf, size_t state_buf_size) {
    assert(state_buf_size >= sizeof(u32));
    state_buf_size -= sizeof(u32);
    u32 *state_hdr = (u32*)state_buf;
    u32 state_size = state_hdr[0];
    if (state_size != sizeof(struct gb_state))
        err("State header mismatch: file statesize is %u byte, program "
            "statesize is %zu byte.\n", state_size, sizeof(struct gb_state));

    assert(state_buf_size >= sizeof(struct gb_state));
    state_buf_size -= sizeof(struct gb_state);

    struct gb_state *fs = (struct gb_state*)(&state_hdr[1]);
    *s = *fs;

    size_t romsize = ROM_BANKSIZE * s->mem_num_banks_rom;
    assert(state_buf_size >= romsize);
    state_buf_size -= romsize;
    s->mem_ROM = malloc(romsize);
    u8 *romstart = (u8*)(fs + 1);
    memcpy(s->mem_ROM, romstart, romsize);

    size_t wramsize = WRAM_BANKSIZE * s->mem_num_banks_wram;
    assert(state_buf_size >= wramsize);
    state_buf_size -= wramsize;
    s->mem_WRAM = malloc(wramsize);
    u8 *wramstart = romstart + romsize;
    memcpy(s->mem_WRAM, wramstart, wramsize);

    size_t extramsize = EXTRAM_BANKSIZE * s->mem_num_banks_extram;
    assert(state_buf_size >= extramsize);
    state_buf_size -= extramsize;
    s->mem_EXTRAM = malloc(extramsize);
    u8 *extramstart = wramstart + wramsize;
    memcpy(s->mem_EXTRAM, extramstart, extramsize);

    size_t vramsize = VRAM_BANKSIZE * s->mem_num_banks_vram;
    assert(state_buf_size >= vramsize);
    state_buf_size -= vramsize;
    s->mem_VRAM = malloc(vramsize);
    u8 *vramstart = extramstart + extramsize;
    memcpy(s->mem_VRAM, vramstart, vramsize);

    assert(state_buf_size == 0);

    return 0;
}
*/

int state_save_extram(struct gb_state *s, u8 **ret_state_buf,
        size_t *ret_state_size) {   
    size_t extramsize = s->mem_num_banks_extram * EXTRAM_BANKSIZE;
    *ret_state_buf = malloc(extramsize);
    memcpy(*ret_state_buf, s->mem_EXTRAM, extramsize);
    *ret_state_size = extramsize;
    return 0;
}

int state_load_extram(struct gb_state *s, u8 *state_buf,
        size_t state_buf_size) {
    size_t extramsize = s->mem_num_banks_extram * EXTRAM_BANKSIZE;
    if (state_buf_size != extramsize)
        err("Mismatch in size, save has size %zu, emulator %zu bytes",
                state_buf_size, extramsize);

    memcpy(s->mem_EXTRAM, state_buf, state_buf_size);
    return 0;
}
