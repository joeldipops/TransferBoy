#include <stdio.h>

#include "mmu.h"
#include "hwdefs.h"
#include "rtc.h"

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
        // Underflow meant we copied the last block and are done.
        s->io_hdma_running = 0;
    }
}

static void mmu_hdma_start(GbState *s, u8 lenmode) {
    u16 blocks = (lenmode & ~(1<<7)) + 1;
    u16 len = blocks * 0x10;
    u8 mode_hblank = (lenmode & (1<<7)) ? 1 : 0;
    u16 src = (s->GbcHdmaSource & 0xFFF0);
    u16 dst = (s->GbcHdmaDestination & 0x1FF0) | 0x8000;

    if (s->io_hdma_running && !mode_hblank) {
        // Cancel ongoing H-Blank HDMA transfer
        s->io_hdma_running = 0;
        s->GbcHdmaControl = 0xff; // done
        return;
    }

    if (!mode_hblank) {
        for (u16 i = 0; i < len; i++)
            mmu_write(s, dst++, mmu_read(s, src++));

        s->GbcHdmaControl = 0xff; // done
        u32 clks = blocks * GB_HDMA_BLOCK_CLKS;
        if (s->double_speed)
            clks *= 2;
        s->last_op_cycles += clks;
    } else {
        s->io_hdma_running = 1;
        s->io_hdma_next_src = src;
        s->io_hdma_next_dst = dst;
        s->GbcHdmaControl = blocks - 1;

        // If in H-Blank
        if ((s->LcdStatus & 3) == 0) 
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
    s->VRAM = s->VRAMBanks + (s->GbcVRAMBank * VRAM_BANK_SIZE); 
}

static void writeBiosSwitch(GbState* s, byte offset, byte value) {
    s->in_bios = 0;
    // Blow away the bios.
    s->ROM0 = s->Cartridge->Rom.Data;
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
    value &= (s->WRAMBankCount - 1);
    s->GbcRamBankSelectRegister = value; 

    if (value == 0) {
        value++;
    }

    s->WRAMX = s->WRAMBanks + (value * WRAM_BANK_SIZE);     
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

/**
 * When a write operation should do nothing.
 * eg. writing to ROM or when there is no SRAM
 */
static void writeNone(GbState* s, u16 location, byte value) {
    ;
}

/**
 * When the result of a read operation is undefined.
 */
static byte readNone(GbState* s, u16 location) {
    return 0xFF;
}

/**
 * 0000 - 1FFF: Fixed ROM bank
 * Writing here tends to disable or enable external RAM.
 */
static void writeRom01(GbState* s, u16 location, byte value) {
    value &= 0xF;

    if (value == 0) {
        s->isSRAMDisabled = true;          
    } else if (value == 0x0A) {
        s->isSRAMDisabled = false;
    }
}

/**
 * Combines both mbc1 bank registers to get the bank to switch to.
 * @param lower The lower five bits at 0x2000-0x3FFF
 * @param upper The upper two bits at 0x4000-0x5FFF
 * @param bankCount The number of ROM banks this particular cartridge uses.
 * @returns Selected bank to place at 0x4000 - 0x7FFF
 */
static inline natural getMbc1RomBank(const byte lower, const byte upper, const byte bankCount) {
    byte result = (upper << 5) | lower;

    // MBC1 RomBankLower can't be 0, so add 1 to these values 
    if (result == 0x00 || result == 0x20 || result == 0x40 || result == 0x60) {
        result++;
    }

    // Don't switch to a bank that doesn't exist. Wrap around instead.
    if (result >= bankCount) {
        result = result % bankCount;
    }    

    return result;
}

/**
 * 2000 - 3FFF: ROM bank switch
 * Sets the lower part of the ROM bank number and then copies the bank in to ROMX.
 */
static void mbc1writeRom23(GbState* s, u16 location, byte value) {
    value &= 0x1F;
    
    byte bank = getMbc1RomBank(value, s->RomBankUpper, s->Cartridge->RomBankCount);

    s->RomBankLower = bank & 0x1F;    
    s->ROMX = s->Cartridge->Rom.Data + (bank * ROM_BANK_SIZE);    
}

/**
 * 0000 - 3FFF: SRAM enable & ROM bank switch
 * Unlike most MBCs, mbc2 has the same behaviour when writting to 0x0000-0x1FFF or 0x2000-0x3FFF
 */
static void mbc2writeRom0123(GbState* s, u16 location, byte value) {
    // Only the lowest 4 bits matter
    s->RomBankLower = value & 0xF;

    if (s->RomBankLower == 0) {
        s->RomBankLower++;
    }        

    if (s->RomBankLower >= s->Cartridge->RomBankCount) {
        s->RomBankLower = s->RomBankLower % s->Cartridge->RomBankCount;
    }   

    s->ROMX = s->Cartridge->Rom.Data + (s->RomBankLower * ROM_BANK_SIZE);      
}

/**
 * 2000 - 3FFF: ROM bank switch
 * Sets the ROM bank number and then copies the bank in to ROMX.
 */
static void mbc3writeRom23(GbState* s, u16 location, byte value) {
    s->RomBankLower = value & 0x7f;
    byte bank = s->RomBankLower < 1 ? s->RomBankLower + 1 : s->RomBankLower;
    s->ROMX = s->Cartridge->Rom.Data + (bank * ROM_BANK_SIZE);       
}

/**
 * Switches current ROMX bank of an mbc5 cartridge based on RomBankLower and RomBankUpper registers
 */
static inline void mbc5switchRomBank(GbState* s, u16 location, byte value) {
    byte bank = (s->RomBankUpper << 8) | s->RomBankLower;

    if (bank >= s->Cartridge->RomBankCount) {
        bank = bank % s->Cartridge->RomBankCount;
    }

    s->ROMX = s->Cartridge->Rom.Data + (bank * ROM_BANK_SIZE);        
}

/**
 * 2000 - 2FFF: ROM bank lower switch
 */
static void mbc5writeRom2(GbState* s, u16 location, byte value) {
    s->RomBankLower = value;
    mbc5switchRomBank(s, location, value);
}

/**
 * 3000 - 3FFF: ROM bank upper switch.
 */
static void mbc5writeRom3(GbState* s, u16 location, byte value) {
    s->RomBankUpper = value & 1;     
    mbc5switchRomBank(s, location, value);    
}

/**
 * 0x4000 - 0x5FFF: ROM/RAM bank switch
 * Sets the RAM bank number or the upper part of the ROM bank number depending on the mbc type and current state.
 */
static void mbc1writeRom45(GbState* s, u16 location, byte value) {
    value &= 3;
    if (s->RomRamSelect == ROM_SELECT || s->Cartridge->RomBankCount > 0x1F) {
        // Apparently for large MBC1s, writing here DOES update ROMX, even in RAM mode!
        // According to the mooneye-gb tests that is.
        if (s->Cartridge->RomBankCount > 0x1F) {
            byte bank = getMbc1RomBank(s->RomBankLower, value, s->Cartridge->RomBankCount);

            s->RomBankUpper = bank >> 5;
            s->ROMX = s->Cartridge->Rom.Data + (bank * ROM_BANK_SIZE);                
        }

        s->ROM0 = s->Cartridge->Rom.Data;

        if (s->RomRamSelect == ROM_SELECT) {
            return;
        }
    }
        
    if (s->RomRamSelect == SRAM_SELECT) {
        // For MBC1 8 and 16Mbit carts, ROM0 actually changes when you write in RAM mode.
        // https://raw.githubusercontent.com/Gekkio/gb-ctr/master/xx-mbc1.tex
        if (s->Cartridge->RomBankCount > 0x20) {
            byte bank = value << 5;
            if (bank >= s->Cartridge->RomBankCount) {
                bank = bank % s->Cartridge->RomBankCount;
            }

            s->ROM0 = s->Cartridge->Rom.Data + (bank) * ROM_BANK_SIZE;
        }

        if (s->Cartridge->RamBankCount <= 1) {
            return;
        }

        s->SRAMBankNumber = value;
    }

    // Swap in new RAM bank.
    s->SRAM = s->Cartridge->Ram.Data + s->SRAMBankNumber * SRAM_BANK_SIZE;    
}

/**
 * 0x4000 - 0x5FFF: ROM/RAM bank switch
 * Sets the RAM bank number.
 */
static void mbc3writeRom45(GbState* s, u16 location, byte value) {
    s->SRAMBankNumber = value;

    // Swap in new RAM bank.
    s->SRAM = s->Cartridge->Ram.Data + s->SRAMBankNumber * SRAM_BANK_SIZE;    
}

/**
 * 0x4000 - 0x5FFF: ROM/RAM bank switch
 * Sets the RAM bank number.
 */
static void mbc5writeRom45(GbState* s, u16 location, byte value) {
    s->SRAMBankNumber = value & 0xf;
    s->SRAMBankNumber &= s->Cartridge->RamBankCount - 1;

    // Swap in new RAM bank.
    s->SRAM = s->Cartridge->Ram.Data + s->SRAMBankNumber * SRAM_BANK_SIZE;       
}

/**
 * 0x6000 - 0x7FFF: Sets the Banking mode.
 */
static void mbc1writeRom67(GbState* s, u16 location, byte value) {
    s->RomRamSelect = value & 0x1;
    if (s->RomRamSelect == ROM_SELECT) {
        s->SRAMBankNumber = 0;
        s->SRAM = s->Cartridge->Ram.Data;   
    } else {
        s->RomBankUpper = 0;
        s->ROMX = s->Cartridge->Rom.Data + (getMbc1RomBank(s->RomBankLower, s->RomBankUpper, s->Cartridge->RomBankCount) * ROM_BANK_SIZE);
    }
}

/**
 * 0x6000 - 0x7FFF: Latches or unlatches the clock.
 */
static void hasRTCwriteRom67(GbState* s, u16 location, byte value) {
    // To update the RTC values in SRAM space, clock should be unlatched to 0 and then latched again to 1
    if (!s->isRTCLatched && value == 0x01) {
        updateRealTimeClock(s);
    }
    s->isRTCLatched = value;
}

/**
 * 0x8000 - 0x9FFF: VRAM
 */
static void writeVRAM(GbState* s, u16 location, byte value) {
    s->VRAM[location - 0x8000] = value;    
}

/**
 * 0xA000 -0xBFFF: Cartridge RAM
 * Only lower four bits of every byte counts.
 * Also there're only 0x0200 available addresses.
 */
static void mbc2writeSRAM(GbState* s, u16 location, byte value) {
    if (!s->isSRAMDisabled) {
        s->SRAM[(location - 0xA000) % 0x200] = value | 0xF0;    
        s->isSRAMDirty = 1;
    }
}

/**
 * Updates SRAM, or the Real Time Clock depending on what RAM bank is selected.
 */
static void hasRTCwriteSRAM(GbState* s, u16 location, byte value) {
    if(s->SRAMBankNumber >= 0x08) {
        if (s->SRAMBankNumber == 0x0C) {
            // We may need to pause or restart the RTC
            RealTimeClockStatus* newClockStatus = (RealTimeClockStatus*) &value;
            RealTimeClockStatus* lastClockStatus = (RealTimeClockStatus*) (s->SRAM + (location - 0xA000));

            if (newClockStatus->IsTimerStopped != lastClockStatus->IsTimerStopped) {
                toggleRealTimeClock(s, newClockStatus->IsTimerStopped);
            }
        }

        // Whole bank is filled with the same value when RTC is updated.
        memset(s->SRAM, value, SRAM_BANK_SIZE);
    } else {
        s->SRAM[location - 0xA000] = value;
    }

    s->isSRAMDirty = 1;        
}

/**
 * 0xA000 -0xBFFF: Cartridge RAM
 */
static void writeSRAM(GbState* s, u16 location, byte value) {
    // Could potentially point SRAM to some junk address when sram is disabled
    // if this check has any perf impact.
    if (!s->isSRAMDisabled && s->hasSRAM) {
        s->SRAM[location - 0xA000] = value;
        s->isSRAMDirty = 1;        
    }
}

/**
 * 0xC000 - 0xCFFF: Fixed RAM
 */
static void writeWRAM0(GbState* s, u16 location, byte value) {
    s->WRAM0[location - 0xc000] = value;
}

/**
 * 0xD000 - 0xDFFF: Switchable RAM
 */
static void writeWRAMX(GbState* s, u16 location, byte value) {
    s->WRAMX[location - 0xd000] = value; 
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

static const mmuWriteOperation mmuWriteTable[] = {
//        0           1           2           3           4           5           6           7           8           9           A           B           C           D           E           F       
/*   0 */ writeRom01, writeRom01, writeRom01, writeRom01, writeRom01, writeRom01, writeRom01, writeRom01, writeRom01, writeRom01, writeRom01, writeRom01, writeRom01, writeRom01, writeRom01, writeRom01,
/*   1 */ writeRom01, writeRom01, writeRom01, writeRom01, writeRom01, writeRom01, writeRom01, writeRom01, writeRom01, writeRom01, writeRom01, writeRom01, writeRom01, writeRom01, writeRom01, writeRom01,
/*   2 */ writeNone,   writeNone,   writeNone,   writeNone,   writeNone,   writeNone,   writeNone,   writeNone,   writeNone,   writeNone,   writeNone,   writeNone,   writeNone,   writeNone,   writeNone,   writeNone,
/*   3 */ writeNone,   writeNone,   writeNone,   writeNone,   writeNone,   writeNone,   writeNone,   writeNone,   writeNone,   writeNone,   writeNone,   writeNone,   writeNone,   writeNone,   writeNone,   writeNone,
/*   4 */ writeNone,   writeNone,   writeNone,   writeNone,   writeNone,   writeNone,   writeNone,   writeNone,   writeNone,   writeNone,   writeNone,   writeNone,   writeNone,   writeNone,   writeNone,   writeNone,
/*   5 */ writeNone,   writeNone,   writeNone,   writeNone,   writeNone,   writeNone,   writeNone,   writeNone,   writeNone,   writeNone,   writeNone,   writeNone,   writeNone,   writeNone,   writeNone,   writeNone,
/*   6 */ writeNone,   writeNone,   writeNone,   writeNone,   writeNone,   writeNone,   writeNone,   writeNone,   writeNone,   writeNone,   writeNone,   writeNone,   writeNone,   writeNone,   writeNone,   writeNone,
/*   7 */ writeNone,   writeNone,   writeNone,   writeNone,   writeNone,   writeNone,   writeNone,   writeNone,   writeNone,   writeNone,   writeNone,   writeNone,   writeNone,   writeNone,   writeNone,   writeNone,
/*   8 */ writeVRAM,  writeVRAM,  writeVRAM,  writeVRAM,  writeVRAM,  writeVRAM,  writeVRAM,  writeVRAM,  writeVRAM,  writeVRAM,  writeVRAM,  writeVRAM,  writeVRAM,  writeVRAM,  writeVRAM,  writeVRAM,
/*   9 */ writeVRAM,  writeVRAM,  writeVRAM,  writeVRAM,  writeVRAM,  writeVRAM,  writeVRAM,  writeVRAM,  writeVRAM,  writeVRAM,  writeVRAM,  writeVRAM,  writeVRAM,  writeVRAM,  writeVRAM,  writeVRAM,
/*   A */ writeSRAM,  writeSRAM,  writeSRAM,  writeSRAM,  writeSRAM,  writeSRAM,  writeSRAM,  writeSRAM,  writeSRAM,  writeSRAM,  writeSRAM,  writeSRAM,  writeSRAM,  writeSRAM,  writeSRAM,  writeSRAM,
/*   B */ writeSRAM,  writeSRAM,  writeSRAM,  writeSRAM,  writeSRAM,  writeSRAM,  writeSRAM,  writeSRAM,  writeSRAM,  writeSRAM,  writeSRAM,  writeSRAM,  writeSRAM,  writeSRAM,  writeSRAM,  writeSRAM,
/*   C */ writeWRAM0, writeWRAM0, writeWRAM0, writeWRAM0, writeWRAM0, writeWRAM0, writeWRAM0, writeWRAM0, writeWRAM0, writeWRAM0, writeWRAM0, writeWRAM0, writeWRAM0, writeWRAM0, writeWRAM0, writeWRAM0, 
/*   D */ writeWRAMX, writeWRAMX, writeWRAMX, writeWRAMX, writeWRAMX, writeWRAMX, writeWRAMX, writeWRAMX, writeWRAMX, writeWRAMX, writeWRAMX, writeWRAMX, writeWRAMX, writeWRAMX, writeWRAMX, writeWRAMX, 
/*   E */ writeEcho,  writeEcho,  writeEcho,  writeEcho,  writeEcho,  writeEcho,  writeEcho,  writeEcho,  writeEcho,  writeEcho,  writeEcho,  writeEcho,  writeEcho,  writeEcho,  writeEcho,  writeEcho,
/*   F */ writeEcho,  writeEcho,  writeEcho,  writeEcho,  writeEcho,  writeEcho,  writeEcho,  writeEcho,  writeEcho,  writeEcho,  writeEcho,  writeEcho,  writeEcho,  writeEcho,  writeOam,   writeHigh
};

/**
 * Writes to an address in the virtual gameboy memory.
 * @param s Gameboy state.
 * @param location address to write to.
 * @param value Value to write.
 */
inline void mmu_write(GbState* s, u16 location, byte value) {
    s->mmuWrites[location >> 8](s, location, value);
}

static byte readHigh(GbState* s, u16 location) {
    u8 lowcation = location & 0x00FF;
    return mmuReadHramTable[lowcation](s, lowcation);
}

/**
 * 0x0000 - 0x3FFF
 */
static byte readRom0(GbState* s, u16 location) {
    return s->ROM0[location];    
}

/**
 * 0x4000 - 0x7FFF
 */
static byte readRomX(GbState* s, u16 location) {
    return s->ROMX[location - 0x4000];
}   


/**
 * 0x0 - 0x0
 */
static byte readVRAM(GbState* s, u16 location) {
    return s->VRAM[location - 0x8000];
}

/**
 * 0xA000 - 0xBFFF
 */
static byte mbc2readSRAM(GbState* s, u16 location) {
    if (s->isSRAMDisabled) {
        return 0xFF;
    } else {
        return s->SRAM[(location - 0xA000) % 0x200] | 0xF0;
    }
}

/**
 * 0xA000 - 0xBFFF
 */
static byte readSRAM(GbState* s, u16 location) {
    if (s->isSRAMDisabled || !s->hasSRAM) {
        return 0xFF;
    } else {
        return s->SRAM[location - 0xA000];
    }
    // May need a block for RTC stuff accuracy
}

/**
 * 0xC000 - 0xCFFF
 */
static byte readWRAM0(GbState* s, u16 location) {
    return s->WRAM0[location - 0xc000];
}

/**
 * 0xD000 - 0xDFFF
 */
static byte readWRAMX(GbState* s, u16 location) {
    return s->WRAMX[location - 0xd000];
}

static byte readEcho(GbState* s, u16 location) {
    if (location < 0xF000) {
        return readWRAM0(s, location - 0x2000);     
    } else {
        return readWRAMX(s, location - 0x2000);
    }
}
 
static byte readOAM(GbState* s, u16 location) {
    if (location <= 0xFE9F) {
        return s->OAM[location - 0xfe00];
    } else {
        return 0;
    }
}

static const mmuReadOperation mmuReadTable[] = {
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
/*   C */ readWRAM0, readWRAM0, readWRAM0, readWRAM0, readWRAM0, readWRAM0, readWRAM0, readWRAM0, readWRAM0, readWRAM0, readWRAM0, readWRAM0, readWRAM0, readWRAM0, readWRAM0, readWRAM0, 
/*   D */ readWRAMX, readWRAMX, readWRAMX, readWRAMX, readWRAMX, readWRAMX, readWRAMX, readWRAMX, readWRAMX, readWRAMX, readWRAMX, readWRAMX, readWRAMX, readWRAMX, readWRAMX, readWRAMX, 
/*   E */ readEcho, readEcho, readEcho, readEcho, readEcho, readEcho, readEcho, readEcho, readEcho, readEcho, readEcho, readEcho, readEcho, readEcho, readEcho, readEcho,
/*   F */ readEcho, readEcho, readEcho, readEcho, readEcho, readEcho, readEcho, readEcho, readEcho, readEcho, readEcho, readEcho, readEcho, readEcho, readOAM,  readHigh,
};

/**
 * Reads from an address in the virtual gameboy memory.
 * @param s Gameboy state.
 * @param location address to read from.
 * @returns value at that address.
 */
inline byte mmu_read(GbState* s, u16 location) {
    return s->mmuReads[location >> 8](s, location);
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

/**
 * Installs different mmu behaviour for different Memory Bank Controller types.
 * @param s Gameboy state.
 */
void mmu_install_mbc(GbState* s) {
    memcpy(s->mmuReads, mmuReadTable, sizeof(mmuReadOperation) * 0x100);
    memcpy(s->mmuWrites, mmuWriteTable, sizeof(mmuWriteOperation) * 0x100);

    if (s->Cartridge->Type == MBC1) {
        for (byte i = 0x20; i < 0x40; i++) {
            s->mmuWrites[i] = mbc1writeRom23;
        }
        for (byte i = 0x40; i < 0x60; i++) {
            s->mmuWrites[i] = mbc1writeRom45;
        }
        for (byte i = 0x60; i < 0x80; i++) {
            s->mmuWrites[i] = mbc1writeRom67;
        }
    } else if (s->Cartridge->Type == MBC2) {
        for (byte i = 0x00; i < 0x40; i++) {
            if (i & 0x01) {
                s->mmuWrites[i] = mbc2writeRom0123;
            } else {
                s->mmuWrites[i] = writeRom01;
            }
        }
        for (byte i = 0xA0; i < 0xC0; i++) {
            s->mmuWrites[i] = mbc2writeSRAM;
            s->mmuReads[i] = mbc2readSRAM;
        }
    } else if (s->Cartridge->Type == MBC3) {
        for (byte i = 0x20; i < 0x40; i++) {
            s->mmuWrites[i] = mbc3writeRom23;
        }
        for (byte i = 0x40; i < 0x60; i++) {
            s->mmuWrites[i] = mbc3writeRom45;
        }
    } else if (s->Cartridge->Type == MBC5) {
        for (byte i = 0x20; i < 0x30; i++) {
            s->mmuWrites[i] = mbc5writeRom2;
        }
        for (byte i = 0x30; i < 0x40; i++) {
            s->mmuWrites[i] = mbc5writeRom3;
        }        
        for (byte i = 0x40; i < 0x60; i++) {
            s->mmuWrites[i] = mbc5writeRom45;
        }        
    }

    if (s->hasRTC) {
        for (byte i = 0x60; i < 0x80; i++) {
            s->mmuWrites[i] = hasRTCwriteRom67;
        }
        for (byte i = 0xA0; i < 0xC0; i++) {
            s->mmuWrites[i] = hasRTCwriteSRAM;
        }
    }

    if (!s->hasSRAM) {
        for (byte i = 0xA0; i < 0xC0; i++) {
            s->mmuWrites[i] = writeNone;
            s->mmuReads[i] = readNone;
        }
    }        
}