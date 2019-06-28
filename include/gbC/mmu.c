#include <stdio.h>

#include "mmu.h"
#include "hwdefs.h"
#include "logger.h"

#if 1
#define MMU_DEBUG_W(fmt, ...) \
    do { \
        if (s->emu_state->dbg_print_mmu) \
            printf(" [MMU] [W] " fmt " @%x: %x\n", ##__VA_ARGS__, location, value); \
    } while(0)

#define MMU_DEBUG_R(fmt, ...) \
    do { \
        if (s->emu_state->dbg_print_mmu) \
            printf(" [MMU] [R] " fmt "\n", ##__VA_ARGS__); \
    } while(0)
#else
#define MMU_DEBUG_W(...)
#define MMU_DEBUG_R(...)
#endif

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
    s->emu_state->last_op_cycles += clks;

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
            s->mem_bank_rom, s->pc,  src, dst, blocks, mode_hblank);

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
        s->emu_state->last_op_cycles += clks;
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
    if (s->emu_state->lcd_entered_hblank && s->io_hdma_running)
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
        s->mem_OAM[i] = mmu_read(s, (value << 8) + i);
    }
}

static void writeGbcVramBank(GbState* s, byte offset, byte value) {
    s->GbcVramBank = value & 1;
}

static void writeBiosSwitch(GbState* s, byte offset, byte value) {
    s->in_bios = 0;
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
    value &= s->mem_num_banks_wram - 1;
    s->GbcRamBankSelectRegister = value;  
}


static void writeHram(GbState* s, byte offset, byte value) {
    s->HRAM[offset] = value;
}

 typedef void (*mmuWriteOperation)(GbState*, byte, byte);
 static mmuWriteOperation mmuWriteTable[] = {
//        0         1         2         3         4         5         6         7         8         9         A         B         C         D         E         F   
/*   0 */ writeHram,writeHram,writeHram,writeHram,writeHram,writeHram,writeHram,writeHram,writeHram,writeHram,writeHram,writeHram,writeHram,writeHram,writeHram,writeHram,
/*   1 */ writeHram,writeHram,writeHram,writeHram,writeHram,writeHram,writeHram,writeHram,writeHram,writeHram,writeHram,writeHram,writeHram,writeHram,writeHram,writeHram,
/*   2 */ writeHram,writeHram,writeHram,writeHram,writeHram,writeHram,writeAudioChannelSwitch,writeHram,writeHram,writeHram,writeHram,writeHram,writeHram,writeHram,writeHram,writeHram,
/*   3 */ writeHram,writeHram,writeHram,writeHram,writeHram,writeHram,writeHram,writeHram,writeHram,writeHram,writeHram,writeHram,writeHram,writeHram,writeHram,writeHram,
/*   4 */ writeHram,writeLcdStatus,writeHram,writeHram,writeCurrentLine,writeNextInterruptLine,writeDmaSource,writeHram,writeHram,writeHram,writeHram,writeHram,writeHram,writeHram,writeHram,writeGbcVramBank,
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

// FF4F GbcVramBank
static byte readGbcVramBank(GbState* s, byte offset) {
    return s->GbcVramBank & 1;
}

// FF69 Background Palette data
static byte readBackgroundPaletteData(GbState* s, byte offset) {
    return s->io_lcd_BGPD[s->GbcBackgroundPaletteIndexRegister & 0x3f];    
}

// FF6B Sprite Palette data
static byte readSpritePaletteData(GbState* s, byte offset) {
    return s->io_lcd_OBPD[s->GbcSpritePaletteIndexRegister & 0x3f];
}

typedef byte (*mmuReadOperation)(GbState*, byte);
static mmuReadOperation mmuReadTable[] = {
//        0         1         2         3         4         5         6         7         8         9         A         B         C         D         E         F       
/*   0 */ readJoypadIo, readHram, readHram, readHram, readHram, readHram, readHram, readHram, readHram, readHram, readHram, readHram, readHram, readHram, readHram, readHram, 
/*   1 */ readHram, readHram, readHram, readHram, readHram, readHram, readHram, readHram, readHram, readHram, readHram, readHram, readHram, readHram, readHram, readHram, 
/*   2 */ readHram, readHram, readHram, readHram, readHram, readHram, readHram, readHram, readHram, readHram, readHram, readHram, readHram, readHram, readHram, readHram, 
/*   3 */ readHram, readHram, readHram, readHram, readHram, readHram, readHram, readHram, readHram, readHram, readHram, readHram, readHram, readHram, readHram, readHram, 
/*   4 */ readHram, readHram, readHram, readHram, readHram, readHram, readHram, readHram, readHram, readHram, readHram, readHram, readHram, readHram, readHram, readGbcVramBank, 
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

void mmu_write(GbState *s, u16 location, u8 value) {
    // Jump straight to IO registers.
    if (location >= 0xFF00 && location < 0xFF80) {
        u8 lowcation = location & 0x00FF;

        mmuWriteTable[lowcation](s, lowcation, value);
        return;
    }

    u8 highcation = location >> 8;
     if (highcation < 0x80) {
        if (highcation < 0x40) {
            if (highcation < 0x20) {
                // 0000 - 1FFF: Fixed ROM bank

                // TODO - Lots of MBC implications here.
                if (value == 0) {
                    s->emu_state->extramDisabled = true;          
                } else if (value == 0x0A) {
                    s->emu_state->extramDisabled = false;
                }
            } else {
                // 2000 - 3FFF: Switchable ROM bank
                if (value == 0 && s->mbc != 5)
                    value = 1;

                if (s->mbc == 0)
                    ; //mmu_assert(value == 1);
                else if (s->mbc == 1)
                    value &= 0x1f;
                else if (s->mbc == 3)
                    value &= 0x7f;
                else if (s->mbc == 5) {
                    // MBC5 splits up this area into 2000-2fff for low bits rom bank,
                    // and 3000-3fff for the high bit.
                    if (location < 0x3000) // lower 8 bit
                        s->mem_bank_rom = (s->mem_bank_rom & (1<<8)) | value;
                    else // Upper bit
                        s->mem_bank_rom = (s->mem_bank_rom & 0xff) | ((value & 1) << 8);    
                } else {
                    mmu_error("Area not implemented for this MBC (mbc=%d, loc=%.4x, val=%x)\n", s->mbc, location, value);
                }
                s->mem_bank_rom = value;
            }
        } else {    
            if (highcation < 0x60) {
                // 0x4000 - 0x5FFF: More switchable ROM
                if (s->mbc == 1) {
                    if (s->mem_mbc1_romram_select == 0) { // ROM mode
                        s->mem_mbc1_rombankupper = value & 3;
                    } else {
                        s->mem_mbc1_extrambank = value & 3;
                        if (s->mem_num_banks_extram == 1)
                            s->mem_mbc1_extrambank &= 1;
                    }
                } else if (s->mbc == 3) {
                    if (value < 8)
                        ; //mmu_assert(value < s->mem_num_banks_extram);
                    else {
                        ; //mmu_assert(value <= 0xc); // RTC is at 08-0C
                        ; //mmu_assert(s->has_rtc);
                    }
                    s->mem_mbc3_extram_rtc_select = value;
                } else if (s->mbc == 5) {
                    s->mem_mbc5_extrambank = value & 0xf;
                    s->mem_mbc5_extrambank &= s->mem_num_banks_extram - 1;
                } else {
                    mmu_error("Area not implemented for this MBC (mbc=%d, loc=%.4x, val=%x)\n", s->mbc, location, value);
                }
            } else {
                                //0x6000 - 0x7FFF: More switchable ROM
                if (s->mbc == 1) {      
                    s->mem_mbc1_romram_select = value & 0x1;
                } else if (s->has_rtc) { // MBC3 only
                    if (s->mem_latch_rtc == 0x01 && value == 0x01) {
                        // TODO... actually latch something?
                        s->mem_latch_rtc = s->mem_latch_rtc;
                    }
                    s->mem_latch_rtc = value;
                } else if (s->mbc == 3 || s->mbc == 5) { // MBC3 without RTC or MBC5
                    // Just ignore it - Pokemon Red writes here because it's coded for
                    // MBC1, but actually has an MBC3, for instance
                } else {
                    mmu_error("Area not implemented for this MBC (mbc=%d, loc=%.4x, val=%x)\n", s->mbc, location, value);
                }
            }
        }
    } else {
        if (highcation < 0xC0) {
            if (highcation < 0xA0) {
                // 0x8000 - 0x9FFF: VRAM
                s->mem_VRAM[s->GbcVramBank * VRAM_BANKSIZE + location - 0x8000] = value;
            } else {
                // 0xA000 -0xBFFF: Cartridge RAM
                if (s->mbc == 1) {
                    if (!s->has_extram)
                        return;
                    if (s->mem_mbc1_romram_select == 1) { // RAM mode
                        s->mem_EXTRAM[s->mem_mbc1_extrambank * EXTRAM_BANKSIZE + location - 0xa000] = value;
                    } else { // ROM mode - we can only use bank 0
                        s->mem_EXTRAM[location - 0xa000] = value;
                    }
                    s->emu_state->extram_dirty = 1;
                } else if (s->mbc == 3) {
                    if (s->mem_mbc3_extram_rtc_select < 0x04) {
                        s->mem_EXTRAM[s->       mem_mbc3_extram_rtc_select * EXTRAM_BANKSIZE + location - 0xa000] = value;
                        s->emu_state->extram_dirty = 1;
                    } else if (s->mem_mbc3_extram_rtc_select >= 0x08 && s->mem_mbc3_extram_rtc_select <= 0x0c)
                        s->mem_RTC[s->mem_mbc3_extram_rtc_select] = value;
                    else
                        mmu_error("Writing to extram/rtc with invalid selection (%d) @%x, val=%x", s->mem_mbc3_extram_rtc_select, location, value);
                } else if (s->mbc == 5) {
                    if (!s->has_extram)
                        return;
                    s->mem_EXTRAM[s->mem_mbc5_extrambank * EXTRAM_BANKSIZE + location - 0xa000] = value;
                    s->emu_state->extram_dirty = 1;
                } else
                    mmu_error("Area not implemented for this MBC (mbc=%d, loc=%.4x, val=%x)\n", s->mbc, location, value);
                return;
            }
        } else {
            if (highcation < 0xE0) {
                if (highcation < 0xD0) {
                    // 0xC000 - 0xCFFF: Fixed RAM
                    s->mem_WRAM[location - 0xc000] = value;
                } else {
                    // 0xD000 - 0xDFFF: Switchable RAM
                    s->mem_WRAM[s->GbcRamBankSelectRegister * WRAM_BANKSIZE + location - 0xd000] = value; 
                }
            } else { 
                if (highcation < 0xF0) {
                    // 0xE000 - 0FDFF: Echoed RAM
                    mmu_error("Writing to ECHO area (0xc00-0xfdff) @%x, val=%x", location, value);
                } else {
                    if (highcation < 0xFF) {
                        if (location < 0xfdff) {
                            // F000 - FDFF: More Echoed RAM
                            mmu_error("Writing to ECHO area (0xc00-0xfdff) @%x, val=%x", location, value);
                        } else {
                            if (location < 0xfea0) {
                                // FE00 - FE9F: OAM RAM - Sprite Attribute Table
                                s->mem_OAM[location - 0xfe00] = value;
                            } else {
                                // FEA0 - FEFF: Unused
                                mmu_error("Writing to unusable area @%x, val=%x", location, value);
                            }
                        }
                    } else {
                        if (location < 0xFFFF) {
                            // 0xFF80 - FFFE: Stack RAM
                            s->mem_HRAM[location - 0xff80] = value;    
                        } else {
                            s->InterruptSwitch = value;
                        }
                    }
                }
            }   
        }
    }
}

/**
 * Optimises lookup of opcode of PC
 * @param s gameboy state
 * @param programCounter value of PC register.
 * @return opcode that PC is pointing to.
 */
u8 getOpCodeFromROM(GbState *s, const u16 programCounter) {
    if (programCounter < 0x4000) {
        // Fixed Bank
        return s->mem_ROM[programCounter];
    } else if (programCounter < 0x8000) {
        // Switchable bank
        u8 bank = s->mem_bank_rom;
        if (s->mbc == 1 && s->mem_mbc1_romram_select == 0) {
            bank |= s->mem_mbc1_rombankupper << 5;
        }
        bank &= s->mem_num_banks_rom - 1;
        return s->mem_ROM[bank * 0x4000 + (programCounter - 0x4000)];
    } else {
        // Probably an error, but mmu_read knows how to handle it better. 
        return mmu_read(s, programCounter);
    }
}

u8 mmu_read(GbState* s, u16 location) {
    // Jump straight to IO registers.
    if (location >= 0xFF00 && location < 0xFF80) {
        u8 lowcation = location & 0x00FF;
        return mmuReadTable[lowcation](s, lowcation);
    }

    u8 highcation = location >> 8;
    if (s->in_bios && highcation < 0x01) {
        return s->mem_BIOS[location];
    }

    if (highcation < 0x80) {
        if (highcation < 0x40) {
            return s->mem_ROM[location];
        } else {
            //MMU_DEBUG_R("ROM B%d, %4x", s->mem_bank_rom, s->mem_bank_rom * 0x4000 + (location - 0x4000));
            u8 bank = s->mem_bank_rom;
            if (s->mbc == 1 && s->mem_mbc1_romram_select == 0)
                bank |= s->mem_mbc1_rombankupper << 5;
            //mmu_assert(s->mem_num_banks_rom > 0);
            //mmu_assert(bank > 0 || s->mbc == 5);
            bank &= s->mem_num_banks_rom - 1;
            return s->mem_ROM[bank * 0x4000 + (location - 0x4000)];
        }
    } else {
        if (highcation < 0xC0) {
            if (highcation < 0xA0) {
                //MMU_DEBUG_R("VRAM");
                return s->mem_VRAM[s->GbcVramBank * VRAM_BANKSIZE + location - 0x8000];
            } else {
                if (s->mbc == 1) {
                    //MMU_DEBUG_R("EXTRAM (rom/ram: %d, B%d)", s->mem_mbc1_romram_select, s->mem_mbc1_extrambank);
                    if (!s->has_extram)
                        return 0xff;
                    if (s->mem_mbc1_romram_select == 1) { // RAM mode
                        //mmu_assert(s->mem_mbc1_extrambank < s->mem_num_banks_extram);
                        return s->mem_EXTRAM[s->mem_mbc1_extrambank * EXTRAM_BANKSIZE + location - 0xa000];
                    } else // ROM mode - we can only be bank 0
                        return s->mem_EXTRAM[location - 0xa000];
                } else if (s->mbc == 3) {
                    //MMU_DEBUG_R("EXTRAM (sw)/RTC (B%d)", s->mem_mbc3_extram_rtc_select);
                    if (s->mem_mbc3_extram_rtc_select < 0x04)
                        return s->mem_EXTRAM[s->mem_mbc3_extram_rtc_select * EXTRAM_BANKSIZE + location - 0xa000];
                    else if (s->mem_mbc3_extram_rtc_select >= 0x08 && s->mem_mbc3_extram_rtc_select <= 0x0c)
                        return s->mem_RTC[s->mem_mbc3_extram_rtc_select];
                    else
                        mmu_error("Reading from extram/rtc with invalid selection (%d) @%x", s->mem_mbc3_extram_rtc_select, location);
                } else if (s->mbc == 5) {
                    //MMU_DEBUG_R("EXTRAM B%d", s->mem_mbc5_extrambank);
                    if (!s->has_extram)
                        return 0xff;
                    //mmu_assert(s->mem_mbc5_extrambank < s->mem_num_banks_extram);
                    return s->mem_EXTRAM[s->mem_mbc5_extrambank * EXTRAM_BANKSIZE + location - 0xa000];
                } else
                    mmu_error("Area not implemented for this MBC (mbc=%d, location=%.4x)\n", s->mbc, location);

                return 0;
            }
        } else {
            if (highcation < 0xE0) {
                if (highcation < 0xD0) {
                    //MMU_DEBUG_R("WRAM B0  @%x", (location - 0xc000));
                    return s->mem_WRAM[location - 0xc000];
                } else {
                    //MMU_DEBUG_R("WRAM B%d @%x", s->GbcRamBankSelectRegister, location - 0xd000);
                    return s->mem_WRAM[s->GbcRamBankSelectRegister * WRAM_BANKSIZE + location - 0xd000];
                }
            } else {
                if (highcation < 0xF0) {
                   return mmu_read(s, location - 0x2000); // TODO XXX 
                    //mmu_error("Reading from ECHO (0xc000 - 0xddff) B0: %x", location);
                } else {
                    if (location < 0xfea0) {
                        if (highcation < 0xfe) {
                            return 0;
                        } else {
                            // FE00 - FE9F
                            //MMU_DEBUG_R("Sprite attribute table (OAM)");
                            return s->mem_OAM[location - 0xfe00];
                        }
                    } else {
                        if (location < 0xff80) {
                            if (highcation < 0xff) {
                                // FEA0 - FEFF
                                return 0;
                            } else {
                                // handled elsewhere
                                return 0;
                            }
                        } else {
                            if (location < 0xffff) {
                                // FF80 - FFFE
                                //MMU_DEBUG_R("HRAM  @%x (%x)", location - 0xff80, s->HRam[location - 0xff80]);
                                return s->mem_HRAM[location - 0xff80];
                            } else {
                                // FFFF
                                //MMU_DEBUG_R("Interrupt enable");
                                return s->InterruptSwitch;
                            }
                        }
                    }
                }
            }
        }
    }
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