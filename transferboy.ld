/* ========================================================================
 * Custom linker script for TransferBoy
 * Adapted from  "n64ld.x" as described below
 *
 * GNU Linker script for building an image that is set up for the N64
 * but still has the data factored into sections.  It is not directly
 * runnable, and it contains the debug info if available.  It will need
 * a 'loader' to perform the final stage of transformation to produce
 * a raw image.
 *
 * Copyright (c) 1999 Ground Zero Development, All rights reserved.
 * Developed by Frank Somers <frank@g0dev.com>
 * Modifications by hcs (halleyscometsoftware@hotmail.com)
 *
 * $Header: /cvsroot/n64dev/n64dev/lib/alt-libn64/n64ld.x,v 1.2 2006/08/11 15:54:11 halleyscometsw Exp $
 *
 * ========================================================================
 */

/*
 This file must be run through the cpp pre-compiler before use.
 Like so:
     cpp $(PRE_LD_FILE) | grep -v '^#'	>>$(LD_FILE)
*/

#include "global.h"

OUTPUT_FORMAT ("elf32-bigmips", "elf32-bigmips", "elf32-littlemips")
OUTPUT_ARCH (mips)
EXTERN (_start)
ENTRY (_start)

SECTIONS {
   /**
    * Start address of code is 1K up in uncached, unmapped RAM.  We have
    * to be at least this far up in order to not interfere with the cart
    * boot code which is copying it down from the cart
    */

    . = 0x80000400 ;

   /**
    * The text section carries the app code and its relocation addr is
    * the first byte of the cart domain in cached, unmapped memory
    */
    .text : {
        FILL (0)

        *(.boot)
        . = ALIGN(16);
        __text_start = . ;
        *(.text)
        *(.text.*)
        *(.ctors)
        *(.dtors)
        *(.rodata)
        *(.rodata.*)
        *(.init)
        *(.fini)
        __text_end  = . ;
    }

   /**
    * Data section has relocation address at start of RAM in cached,
    * unmapped memory, but is loaded just at the end of the text segment,
    * and must be copied to the correct location at startup
    */
    .data : {
        /**
         * Gather all initialised data together.  The memory layout
         * will place the global initialised data at the lowest addrs.
         * The lit8, lit4, sdata and sbss sections have to be placed
         * together in that order from low to high addrs with the _gp symbol
         * positioned (aligned) at the start of the sdata section.
         * We then finish off with the standard bss section
         */

        FILL (0xaa)

        . = ALIGN(16);
        __data_start = . ;
        *(.data)
        *(.irsp)
        *(.lit8)
        *(.lit4) ;
        . = ALIGN(16);
         _gp = . ;
        *(.sdata)
        . = ALIGN(4);
        __data_end = . ;
    }

    /**
     * These scripts must all be aligned to 8 bytes as the DMA channel to DMEM/IMEM can't address below 8 bytes.
     */
    .rspScripts : {
        . = ALIGN(8);
        *(.ppuGBC_data);
        . = ALIGN(8);
        *(.ppuGBC_text);
        . = ALIGN(8);
        *(.ppuSGB_data);
        . = ALIGN(8);
        *(.ppuSGB_text);
        . = ALIGN(8);
        *(.ppuDMG_data);
        . = ALIGN(8);
        *(.ppuDMG_text)
    }

    . = ALIGN(8);

    .ctors : {
        __CTOR_LIST_SIZE__ = .;
        LONG((__CTOR_END__ - __CTOR_LIST__) / 4 - 1)
        __CTOR_LIST__ = .;
        *(.ctors)
        LONG(0)
        __CTOR_END__ = .;
    }

    .bss (NOLOAD) : {
        __bss_start = . ;
        *(.scommon)
        *(.sbss)
        *(COMMON)
        *(.bss)
        . = ALIGN(4);
        __bss_end = . ;
        end = . ;
    }

    /* Places the structure that is shared between the CPU and RSP at a known address so it can be DMAd */
    .rspInterface RSP_INTERFACE_ADDRESS : {
        *(.rspInterface)
    }
}