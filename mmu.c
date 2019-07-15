#include <stdio.h>

#include "mmu.h"
#include "hwdefs.h"
#include "logger.h"

#define mmu_error(fmt, ...) \
    do { \
        /*logAndPause("MMU Error: " fmt "\n", ##__VA_ARGS__);*/ \
    } while (0)

#define mmu_assert(cond) \
    do { \
        if (!(cond)) { \
            printf("MMU Assertion failed at %s:%d: " #cond "\n", __FILE__, __LINE__); \
        } \
    } while (0)

static void mmu_hdma_do(GbState *s) {
    // DMA one block (0x10 byte), should be called at start of H-Blank. */
    //mmu_assert(s->io_hdma_running);
    //mmu_assert((s->GbcHdmaControl & (1<<7)) == 0);
    //mmu_assert((s->LcdStatus & 3) == 0);

    for (int i = 0; i < 0x10; i++) {
        u8 dat = mmu_read(s, s->io_hdma_next_src++);
        mmu_write(s, s->io_hdma_next_dst++, dat);
    }

    u32 clks = GB_HDMA_BLOCK_CLKS;
    if (s->double_speed)
        clks *= 2;
    s->last_op_cycles += clks;

    s->GbcHdmaControl--;
    if (s->GbcHdmaControl == 0xff) {
        /* Underflow meant we copied the last block and are done. */
        s->io_hdma_running = 0;
    }
}

static void mmu_hdma_start(GbState *s, u8 lenmode) {
    u16 blocks = (lenmode & ~(1<<7)) + 1;
    u16 len = blocks * 0x10;
    u8 mode_hblank = (lenmode & (1<<7)) ? 1 : 0;
    u16 src = ((s->io_hdma_src_high << 8) | s->io_hdma_src_low) & ~0xf;
    u16 dst = ((s->io_hdma_dst_high << 8) | s->io_hdma_dst_low) & ~0xf;
    dst = (dst & 0x1fff) | 0x8000; /* Ignore upper 3 bits (always in VRAM) */

    printf("HDMA @%.2x:%.4x %.4x -> %.4x, blocks=%.2x mode_hblank=%d\n",
            s->RomBankLower, s->pc,  src, dst, blocks, mode_hblank);

    if (s->io_hdma_running && !mode_hblank) {
        /* Cancel ongoing H-Blank HDMA transfer */
        s->io_hdma_running = 0;
        s->GbcHdmaControl = 0xff; /* done */
        return;
    }

    //mmu_assert(blocks > 0 && blocks <= 0x80);
    //mmu_assert(src + len <= 0x8000 || /* ROM */ (src >= 0xa000 && src + len <= 0xe000)); /* EXT_RAM */
    //mmu_assert(dst >= 0x8000 && dst + len <= 0xa000); /* VRAM */

    if (!mode_hblank) {
        for (u16 i = 0; i < len; i++)
            mmu_write(s, dst++, mmu_read(s, src++));

        s->GbcHdmaControl = 0xff; /* done */
        u32 clks = blocks * GB_HDMA_BLOCK_CLKS;
        if (s->double_speed)
            clks *= 2;
        s->last_op_cycles += clks;
    } else {
        s->io_hdma_running = 1;
        s->io_hdma_next_src = src;
        s->io_hdma_next_dst = dst;
        s->GbcHdmaControl = blocks - 1;

        if ((s->LcdStatus & 3) == 0) /* H-Blank */
            mmu_hdma_do(s);
    }
}

void mmu_step(GbState *s) {
    if (s->lcd_entered_hblank && s->io_hdma_running)
        mmu_hdma_do(s);
}

// mmu_write special cases
static void writeAudioChannelSwitch(GbState* s, byte offset, byte value) {
    s->AudioChannelSwitch = (value & 0x80) | (s->AudioChannelSwitch & 0x7f);
}

static void writeLcdStatus(GbState* s, byte offset, byte value) {
    s->LcdStatus = (value & ~7) | (s->LcdStatus & 7);
}

static void writeCurrentLine(GbState* s, byte offset, byte value) {
    s->CurrentLine = 0x0;
    s->LcdStatus = (s->LcdStatus & 0xfb) | ((s->CurrentLine == s->NextInterruptLine) << 2);
}

static void writeNextInterruptLine(GbState* s, byte offset, byte value) {
    s->NextInterruptLine = value;
    s->LcdStatus = (s->LcdStatus & 0xfb) | ((s->CurrentLine == s->NextInterruptLine) << 2);
}

static void writeDmaSource(GbState* s, byte offset, byte value) {
    // FF46: Transfers memory to OAM for rendering
   
    // Normally this transfer takes ~160ms (during which only HRAM
    // is accessible) but it's okay to be instantaneous. Normally
    // roms loop for ~200 cycles or so to wait.  
    for (unsigned i = 0; i < OAM_SIZE; i++) {
        s->OAM[i] = mmu_read(s, (value << 8) + i);
    }
}

static void writeGbcVRAMBank(GbState* s, byte offset, byte value) {
    s->GbcVRAMBank = value & 1;
}

static void writeBiosSwitch(GbState* s, byte offset, byte value) {
    s->in_bios = 0;
    // Blow away the bios.
    memcpy(s->ROM0, s->Cartridge->Rom.Data, ROM_BANK_SIZE);
}

static void writeGbcHdmaControl(GbState* s, byte offset, byte value) {
    mmu_hdma_start(s, value);
}

static void writeGbcBackgroundPaletteData(GbState* s, byte offset, byte value) {
    s->io_lcd_BGPD[s->GbcBackgroundPaletteIndexRegister & 0x3f] = value;
    if (s->GbcBackgroundPaletteIndexRegister & (1 << 7)) {
        s->GbcBackgroundPaletteIndexRegister = (((s->GbcBackgroundPaletteIndexRegister & 0x3f) + 1) & 0x3f) | (1 << 7);
    }
}

static void writeGbcSpritePaletteData(GbState* s, byte offset, byte value) {
    s->io_lcd_OBPD[s->GbcSpritePaletteIndexRegister & 0x3f] = value;
    if (s->GbcSpritePaletteIndexRegister & (1 << 7)) {
        s->GbcSpritePaletteIndexRegister = (((s->GbcSpritePaletteIndexRegister & 0x3f) + 1) & 0x3f) | (1 << 7);        
    }
 }

static void writeGbcRamBankSelect(GbState* s, byte offset, byte value) {
    if (value == 0) {
        value = 1;
    }
    value &= s->WRAMBankCount - 1;
    s->GbcRamBankSelectRegister = value;  
}


static void writeHram(GbState* s, byte offset, byte value) {
    s->HRAM[offset] = value;
}

 typedef void (*mmuWriteHramOperation)(GbState*, byte, byte);
 static mmuWriteHramOperation mmuWriteHramTable[] = {
//        0         1         2         3         4         5         6         7         8         9         A         B         C         D         E         F   
/*   0 */ writeHram,writeHram,writeHram,writeHram,writeHram,writeHram,writeHram,writeHram,writeHram,writeHram,writeHram,writeHram,writeHram,writeHram,writeHram,writeHram,
/*   1 */ writeHram,writeHram,writeHram,writeHram,writeHram,writeHram,writeHram,writeHram,writeHram,writeHram,writeHram,writeHram,writeHram,writeHram,writeHram,writeHram,
/*   2 */ writeHram,writeHram,writeHram,writeHram,writeHram,writeHram,writeAudioChannelSwitch,writeHram,writeHram,writeHram,writeHram,writeHram,writeHram,writeHram,writeHram,writeHram,
/*   3 */ writeHram,writeHram,writeHram,writeHram,writeHram,writeHram,writeHram,writeHram,writeHram,writeHram,writeHram,writeHram,writeHram,writeHram,writeHram,writeHram,
/*   4 */ writeHram,writeLcdStatus,writeHram,writeHram,writeCurrentLine,writeNextInterruptLine,writeDmaSource,writeHram,writeHram,writeHram,writeHram,writeHram,writeHram,writeHram,writeHram,writeGbcVRAMBank,
/*   5 */ writeBiosSwitch,writeHram,writeHram,writeHram,writeHram,writeGbcHdmaControl,writeHram,writeHram,writeHram,writeHram,writeHram,writeHram,writeHram,writeHram,writeHram,writeHram,
/*   6 */ writeHram,writeHram,writeHram,writeHram,writeHram,writeHram,writeHram,writeHram,writeHram,writeGbcBackgroundPaletteData,writeHram,writeGbcSpritePaletteData,writeHram,writeHram,writeHram,writeHram,
/*   7 */ writeGbcRamBankSelect,writeHram,writeHram,writeHram,writeHram,writeHram,writeHram,writeHram,writeHram,writeHram,writeHram,writeHram,writeHram,writeHram,writeHram,writeHram,
/*   8 */ writeHram,writeHram,writeHram,writeHram,writeHram,writeHram,writeHram,writeHram,writeHram,writeHram,writeHram,writeHram,writeHram,writeHram,writeHram,writeHram,
/*   9 */ writeHram,writeHram,writeHram,writeHram,writeHram,writeHram,writeHram,writeHram,writeHram,writeHram,writeHram,writeHram,writeHram,writeHram,writeHram,writeHram,
/*   A */ writeHram,writeHram,writeHram,writeHram,writeHram,writeHram,writeHram,writeHram,writeHram,writeHram,writeHram,writeHram,writeHram,writeHram,writeHram,writeHram,
/*   B */ writeHram,writeHram,writeHram,writeHram,writeHram,writeHram,writeHram,writeHram,writeHram,writeHram,writeHram,writeHram,writeHram,writeHram,writeHram,writeHram,
/*   C */ writeHram,writeHram,writeHram,writeHram,writeHram,writeHram,writeHram,writeHram,writeHram,writeHram,writeHram,writeHram,writeHram,writeHram,writeHram,writeHram,
/*   D */ writeHram,writeHram,writeHram,writeHram,writeHram,writeHram,writeHram,writeHram,writeHram,writeHram,writeHram,writeHram,writeHram,writeHram,writeHram,writeHram,
/*   E */ writeHram,writeHram,writeHram,writeHram,writeHram,writeHram,writeHram,writeHram,writeHram,writeHram,writeHram,writeHram,writeHram,writeHram,writeHram,writeHram,
/*   F */ writeHram,writeHram,writeHram,writeHram,writeHram,writeHram,writeHram,writeHram,writeHram,writeHram,writeHram,writeHram,writeHram,writeHram,writeHram,writeHram
};

static byte readHram(GbState* s, byte offset) {
    return s->HRAM[offset];
}

static byte readJoypadIo(GbState* s, byte offset) {
    // FF00: Joypad
    u8 rv = 0;
    if ((s->JoypadIo & (1 << 4)) == 0)
        rv = (s->JoypadIo & 0xf0) | (s->io_buttons_dirs & 0x0f);
    else if ((s->JoypadIo & (1 << 5)) == 0)
        rv =  (s->JoypadIo & 0xf0) | (s->io_buttons_buttons & 0x0f);
    else
        rv = (s->JoypadIo & 0xf0) | (s->io_buttons_buttons & 0x0f);
    return rv;    
}

// FF4F GbcVRAMBank
static byte readGbcVRAMBank(GbState* s, byte offset) {
    return s->GbcVRAMBank & 1;
}

// FF69 Background Palette data
static byte readBackgroundPaletteData(GbState* s, byte offset) {
    return s->io_lcd_BGPD[s->GbcBackgroundPaletteIndexRegister & 0x3f];    
}

// FF6B Sprite Palette data
static byte readSpritePaletteData(GbState* s, byte offset) {
    return s->io_lcd_OBPD[s->GbcSpritePaletteIndexRegister & 0x3f];
}

typedef byte (*mmuReadHramOperation)(GbState*, byte);
static mmuReadHramOperation mmuReadHramTable[] = {
//        0         1         2         3         4         5         6         7         8         9         A         B         C         D         E         F       
/*   0 */ readJoypadIo, readHram, readHram, readHram, readHram, readHram, readHram, readHram, readHram, readHram, readHram, readHram, readHram, readHram, readHram, readHram, 
/*   1 */ readHram, readHram, readHram, readHram, readHram, readHram, readHram, readHram, readHram, readHram, readHram, readHram, readHram, readHram, readHram, readHram, 
/*   2 */ readHram, readHram, readHram, readHram, readHram, readHram, readHram, readHram, readHram, readHram, readHram, readHram, readHram, readHram, readHram, readHram, 
/*   3 */ readHram, readHram, readHram, readHram, readHram, readHram, readHram, readHram, readHram, readHram, readHram, readHram, readHram, readHram, readHram, readHram, 
/*   4 */ readHram, readHram, readHram, readHram, readHram, readHram, readHram, readHram, readHram, readHram, readHram, readHram, readHram, readHram, readHram, readGbcVRAMBank, 
/*   5 */ readHram, readHram, readHram, readHram, readHram, readHram, readHram, readHram, readHram, readHram, readHram, readHram, readHram, readHram, readHram, readHram, 
/*   6 */ readHram, readHram, readHram, readHram, readHram, readHram, readHram, readHram, readHram, readBackgroundPaletteData, readHram, readSpritePaletteData, readHram, readHram, readHram, readHram, 
/*   7 */ readHram, readHram, readHram, readHram, readHram, readHram, readHram, readHram, readHram, readHram, readHram, readHram, readHram, readHram, readHram, readHram, 
/*   8 */ readHram, readHram, readHram, readHram, readHram, readHram, readHram, readHram, readHram, readHram, readHram, readHram, readHram, readHram, readHram, readHram, 
/*   9 */ readHram, readHram, readHram, readHram, readHram, readHram, readHram, readHram, readHram, readHram, readHram, readHram, readHram, readHram, readHram, readHram, 
/*   A */ readHram, readHram, readHram, readHram, readHram, readHram, readHram, readHram, readHram, readHram, readHram, readHram, readHram, readHram, readHram, readHram, 
/*   B */ readHram, readHram, readHram, readHram, readHram, readHram, readHram, readHram, readHram, readHram, readHram, readHram, readHram, readHram, readHram, readHram, 
/*   C */ readHram, readHram, readHram, readHram, readHram, readHram, readHram, readHram, readHram, readHram, readHram, readHram, readHram, readHram, readHram, readHram, 
/*   D */ readHram, readHram, readHram, readHram, readHram, readHram, readHram, readHram, readHram, readHram, readHram, readHram, readHram, readHram, readHram, readHram, 
/*   E */ readHram, readHram, readHram, readHram, readHram, readHram, readHram, readHram, readHram, readHram, readHram, readHram, readHram, readHram, readHram, readHram, 
/*   F */ readHram, readHram, readHram, readHram, readHram, readHram, readHram, readHram, readHram, readHram, readHram, readHram, readHram, readHram, readHram, readHram, 
};

static void writeRom01(GbState* s, u16 location, byte value) {
    // 0000 - 1FFF: Fixed ROM bank
    if (value == 0) {
        s->isSRAMDisabled = true;          
    } else if (value == 0x0A) {
        s->isSRAMDisabled = false;
    }
}

natural static inline getMbc1RomBank(const GbState* s) {
    return (s->RomBankUpper << 5) | s->RomBankLower;
}

natural static inline getMbc5RomBank(const GbState* s) {
    return (s->RomBankUpper << 8) | s->RomBankLower;
}

/**
 * 2000 - 3FFF: ROM bank switch
 * Sets the lower part of the ROM bank number (in most cases) and then copies the bank in to ROMX.
 */
static void writeRom23(GbState* s, u16 location, byte value) {
    if (s->mbc == 0) {
        return;
    }

    natural bank = 0;

    if (value == 0 && s->mbc != 5) {
        value = 1;
    }

    if (s->mbc == 0) {
        return;
    } else if (s->mbc == 1) {
        s->RomBankLower = value & 0x1f;
        bank = getMbc1RomBank(s);
    } else if (s->mbc == 3) {
        s->RomBankLower = value & 0x7f;
        bank = s->RomBankLower;
    } else if (s->mbc == 5) {
        // MBC5 splits up this area into 2000-2fff for low bits rom bank,
        // and 3000-3fff for the high bit.
        if (location < 0x3000) { // lower 8 bit
            s->RomBankLower = value;
        } else { // Upper bit
            s->RomBankUpper = value & 1;    
        }
        bank = getMbc5RomBank(s);
    } else {
        // return ERROR_MBC_NOT_IMPLEMENTED;
        return; 
    }

    s->ROMX = s->Cartridge->Rom.Data + (bank * ROM_BANK_SIZE);
}

/**
 * 0x4000 - 0x5FFF: ROM/RAM bank switch
 * Sets the RAM bank number or  the upper part of the ROM bank number depending on the mbc type and current state.
 */
static void writeRom45(GbState* s, u16 location, byte value) {
    byte oldBankNumber = s->SRamBankNumber;
    if (s->mbc == 1) {
        if (s->RomRamSelect == ROM_SELECT) {
            s->RomBankUpper = value & 3;
            s->ROMX = s->Cartridge->Rom.Data + (getMbc1RomBank(s) * ROM_BANK_SIZE);
            return;
        } else { // RAM_SELECT
            s->SRamBankNumber = value & 3;
            if (s->Cartridge->RamBankCount == 1) {
                s->SRamBankNumber &= 1;
            }
        }
    } else if (s->mbc == 3) {
        s->mem_mbc3_extram_rtc_select = value;
    } else if (s->mbc == 5) {
        s->SRamBankNumber = value & 0xf;
        s->SRamBankNumber &= s->Cartridge->RamBankCount - 1;
    } else {
        //return ERROR_MBC_NOT_IMPLEMENTED;
        return;
    }

    // Swap in new RAM bank.
    s->SRAM = s->Cartridge->Ram.Data + s->SRamBankNumber * SRAM_BANK_SIZE;
}

/**
 * 0x6000 - 0x7FFF: Mainly Real-Time-Clock stuff
 */
static void writeRom67(GbState* s, u16 location, byte value) {
    
    if (s->mbc == 1) {      
        s->RomRamSelect = value & 0x1;
    } else if (s->hasRtc) { // MBC3 only
        if (s->mem_latch_rtc == 0x01 && value == 0x01) {
            // TODO... actually latch something?
            s->mem_latch_rtc = s->mem_latch_rtc;
        }
        s->mem_latch_rtc = value;
    } else if (s->mbc == 3 || s->mbc == 5) { // MBC3 without RTC or MBC5
        // Just ignore it - Pokemon Red writes here because it's coded for
        // MBC1, but actually has an MBC3, for instance
    } else {
        //return ERROR_MBC_NOT_IMPLEMENTED;
        return;
    }
}

/**
 * 0x8000 - 0x9FFF: VRAM
 */
static void writeVRAM(GbState* s, u16 location, byte value) {
    s->VRAMBanks[s->GbcVRAMBank * VRAM_BANK_SIZE + location - 0x8000] = value;    
}

/**
 * 0xA000 -0xBFFF: Cartridge RAM
 */
static void writeSRAM(GbState* s, u16 location, byte value) {
    if (!s->hasSRAM || s->isSRAMDisabled) {
        return;
    } else if (s->mbc == 3 && s->mem_mbc3_extram_rtc_select >= 0x04) {
        if (s->mem_mbc3_extram_rtc_select >= 0x08 && s->mem_mbc3_extram_rtc_select <= 0x0c) {
            s->mem_RTC[s->mem_mbc3_extram_rtc_select] = value;
        }
        // else { doesn't do anything. }
    } else {
        s->SRAM[location - 0xA000] = value;
        s->isSRAMDirty = 1;        
    }
}

/**
 * 0xC000 - 0xCFFF: Fixed RAM
 */
static void writeWRAMC(GbState* s, u16 location, byte value) {
    s->WRAMBanks[location - 0xc000] = value;
}

/**
 * 0xD000 - 0xDFFF: Switchable RAM
 */
static void writeWRAMD(GbState* s, u16 location, byte value) {
    s->WRAMBanks[s->GbcRamBankSelectRegister * WRAM_BANK_SIZE + location - 0xd000] = value; 
}

/**
 * 0xE000 - 0FDFF: Echoed of WRAM
 */
static void writeEcho(GbState* s, u16 location, byte value) {
    // return ERROR_UNUSED;
}

/**
 * FE00 - FE9F: OAM RAM - Sprite Attribute Table
 */
static void writeOam(GbState* s, u16 location, byte value) {
    if (location <= 0xFE9F) {
        s->OAM[location - 0xfe00] = value;
    }
}

static void writeHigh(GbState* s, u16 location, byte value) {
    mmuWriteHramTable[location & 0x00FF](s, location, value);
}

typedef void (*mmuWriteOperation)(GbState*, u16, u8);
static mmuWriteOperation mmuWriteTable[] = {
//        0           1           2           3           4           5           6           7           8           9           A           B           C           D           E           F       
/*   0 */ writeRom01, writeRom01, writeRom01, writeRom01, writeRom01, writeRom01, writeRom01, writeRom01, writeRom01, writeRom01, writeRom01, writeRom01, writeRom01, writeRom01, writeRom01, writeRom01,
/*   1 */ writeRom01, writeRom01, writeRom01, writeRom01, writeRom01, writeRom01, writeRom01, writeRom01, writeRom01, writeRom01, writeRom01, writeRom01, writeRom01, writeRom01, writeRom01, writeRom01,
/*   2 */ writeRom23, writeRom23, writeRom23, writeRom23, writeRom23, writeRom23, writeRom23, writeRom23, writeRom23, writeRom23, writeRom23, writeRom23, writeRom23, writeRom23, writeRom23, writeRom23,
/*   3 */ writeRom23, writeRom23, writeRom23, writeRom23, writeRom23, writeRom23, writeRom23, writeRom23, writeRom23, writeRom23, writeRom23, writeRom23, writeRom23, writeRom23, writeRom23, writeRom23,
/*   4 */ writeRom45, writeRom45, writeRom45, writeRom45, writeRom45, writeRom45, writeRom45, writeRom45, writeRom45, writeRom45, writeRom45, writeRom45, writeRom45, writeRom45, writeRom45, writeRom45,
/*   5 */ writeRom45, writeRom45, writeRom45, writeRom45, writeRom45, writeRom45, writeRom45, writeRom45, writeRom45, writeRom45, writeRom45, writeRom45, writeRom45, writeRom45, writeRom45, writeRom45,
/*   6 */ writeRom67, writeRom67, writeRom67, writeRom67, writeRom67, writeRom67, writeRom67, writeRom67, writeRom67, writeRom67, writeRom67, writeRom67, writeRom67, writeRom67, writeRom67, writeRom67,
/*   7 */ writeRom67, writeRom67, writeRom67, writeRom67, writeRom67, writeRom67, writeRom67, writeRom67, writeRom67, writeRom67, writeRom67, writeRom67, writeRom67, writeRom67, writeRom67, writeRom67,
/*   8 */ writeVRAM,  writeVRAM,  writeVRAM,  writeVRAM,  writeVRAM,  writeVRAM,  writeVRAM,  writeVRAM,  writeVRAM,  writeVRAM,  writeVRAM,  writeVRAM,  writeVRAM,  writeVRAM,  writeVRAM,  writeVRAM,
/*   9 */ writeVRAM,  writeVRAM,  writeVRAM,  writeVRAM,  writeVRAM,  writeVRAM,  writeVRAM,  writeVRAM,  writeVRAM,  writeVRAM,  writeVRAM,  writeVRAM,  writeVRAM,  writeVRAM,  writeVRAM,  writeVRAM,
/*   A */ writeSRAM,  writeSRAM,  writeSRAM,  writeSRAM,  writeSRAM,  writeSRAM,  writeSRAM,  writeSRAM,  writeSRAM,  writeSRAM,  writeSRAM,  writeSRAM,  writeSRAM,  writeSRAM,  writeSRAM,  writeSRAM,
/*   B */ writeSRAM,  writeSRAM,  writeSRAM,  writeSRAM,  writeSRAM,  writeSRAM,  writeSRAM,  writeSRAM,  writeSRAM,  writeSRAM,  writeSRAM,  writeSRAM,  writeSRAM,  writeSRAM,  writeSRAM,  writeSRAM,
/*   C */ writeWRAMC, writeWRAMC, writeWRAMC, writeWRAMC, writeWRAMC, writeWRAMC, writeWRAMC, writeWRAMC, writeWRAMC, writeWRAMC, writeWRAMC, writeWRAMC, writeWRAMC, writeWRAMC, writeWRAMC, writeWRAMC, 
/*   D */ writeWRAMD, writeWRAMD, writeWRAMD, writeWRAMD, writeWRAMD, writeWRAMD, writeWRAMD, writeWRAMD, writeWRAMD, writeWRAMD, writeWRAMD, writeWRAMD, writeWRAMD, writeWRAMD, writeWRAMD, writeWRAMD, 
/*   E */ writeEcho,  writeEcho,  writeEcho,  writeEcho,  writeEcho,  writeEcho,  writeEcho,  writeEcho,  writeEcho,  writeEcho,  writeEcho,  writeEcho,  writeEcho,  writeEcho,  writeEcho,  writeEcho,
/*   F */ writeEcho,  writeEcho,  writeEcho,  writeEcho,  writeEcho,  writeEcho,  writeEcho,  writeEcho,  writeEcho,  writeEcho,  writeEcho,  writeEcho,  writeEcho,  writeEcho,  writeOam,   writeHigh
};

void mmu_write(GbState* s, u16 location, byte value) {
    mmuWriteTable[location >> 8](s, location, value);
}

static byte readHigh(GbState* s, u16 location) {
    u8 lowcation = location & 0x00FF;
    return mmuReadHramTable[lowcation](s, lowcation);
}

static byte readRom0(GbState* s, u16 location) {
    return s->ROM0[location];    
}

static byte readRomX(GbState* s, u16 location) {
    return s->ROMX[location - 0x4000];
}   

static byte readVRAM(GbState* s, u16 location) {
    return s->VRAMBanks[s->GbcVRAMBank * VRAM_BANK_SIZE + location - 0x8000];
}

static byte readSRAM(GbState* s, u16 location) {
    if (s->isSRAMDisabled || !s->hasSRAM) {
        return 0xFF;
    }

    // May need RTC stuff
    if (s->mbc == 3 && s->mem_mbc3_extram_rtc_select < 0x04) {
        if (s->mem_mbc3_extram_rtc_select >= 0x08 && s->mem_mbc3_extram_rtc_select <= 0x0c) {
            return s->mem_RTC[s->mem_mbc3_extram_rtc_select];
        } else {
            return 0x00;
        }
    } else {  
        return s->SRAM[location - 0xA000];
    }
}

static byte readWRAM(GbState* s, u16 location) {
    if (location < 0xD000) {
        return s->WRAM0[location - 0xc000];
    } else {
        return s->WRAMBanks[s->GbcRamBankSelectRegister * WRAM_BANK_SIZE + location - 0xd000];
    }
}

static byte readEcho(GbState* s, u16 location) {
    return readWRAM(s, location - 0x2000);
}
 
static byte readOAM(GbState* s, u16 location) {
    if (location <= 0xFE9F) {
        return s->OAM[location - 0xfe00];
    } else {
        return 0;
    }
}

typedef byte (*mmuReadOperation)(GbState*, u16);
static mmuReadOperation mmuReadTable[] = {
//        0         1         2         3         4         5         6         7         8         9         A         B         C         D         E         F       
/*   0 */ readRom0, readRom0, readRom0, readRom0, readRom0, readRom0, readRom0, readRom0, readRom0, readRom0, readRom0, readRom0, readRom0, readRom0, readRom0, readRom0,
/*   1 */ readRom0, readRom0, readRom0, readRom0, readRom0, readRom0, readRom0, readRom0, readRom0, readRom0, readRom0, readRom0, readRom0, readRom0, readRom0, readRom0,
/*   2 */ readRom0, readRom0, readRom0, readRom0, readRom0, readRom0, readRom0, readRom0, readRom0, readRom0, readRom0, readRom0, readRom0, readRom0, readRom0, readRom0, 
/*   3 */ readRom0, readRom0, readRom0, readRom0, readRom0, readRom0, readRom0, readRom0, readRom0, readRom0, readRom0, readRom0, readRom0, readRom0, readRom0, readRom0, 
/*   4 */ readRomX, readRomX, readRomX, readRomX, readRomX, readRomX, readRomX, readRomX, readRomX, readRomX, readRomX, readRomX, readRomX, readRomX, readRomX, readRomX,
/*   5 */ readRomX, readRomX, readRomX, readRomX, readRomX, readRomX, readRomX, readRomX, readRomX, readRomX, readRomX, readRomX, readRomX, readRomX, readRomX, readRomX,
/*   6 */ readRomX, readRomX, readRomX, readRomX, readRomX, readRomX, readRomX, readRomX, readRomX, readRomX, readRomX, readRomX, readRomX, readRomX, readRomX, readRomX,
/*   7 */ readRomX, readRomX, readRomX, readRomX, readRomX, readRomX, readRomX, readRomX, readRomX, readRomX, readRomX, readRomX, readRomX, readRomX, readRomX, readRomX,
/*   8 */ readVRAM, readVRAM, readVRAM, readVRAM, readVRAM, readVRAM, readVRAM, readVRAM, readVRAM, readVRAM, readVRAM, readVRAM, readVRAM, readVRAM, readVRAM, readVRAM,
/*   9 */ readVRAM, readVRAM, readVRAM, readVRAM, readVRAM, readVRAM, readVRAM, readVRAM, readVRAM, readVRAM, readVRAM, readVRAM, readVRAM, readVRAM, readVRAM, readVRAM,
/*   A */ readSRAM, readSRAM, readSRAM, readSRAM, readSRAM, readSRAM, readSRAM, readSRAM, readSRAM, readSRAM, readSRAM, readSRAM, readSRAM, readSRAM, readSRAM, readSRAM,
/*   B */ readSRAM, readSRAM, readSRAM, readSRAM, readSRAM, readSRAM, readSRAM, readSRAM, readSRAM, readSRAM, readSRAM, readSRAM, readSRAM, readSRAM, readSRAM, readSRAM,
/*   C */ readWRAM, readWRAM, readWRAM, readWRAM, readWRAM, readWRAM, readWRAM, readWRAM, readWRAM, readWRAM, readWRAM, readWRAM, readWRAM, readWRAM, readWRAM, readWRAM, 
/*   D */ readWRAM, readWRAM, readWRAM, readWRAM, readWRAM, readWRAM, readWRAM, readWRAM, readWRAM, readWRAM, readWRAM, readWRAM, readWRAM, readWRAM, readWRAM, readWRAM, 
/*   E */ readEcho, readEcho, readEcho, readEcho, readEcho, readEcho, readEcho, readEcho, readEcho, readEcho, readEcho, readEcho, readEcho, readEcho, readEcho, readEcho,
/*   F */ readEcho, readEcho, readEcho, readEcho, readEcho, readEcho, readEcho, readEcho, readEcho, readEcho, readEcho, readEcho, readEcho, readEcho, readOAM,  readHigh,
};

byte mmu_read(GbState* s, u16 location) {
    return mmuReadTable[location >> 8](s, location);
}

u16 mmu_read16(GbState *s, u16 location) {
    return mmu_read(s, location) | ((u16)mmu_read(s, location + 1) << 8);
}

void mmu_write16(GbState *s, u16 location, u16 value) {
    mmu_write(s, location, value & 0xff);
    mmu_write(s, location + 1, value >> 8);
}

u16 mmu_pop16(GbState *s) {
    u16 val = mmu_read16(s, s->sp);
    s->sp += 2;
    return val;
}

void mmu_push16(GbState *s, u16 value) {
    s->sp -= 2;
    mmu_write16(s, s->sp, value);
}