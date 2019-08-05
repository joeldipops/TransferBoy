#ifndef MMU_H
#define MMU_H

#include "types.h"

/*
 * Memory layout of the GameBoy:
 *  $0000-$3FFF ROM (bank 0)
 *  $4000-$7FFF ROM (bank 1..n, switchable)
 *  $8000-$9FFF VRAM (bank 0 non-CGB, bank 0-1 on CGB)
 *  $A000-$BFFF External RAM (cartridge, optional)
 *  $C000-$CFFF Internal WRAM (bank 0)
 *  $D000-$DFFF Internal WRAM (bank 1-7, switchable, CGB only)
 *  $E000-$FDFF Echo RAM (reserved)
 *  $FE00-$FE9F OAM - Object Attribute Memory
 *  $FEA0-$FEFF Unusable
 *  $FF00-$FF7F Hardware I/O Registers
 *  $FF80-$FFFE Zero
 *  $FFFF       Interrupt Enable Flag
 */

void mmu_step(GbState *s);

/**
 * Reads from an address in the virtual gameboy memory.
 * @param s Gameboy state.
 * @param location address to read from.
 * @returns value at that address.
 */
u8 mmu_read(GbState *s, u16 location);

/**
 * Writes to an address in the virtual gameboy memory.
 * @param s Gameboy state.
 * @param location address to write to.
 * @param value Value to write.
 */
void mmu_write(GbState *s, u16 location, u8 value);

u16 mmu_read16(GbState *s, u16 location);
void mmu_write16(GbState *s, u16 location, u16 value);
u16 mmu_pop16(GbState *s);
void mmu_push16(GbState *s, u16 value);

/**
 * Installs different mmu behaviour for different Memory Bank Controller types.
 * @param s Gameboy state.
 */
void mmu_install_mbc(GbState* s);


#endif
