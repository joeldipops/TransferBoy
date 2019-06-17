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
    //mmu_assert((s->io_hdma_status & (1<<7)) == 0);
    //mmu_assert((s->io_lcd_STAT & 3) == 0);

    for (int i = 0; i < 0x10; i++) {
        u8 dat = mmu_read(s, s->io_hdma_next_src++);
        mmu_write(s, s->io_hdma_next_dst++, dat);
    }

    u32 clks = GB_HDMA_BLOCK_CLKS;
    if (s->double_speed)
        clks *= 2;
    s->emu_state->last_op_cycles += clks;

    s->io_hdma_status--;
    if (s->io_hdma_status == 0xff) {
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
            s->mem_bank_rom, PC,  src, dst, blocks, mode_hblank);

    if (s->io_hdma_running && !mode_hblank) {
        /* Cancel ongoing H-Blank HDMA transfer */
        s->io_hdma_running = 0;
        s->io_hdma_status = 0xff; /* done */
        return;
    }

    //mmu_assert(blocks > 0 && blocks <= 0x80);
    //mmu_assert(src + len <= 0x8000 || /* ROM */ (src >= 0xa000 && src + len <= 0xe000)); /* EXT_RAM */
    //mmu_assert(dst >= 0x8000 && dst + len <= 0xa000); /* VRAM */

    if (!mode_hblank) {
        for (u16 i = 0; i < len; i++)
            mmu_write(s, dst++, mmu_read(s, src++));

        s->io_hdma_status = 0xff; /* done */
        u32 clks = blocks * GB_HDMA_BLOCK_CLKS;
        if (s->double_speed)
            clks *= 2;
        s->emu_state->last_op_cycles += clks;
    } else {
        s->io_hdma_running = 1;
        s->io_hdma_next_src = src;
        s->io_hdma_next_dst = dst;
        s->io_hdma_status = blocks - 1;

        if ((s->io_lcd_STAT & 3) == 0) /* H-Blank */
            mmu_hdma_do(s);
    }
}

void mmu_step(GbState *s) {
    if (s->emu_state->lcd_entered_hblank && s->io_hdma_running)
        mmu_hdma_do(s);
}

static inline u8 mmu_register_read(GbState* s, u16 location) {
    u8 lowcation = location & 0x00FF;
    
    if (lowcation < 0x80) {
        if (lowcation < 0x40) {
            if (lowcation < 0x20) {
                if (lowcation < 0x10) {
                    if (lowcation < 0x08) {
                        if (lowcation < 0x04) {
                            if (lowcation < 0x02) {
                                if (lowcation < 0x01) {
                                    // FF00: Joypad
                                    u8 rv = 0;
                                    if ((s->io_buttons & (1 << 4)) == 0)
                                        rv = (s->io_buttons & 0xf0) | (s->io_buttons_dirs & 0x0f);
                                    else if ((s->io_buttons & (1 << 5)) == 0)
                                        rv =  (s->io_buttons & 0xf0) | (s->io_buttons_buttons & 0x0f);
                                    else
                                        rv = (s->io_buttons & 0xf0) | (s->io_buttons_buttons & 0x0f);
                                    return rv;
                                } else {
                                    // FF01: Serial Link Data
                                    return s->io_serial_data;
                                }
                            } else {
                                if (lowcation < 0x03) {
                                    // FF02: Serial Control  
                                    return s->io_serial_control; 
                                } else {
                                    ; // FF03: none
                                }
                            }
                        } else {
                            if (lowcation < 0x06) {
                                if (lowcation < 0x05) {
                                    // FF04: Timer Divider
                                    return s->io_timer_DIV;
                                } else {
                                    // FF05: Timer
                                    return s->io_timer_TIMA;
                                }
                            } else {
                                if (lowcation < 0x07) {
                                    // FF06: Timer Modulo
                                    return s->io_timer_TMA;
                                } else {
                                    // FF07: Timer Control
                                    return s->io_timer_TAC;
                                }
                            }
                        }
                    } else {
                        if (lowcation == 0x0f) {
                            // FF0F: Interrupt request
                            return s->interrupts_request;
                        } else {
                            ;
                        }
                    }
                } else {
                    if (lowcation < 0x18) {
                        if (lowcation < 0x14) {
                            if (lowcation < 0x12) {
                                if (lowcation < 0x11) {
                                    // FF10: channel 1 sweep
                                    return s->io_sound_channel1_sweep;
                                } else {
                                    // FF11: channel 1 length/pattern
                                    return s->io_sound_channel1_length_pattern;
                                }
                            } else {
                                if (lowcation < 0x13) {
                                    // FF12: channel 1 envelope
                                    return s->io_sound_channel1_envelope;
                                } else {
                                    // FF13: channel frequence
                                    return s->io_sound_channel1_freq_lo;
                                }
                            }
                        } else {
                            if (lowcation < 0x16) {
                                if (lowcation < 0x15) {
                                    // FF14: channel 1 frequency high bits and control
                                    return s->io_sound_channel1_freq_hi;
                                } else {
                                    // FF15: unused channel 2 sweep 
                                }
                            } else {
                                if (lowcation < 0x17) {
                                    // FF16: channel 2 length and pattern
                                    return s->io_sound_channel2_length_pattern;
                                } else {
                                    // FF17: channel 2 envelope
                                    return s->io_sound_channel2_envelope;
                                }
                            }
                        }
                    } else {
                        if (lowcation < 0x1C) {
                            if (lowcation < 0x1A) {
                                if (lowcation < 0x19) {
                                    // FF18: channel 2 frequency
                                    return s->io_sound_channel2_freq_lo;
                                } else {
                                    // FF19: channel 2 high frequench and control
                                    return s->io_sound_channel2_freq_hi;
                                }
                            } else {
                                if (lowcation < 0x1B) {
                                    // FF1A: channel 3 enable flag
                                    return s->io_sound_channel3_enabled;
                                } else {
                                    // FF1B: channel 3 length
                                    return s->io_sound_channel3_length;
                                }
                            }
                        } else {
                            if (lowcation < 0x1E) {
                                if (lowcation < 0x1D) {
                                    // FF1C: channel 3 level
                                    return s->io_sound_channel3_level;
                                } else {
                                    // FF1D: channel 3 frequency
                                    return s->io_sound_channel3_freq_lo;
                                }
                            } else {
                                if (lowcation < 0x1F) {
                                    // FF1E channel 3 frequench high and control
                                    return s->io_sound_channel3_freq_hi;
                                } else {
                                    // FF1F unused channel 4 sweep
                                    ;
                                }
                            }
                        }
                    }
                }
            } else {
                if (lowcation < 0x30) {
                    if (lowcation < 0x28) {
                        if (lowcation < 0x24) {
                            if (lowcation < 0x22) {
                                if (lowcation < 0x21) {
                                    // FF20: channel 4 length
                                    return s->io_sound_channel4_length;
                                } else {
                                    // FF21: channel 4 envelope
                                    return s->io_sound_channel4_envelope;
                                }
                            } else {
                                if (lowcation < 0x23) {
                                    // FF22: channel 4 waveform
                                    return s->io_sound_channel4_poly;
                                } else {
                                    // FF23: channel 4 flags
                                    return s->io_sound_channel4_consec_initial;
                                }
                            }
                        } else {
                            if (lowcation < 0x26) {
                                if (lowcation < 0x25) {
                                    // FF24: sound channel control
                                    return s->io_sound_terminal_control;
                                } else {
                                    // FF25: sound speaker control
                                    return s->io_sound_out_terminal;
                                }
                            } else {
                                if (lowcation < 0x27) {
                                    // FF26: Sound master switch
                                    return s->io_sound_enabled;
                                } else {
                                    ; //unused
                                }
                            }
                        }
                    } else {
                        // FF27 - FF2F: unused 
                        ;
                    }
                } else {
                    // FF30 - FF3f: channel 4 wave data
                    return s->io_sound_channel3_ram[location-0xff30];
                }
            }
        } else {
            if (lowcation < 0x60) {
                if (lowcation < 0x50) {
                    if (lowcation < 0x48) {
                        if (lowcation < 0x44) {
                            if (lowcation < 0x42) {
                                if (lowcation < 0x41) {
                                    // FF40: LCD Control
                                    return s->io_lcd_LCDC;
                                } else {
                                    // FF41: LCD Status
                                    return s->io_lcd_STAT;
                                }
                            } else {
                                if (lowcation < 0x43) {
                                    // FF42: Background scroll Y
                                    return s->io_lcd_SCY;
                                } else {
                                    // FF43: Background scroll X
                                    return s->io_lcd_SCX; 
                                }
                            }
                        } else {
                            if (lowcation < 0x46) {
                                if (lowcation < 0x45) {
                                    // FF44: LCD Line Y?
                                    return s->io_lcd_LY;
                                } else {
                                    // FF45: LCD LYC - no idea what that means
                                    return s->io_lcd_LYC;
                                }
                            } else {
                                if (lowcation < 0x47) {
                                    // FF46: Transfers memory to OAM for rendering
                                    ;                                    
                                } else {
                                    // FF47: Background Palette'
                                    return s->io_lcd_BGP;
                                }
                            }
                        }
                    } else {
                        if (lowcation < 0x4C) {
                            if (lowcation < 0x4A) {
                                if (lowcation < 0x49) {
                                    // FF48: Sprite Palette 0
                                    return s->io_lcd_OBP0;
                                } else {
                                    //FF49: Sprite palette 2
                                    return s->io_lcd_OBP1;
                                }
                            } else {
                                if (lowcation < 0x4B) {
                                    // FF4A: Window Y position
                                    return s->io_lcd_WY;
                                } else {
                                    // FF4B: WIndow X position
                                    return s->io_lcd_WX;
                                }
                            }
                        } else {
                            if (lowcation < 0x4E) {
                                // FF4C: unused
                                // FF4D: unused - "Key1: CGB speed"???
                            } else {
                                // FF4E: unused
                                if (lowcation == 0x4F) {
                                    // FF4F: VRAM bank flag
                                    return s->mem_bank_vram & 1;
                                }
                            }
                        }
                    }
                } else {
                    if (lowcation < 0x58) {
                        if (lowcation < 0x54) {
                            if (lowcation < 0x52) {
                                if (lowcation < 0x51) {
                                } else {
                                    // FF51: hdma source high byte
                                }
                            } else {
                                if (lowcation < 0x53) {
                                    // FF52: hdma source low byte
                                } else {
                                    // FF53: hdma destination high byte
                                }
                            }
                        } else {
                            if (lowcation < 0x56) {
                                if (lowcation < 0x55) {
                                    // FF54: hdma destination low byte
                                } else {
                                    // FF55: hdma length & control
                                    return s->io_hdma_status;
                                }
                            } else {
                                if (lowcation < 0x57) {
                                    // FF56: GBC Infrared transfer
                                    return s->io_infrared;
                                }
                            }
                        }
                    }
                } 
            } else {
                if (lowcation < 0x70) { 
                    if (lowcation < 0x68) {
                        ; // unused
                    } else {
                        if (lowcation < 0x6C) {
                            if (lowcation < 0x6A) {
                                if (lowcation < 0x69) {
                                    // FF68: Background palette index
                                } else {
                                    // FF69: Background Palette data
                                    return s->io_lcd_BGPD[s->io_lcd_BGPI & 0x3f];
                                }
                            } else {
                                if (lowcation < 0x6B) {
                                } else {
                                    //FF6B: Sprite palette data
                                    return s->io_lcd_OBPD[s->io_lcd_OBPI & 0x3f];
                                }
                            }
                        }
                    }
                } else {
                    if (lowcation == 0x70) {
                        // FF70: Change RAM bank
                        return s->mem_bank_wram; 
                    }
                }   
            }
        }
    }  
    return 0x00;  
}

/**
 * Writes to one of the IO registers
 * @param s gameboy state
 * @param location to write to
 * @param value to write.
 */
static inline void mmu_register_write(GbState* s, u16 location, u8 value) {
    // assuming 8 bit compares are faster than 16bit?
    u8 lowcation = location & 0x00FF;
    
    if (lowcation < 0x80) {
        if (lowcation < 0x40) {
            if (lowcation < 0x20) {
                if (lowcation < 0x10) {
                    if (lowcation < 0x08) {
                        if (lowcation < 0x04) {
                            if (lowcation < 0x02) {
                                if (lowcation < 0x01) {
                                    // FF00: Joypad
                                    s->io_buttons = value;
                                } else {
                                    // FF01: Serial Link Data
                                    s->io_serial_data = value;
                                }
                            } else {
                                if (lowcation < 0x03) {
                                    // FF02: Serial Control  
                                    s->io_serial_control = value; 
                                } else {
                                    ; // FF03: none
                                }
                            }
                        } else {
                            if (lowcation < 0x06) {
                                if (lowcation < 0x05) {
                                    // FF04: Timer Divider
                                    s->io_timer_DIV = 0x00;
                                } else {
                                    // FF05: Timer
                                    s->io_timer_TIMA = value;
                                }
                            } else {
                                if (lowcation < 0x07) {
                                    // FF06: Timer Modulo
                                    s->io_timer_TMA = value;
                                } else {
                                    // FF07: Timer Control
                                    s->io_timer_TAC = value;
                                }
                            }
                        }
                    } else {
                        if (lowcation == 0x0f) {
                            // FF0F: Interrupt request
                            s->interrupts_request = value;
                        } else {
                            ;
                        }
                    }
                } else {
                    if (lowcation < 0x18) {
                        if (lowcation < 0x14) {
                            if (lowcation < 0x12) {
                                if (lowcation < 0x11) {
                                    // FF10: channel 1 sweep
                                    s->io_sound_channel1_sweep = value;
                                } else {
                                    // FF11: channel 1 length/pattern
                                    s->io_sound_channel1_length_pattern = value;
                                }
                            } else {
                                if (lowcation < 0x13) {
                                    // FF12: channel 1 envelope
                                    s->io_sound_channel1_envelope = value;
                                } else {
                                    // FF13: channel frequence
                                    s->io_sound_channel1_freq_lo = value;
                                }
                            }
                        } else {
                            if (lowcation < 0x16) {
                                if (lowcation < 0x15) {
                                    // FF14: channel 1 frequency high bits and control
                                    s->io_sound_channel1_freq_hi = value;
                                } else {
                                    // FF15: unused channel 2 sweep 
                                }
                            } else {
                                if (lowcation < 0x17) {
                                    // FF16: channel 2 length and pattern
                                    s->io_sound_channel2_length_pattern = value;
                                } else {
                                    // FF17: channel 2 envelope
                                    s->io_sound_channel2_envelope = value;
                                }
                            }
                        }
                    } else {
                        if (lowcation < 0x1C) {
                            if (lowcation < 0x1A) {
                                if (lowcation < 0x19) {
                                    // FF18: channel 2 frequency
                                    s->io_sound_channel2_freq_lo = value;
                                } else {
                                    // FF19: channel 2 high frequench and control
                                    s->io_sound_channel2_freq_hi = value;
                                }
                            } else {
                                if (lowcation < 0x1B) {
                                    // FF1A: channel 3 enable flag
                                    s->io_sound_channel3_enabled = value;
                                } else {
                                    // FF1B: channel 3 length
                                    s->io_sound_channel3_length = value;
                                }
                            }
                        } else {
                            if (lowcation < 0x1E) {
                                if (lowcation < 0x1D) {
                                    // FF1C: channel 3 level
                                    s->io_sound_channel3_level = value;
                                } else {
                                    // FF1D: channel 3 frequency
                                    s->io_sound_channel3_freq_lo = value;
                                }
                            } else {
                                if (lowcation < 0x1F) {
                                    // FF1E channel 3 frequench high and control
                                    s->io_sound_channel3_freq_hi = value;
                                } else {
                                    // FF1F unused channel 4 sweep
                                    ;
                                }
                            }
                        }
                    }
                }
            } else {
                if (lowcation < 0x30) {
                    if (lowcation < 0x28) {
                        if (lowcation < 0x24) {
                            if (lowcation < 0x22) {
                                if (lowcation < 0x21) {
                                    // FF20: channel 4 length
                                    s->io_sound_channel4_length = value;
                                } else {
                                    // FF21: channel 4 envelope
                                    s->io_sound_channel4_envelope = value;
                                }
                            } else {
                                if (lowcation < 0x23) {
                                    // FF22: channel 4 waveform
                                    s->io_sound_channel4_poly = value;
                                } else {
                                    // FF23: channel 4 flags
                                    s->io_sound_channel4_consec_initial = value;
                                }
                            }
                        } else {
                            if (lowcation < 0x26) {
                                if (lowcation < 0x25) {
                                    // FF24: sound channel control
                                    s->io_sound_terminal_control = value;
                                } else {
                                    // FF25: sound speaker control
                                    s->io_sound_out_terminal = value;
                                }
                            } else {
                                if (lowcation < 0x27) {
                                    // FF26: Sound master switch
                                    s->io_sound_enabled = (value & 0x80) | (s->io_sound_enabled & 0x7f);
                                } else {
                                    ; //unused
                                }
                            }
                        }
                    } else {
                        // FF27 - FF2F: unused 
                        ;
                    }
                } else {
                    // FF30 - FF3f: channel 4 wave data
                    s->io_sound_channel3_ram[lowcation-0x30] = value;
                }
            }
        } else {
            if (lowcation < 0x60) {
                if (lowcation < 0x50) {
                    if (lowcation < 0x48) {
                        if (lowcation < 0x44) {
                            if (lowcation < 0x42) {
                                if (lowcation < 0x41) {
                                    // FF40: LCD Control
                                    s->io_lcd_LCDC = value;
                                } else {
                                    // FF41: LCD Status
                                    s->io_lcd_STAT = (value & ~7) | (s->io_lcd_STAT & 7);
                                }
                            } else {
                                if (lowcation < 0x43) {
                                    // FF42: Background scroll Y
                                    s->io_lcd_SCY = value;
                                } else {
                                    // FF43: Background scroll X
                                    s->io_lcd_SCX = value; 
                                }
                            }
                        } else {
                            if (lowcation < 0x46) {
                                if (lowcation < 0x45) {
                                    // FF44: LCD Line Y?
                                    s->io_lcd_LY = 0x0;
                                    s->io_lcd_STAT = (s->io_lcd_STAT & 0xfb) | ((s->io_lcd_LY == s->io_lcd_LYC) << 2);
                                } else {
                                    // FF45: LCD LYC - no idea what that means
                                    s->io_lcd_LYC = value;
                                    s->io_lcd_STAT = (s->io_lcd_STAT & 0xfb) | ((s->io_lcd_LY == s->io_lcd_LYC) << 2);
                                }
                            } else {
                                if (lowcation < 0x47) {
                                    // FF46: Transfers memory to OAM for rendering
                                    
                                    // Normally this transfer takes ~160ms (during which only HRAM
                                    // is accessible) but it's okay to be instantaneous. Normally
                                    // roms loop for ~200 cycles or so to wait.  
                                    for (unsigned i = 0; i < OAM_SIZE; i++)
                                        s->mem_OAM[i] = mmu_read(s, (value << 8) + i);
                                } else {
                                    // FF47: Background Palette'
                                    s->io_lcd_BGP = value;
                                }
                            }
                        }
                    } else {
                        if (lowcation < 0x4C) {
                            if (lowcation < 0x4A) {
                                if (lowcation < 0x49) {
                                    // FF48: Sprite Palette 0
                                    s->io_lcd_OBP0 = value;
                                } else {
                                    //FF49: Sprite palette 2
                                    s->io_lcd_OBP1 = value;
                                }
                            } else {
                                if (lowcation < 0x4B) {
                                    // FF4A: Window Y position
                                    s->io_lcd_WY = value;
                                } else {
                                    // FF4B: WIndow X position
                                    s->io_lcd_WX = value;
                                }
                            }
                        } else {
                            if (lowcation < 0x4E) {
                                // FF4C: unused
                                // FF4D: unused - "Key1: CGB speed"???
                            } else {
                                // FF4E: unused
                                if (lowcation == 0x4F) {
                                    // FF4F: VRAM bank flag
                                    s->mem_bank_vram = value & 1;
                                }
                            }
                        }
                    }
                } else {
                    if (lowcation < 0x58) {
                        if (lowcation < 0x54) {
                            if (lowcation < 0x52) {
                                if (lowcation < 0x51) {
                                    // FF50: bios enabled flag
                                    s->in_bios = 0; 
                                } else {
                                    // FF51: hdma source high byte
                                    s->io_hdma_src_high = value;
                                }
                            } else {
                                if (lowcation < 0x53) {
                                    // FF52: hdma source low byte
                                    s->io_hdma_src_low = value;
                                } else {
                                    // FF53: hdma destination high byte
                                    s->io_hdma_dst_high = value;
                                }
                            }
                        } else {
                            if (lowcation < 0x56) {
                                if (lowcation < 0x55) {
                                    // FF54: hdma destination low byte
                                    s->io_hdma_dst_low = value;
                                } else {
                                    // FF55: hdma length & control
                                    mmu_hdma_start(s, value);
                                }
                            } else {
                                if (lowcation < 0x57) {
                                    // FF56: GBC Infrared transfer
                                    s->io_infrared = value;
                                }
                            }
                        }
                    }
                } 
            } else {
                if (lowcation < 0x70) { 
                    if (lowcation < 0x68) {
                        ; // unused
                    } else {
                        if (lowcation < 0x6C) {
                            if (lowcation < 0x6A) {
                                if (lowcation < 0x69) {
                                    // FF68: Background palette index
                                    s->io_lcd_BGPI = value;
                                } else {
                                    // FF69: Background Palette data
                                    s->io_lcd_BGPD[s->io_lcd_BGPI & 0x3f] = value;
                                    if (s->io_lcd_BGPI & (1 << 7))
                                        s->io_lcd_BGPI = (((s->io_lcd_BGPI & 0x3f) + 1) & 0x3f) | (1 << 7);
                                }
                            } else {
                                if (lowcation < 0x6B) {
                                    // FF6A: Sprite palette index
                                    s->io_lcd_OBPI = value;
                                } else {
                                    //FF6B: Sprite palette data
                                    s->io_lcd_OBPD[s->io_lcd_OBPI & 0x3f] = value;
                                    if (s->io_lcd_OBPI & (1 << 7))
                                        s->io_lcd_OBPI = (((s->io_lcd_OBPI & 0x3f) + 1) & 0x3f) | (1 << 7);
                                }
                            }
                        }
                    }
                } else {
                    if (lowcation == 0x70) {
                        // FF70: Change RAM bank
                        if (value == 0)
                            value = 1;
                        value &= s->mem_num_banks_wram - 1;
                        s->mem_bank_wram = value;    
                    }
                }   
            }
        }
    }
}

void mmu_write(GbState *s, u16 location, u8 value) {
    // Jump straight to IO registers.
    if (location >= 0xFF00 && location < 0xFF80) {
        return mmu_register_write(s, location, value);
    }

    u8 highcation = location >> 8;
     if (highcation < 0x80) {
        if (highcation < 0x40) {
            if (highcation < 0x20) {
                // 0000 - 1FFF: Fixed ROM bank
                // Dummy, we always have those enabled.
                // Turning off the RAM could indicate that battery-backed data is done
                // being written and could be flushed to disk.
                if (value == 0)
                    s->emu_state->flush_extram = 1;          
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
                    s       ->mem_mbc1_romram_select = value & 0x1;
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
                s->mem_VRAM[s->mem_bank_vram * VRAM_BANKSIZE + location - 0x8000] = value;
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
                    s->mem_WRAM[s->mem_bank_wram * WRAM_BANKSIZE + location - 0xd000] = value; 
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
                        if (location < 0xFF80) {
                            //FF00 - FF7F: I/O Registers
                            mmu_register_write(s, location, value);
                        } else {
                            if (location < 0xFFFF) {
                                // 0xFF80 - FFFE: Stack RAM
                                s->mem_HRAM[location - 0xff80] = value;    
                            } else {
                                s->interrupts_enable = value;
                            }
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
        return mmu_register_read(s, location);
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
                return s->mem_VRAM[s->mem_bank_vram * VRAM_BANKSIZE + location - 0x8000];
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
                    //MMU_DEBUG_R("WRAM B%d @%x", s->mem_bank_wram, location - 0xd000);
                    return s->mem_WRAM[s->mem_bank_wram * WRAM_BANKSIZE + location - 0xd000];
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
                                return mmu_register_read(s, location);
                            }
                        } else {
                            if (location < 0xffff) {
                                // FF80 - FFFE
                                //MMU_DEBUG_R("HRAM  @%x (%x)", location - 0xff80, s->mem_HRAM[location - 0xff80]);
                                return s->mem_HRAM[location - 0xff80];
                            } else {
                                // FFFF
                                //MMU_DEBUG_R("Interrupt enable");
                                return s->interrupts_enable;
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