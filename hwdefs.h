#ifndef HWDEFS_H
#define HWDEFS_H

#include "global.h"

static const int GB_LCD_LY_MAX = 153;

static const int GB_HDMA_BLOCK_CLKS = 8;    /* Per block of 0x10 bytes */

static const int GB_LCD_MODE_0_CLKS = 204;  /* H-Blank */
static const int GB_LCD_MODE_1_CLKS = 4560; /* V-Blank */
static const int GB_LCD_MODE_2_CLKS = 80;   /* OAM read */
static const int GB_LCD_MODE_3_CLKS = 172;  /* Line rendering */
static const int GB_LCD_FRAME_CLKS  = 70224; /* Total cycles per frame */

static const int GB_DIV_FREQ = 16384;  /* Hz */
static const int GB_TIMA_FREQS[] = { 4096, 262144, 65536, 16384 };  /* Hz */

static const double GB_SND_DUTY_PERC[] = { .125, .25, .50, .75 };
static const int GB_SND_ENVSTEP_CYC = GB_FREQ/64; /* n*(1/64)th seconds */

static const unsigned ROMHDR_TITLE      = 0x134;
static const unsigned ROMHDR_CGBFLAG    = 0x143;
static const unsigned ROMHDR_CARTTYPE   = 0x147;
static const unsigned ROMHDR_ROMSIZE    = 0x148;
static const unsigned ROMHDR_EXTRAMSIZE = 0x149;

static const unsigned ROM_BANK_SIZE      = 0x4000; /* 16K */
static const unsigned WRAM_BANK_SIZE     = 0x1000; /* 4K */
static const unsigned VRAM_BANK_SIZE     = 0x2000; /* 8K */
static const unsigned SRAM_BANK_SIZE    = 0x2000; /* 8K */

static const unsigned OAM_SIZE          = 0xa0;

#endif
