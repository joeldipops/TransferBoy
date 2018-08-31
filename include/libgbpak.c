/*
 * libgbpak v0.7 beta for libdragon
 * by saturnu
 *
 * TODO: testing mbc1 above bank 31 and mbc2-mbc5
 * TODO: reset high bits mbc1 mbc5 if not reading whole rom in serial
 * TODO: rtc/rumble support
 * mapper info: http://gbdev.gg8.se/wiki/articles/Memory_Bank_Controllers
 * libdragon: https://github.com/DragonMinded/libdragon
 */

/*
 * Completely bastardised from original code to fit my purpose and formatting standards.
 */

#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <stdint.h>
#include <libdragon.h>
#include <unistd.h>
#include <pthread.h>

#include "logger.h"
#include "constants.h"

#include "libgbpak.h"

uint8_t data[32];

int _set_gbPower(const char controllerNumber, int status) { //setstatus ???
    //1=on 2=off

    if(status) {
        //set on
        status = 0x84;
    } else {
        //set off
        status = 0xFE;
    }

    int value=0;

    memset(data, status, 32);
    value = write_mempak_address(controllerNumber, 0x8001, data);

    return value;
}


//int get_gbPower(uint8_t *rdata){
int _get_gbPower(const char controllerNumber) { //checkstatus???
    uint8_t rdata[32];
    memset(rdata, 0xFF, 32);
    int value = -1;

    value = read_mempak_address(controllerNumber, 0x8001, rdata);

    if(rdata[0] == 0x00) {
        value = 0;
    } else if(rdata[0] == 0x84) {
        value = 1;
    }

    return value;
}


int _get_gbAccessState(const char controllerNumber) {
    //real: get power ????

	uint8_t rdata[32];
	memset(rdata, 0xFF, 32);

	int value=0;

    value = read_mempak_address(controllerNumber, 0xB010, rdata);

	/*
	 * Sets bit 2 of the first return value if the access mode was recently changed.
	 * Will not set bit 2 again untill access mode is changed again.
	 */

	if(rdata[0] == 0x89) { //mode 1
        value = 1;
	} else if(rdata[0]==0x80) { //mode 0 //0x84 bot changed?
        value = 0;
    } else if(rdata[0]==0x84) {
        value = 2;
	} else if(rdata[0]==0x40) { //no gbcart inserted
        value=3;
    } else {
        value=rdata[0];
    }

	return value;
}


int _set_gbAccessState(const char controllerNumber, int status) { //set power ????

	if(status) {
		//mode 01
		status = 0x01;
	} else {
		//mode 00
		status = 0x00;
	}

	int value=0;

    memset(data, status, 32);
    value = write_mempak_address(controllerNumber, 0xB010, data);

	return value;
}


int disable_gbRam(const GameboyCart gbcart) {

	uint8_t sdata[32];
	int value=0;

	if(gbcart.mapper == GB_NORM) {
		return -1; //no ram
	} else {
		//same for all mbc 1-5 :>

		//protect ram again
		memset(sdata, 0x00, 32);
		value = write_mempak_address( 0, 0xA00C, sdata ); //prepare for ram enable
		value = write_mempak_address( 0, 0xC000, sdata ); //set ram diable for reading/writing
	}

    return value;
}


int _set_gbRamBank(const char controllerNumber, const GameboyCart gbcart, int bank) {
	if(gbcart.ram!=TRUE) {
        return -1;
    }

	//e.g. 00
	bank = 0x01 * bank; // :D pointless
	uint8_t sdata[32];
	int value = 0;

	if(gbcart.mapper==GB_NORM) {
		return -1; //no ram
	} else if(
        ((gbcart.mapper==GB_MBC1 || gbcart.mapper==GB_MBC3 || gbcart.mapper==GB_HUC1) && bank <=0x03)
		|| (gbcart.mapper==GB_MBC5 && bank <=0x0F) || (gbcart.mapper==GB_MBC4 && bank <=0x0F)
    ) {
		memset(sdata, 0x00, 32);
		value = write_mempak_address(controllerNumber, 0xA00C, sdata ); //prepare for ram enable

		memset(sdata, 0x0A, 32); // 0x00 disable(rom) 0x0A enable(ram)
		value = write_mempak_address(controllerNumber, 0xC000, sdata ); //set ram enable for reading/writing

		memset(sdata, 0x01, 32);
		value = write_mempak_address(controllerNumber, 0xA00C, sdata ); //prepare for rambank switch

		memset(sdata, 0x01, 32); //00h rombanking 01h rambanking
		value = write_mempak_address(controllerNumber, 0xE000, sdata ); //switch to rambanking

		memset(sdata, bank, 32); //00h ram bank 00-03h or 00-0Fh
		value = write_mempak_address(controllerNumber, 0xC000, sdata ); //set rambank
	} else if( (gbcart.mapper==GB_CAMERA && bank <=0x0F) ) {

		memset(sdata, 0x00, 32);
		value = write_mempak_address(controllerNumber, 0xA00C, sdata ); //prepare for ram enable

		//create second test version with 0x00?
		memset(sdata, 0x0A, 32); // disable only -> for 0A enable setting - set PHI=pin1 high first on hardware, too?
		value = write_mempak_address(controllerNumber, 0xC000, sdata ); //set ram enable for reading/writing

		memset(sdata, 0x01, 32);
		value = write_mempak_address(controllerNumber, 0xA00C, sdata ); //prepare for rambank switch

		memset(sdata, 0x01, 32); //00h rombanking 01h rambanking
		value = write_mempak_address(controllerNumber, 0xE000, sdata ); //switch to rambanking

		memset(sdata, bank, 32); //00h ram bank 00-0Fh, bit 5 0x10 set cam registers
		value = write_mempak_address(controllerNumber, 0xC000, sdata ); //set rambank
	} else if(gbcart.mapper==GB_MBC2) {		//512x4bits RAM, built-in into the MBC2 chip

		//only one bank?
		memset(sdata, 0x00, 32);
		value = write_mempak_address(controllerNumber, 0xA00C, sdata ); //prepare for ram enable

		memset(sdata, 0x0A, 32); // enable/disable with whatever as value in the correct range - was 0x01?
		value = write_mempak_address(controllerNumber, 0xC000, sdata ); //set ram enable for reading/writing
	} else {
		//mapper not found or out of range
		return -1;
	}

    return value;
}


int _set_gbRomBank(const char controllerNumber, GameboyCart* gbcart, int bank) {

	//e.g. 00
	bank = 0x01 * bank; // :D pointless
	uint8_t sdata[32];
	int value = 0;

	if (gbcart->mapper == GB_NORM || bank == 0x00) {

		memset( sdata, bank, 32 );
		value = write_mempak_address( 0, 0xA00C, sdata );
	} else if(gbcart->mapper == GB_MBC1 || gbcart->mapper == GB_HUC1) {
		if (bank==0x20 || bank==0x40 || bank==0x60) //bank 0x20, 0x40, 0x60 isn't addressable
            return -1;

		//NOTE: untested don't own a mbc1 cart with 32 banks or above ;_;
		if (bank>0x20) { //don't need to set upper bits for every bank < 32

			//set to setting mode :>
			memset( sdata, 0x01, 32);
			value = write_mempak_address(controllerNumber, 0xA00C, sdata );
			if (value!=0x00)
                return value;

			//0x00 rombanking mode 0x01 rambanking mode
			//disable ram
			memset( sdata, 0x00, 32 );
			value = write_mempak_address(controllerNumber, 0xE016, sdata ); //0x6000
			if (value!=0x00)
                return value;

			//set upper 2 bits (6,7)
			//0h = +0 int
			//1h = +32 int
			//2h = +64 int
			//3h = +96 int
			if(bank>0x60) {
				memset( sdata, 0x03, 32 );
				bank-=0x60;
			} else if(bank>0x40) {
				memset( sdata, 0x02, 32 );
				bank-=0x40;
			} else {
				memset( sdata, 0x01, 32 );
				bank-=0x20;
            }

			//set bit 6 and 7
			//8765 4321
			//0XX0 0000
			value = write_mempak_address(controllerNumber, 0xC000, sdata ); //write at 0x4000
			if (value != 0x00)
                return value;

			//TODO: disable high bits
		}

		//lower bits
		memset( sdata, 0x00, 32);
		value = write_mempak_address(controllerNumber, 0xA00C, sdata );
		if(value!=0x00)
            return value;

		//set bit 1-5
		//8765 4321
		//000X XXXX
		memset( sdata, bank, 32 );
		value = write_mempak_address(controllerNumber, 0xE100, sdata );
		if(value!=0x00)
            return value;

		memset( sdata, 0x01, 32);
		value = write_mempak_address(controllerNumber, 0xA00C, sdata );
        if(value!=0x00)
            return value;

	} else if(gbcart->mapper==GB_MBC2 && bank <=0x0F) {
        //NOTE: untested don't own a mbc2 cart ;_;

		memset( sdata, 0x00, 32);
		value = write_mempak_address(controllerNumber, 0xA00C, sdata );
		if(value!=0x00)
            return value;

		//max 16 (0x00 - 0x0F) banks here
		memset( sdata, bank, 32 );
		value = write_mempak_address(controllerNumber, 0xE100, sdata );
		if(value!=0x00)
            return value;

		memset( sdata, 0x01, 32);
		value = write_mempak_address(controllerNumber, 0xA00C, sdata );
		if(value!=0x00)
            return value;

	} else if(gbcart->mapper==GB_MBC3 && bank <=0x7F) {
        //NOTE: untested don't own a mbc3 cart ;_;

		memset( sdata, 0x00, 32);
		value = write_mempak_address(controllerNumber, 0xA00C, sdata );
		if(value!=0x00)
            return value;

		//whole 7 bit can be used for addressing here
		//128 (0x00 - 0x7F) banks max
		memset( sdata, bank, 32 );
		value = write_mempak_address(controllerNumber, 0xE100, sdata );
		if(value!=0x00)
            return value;

		memset( sdata, 0x01, 32);
		value = write_mempak_address(controllerNumber, 0xA00C, sdata );
		if(value!=0x00)
            return value;

	} else if(gbcart->mapper==GB_CAMERA && bank <=0x3F) { //camera
        //NOTE: untested don't own mbc5 cart ;_;

		memset( sdata, 0x00, 32);
		value = write_mempak_address(controllerNumber, 0xA00C, sdata );
		if(value!=0x00)
            return value;

		//whole 7 bit can be used for addressing here
		//64 (0x00 - 0x3F) banks max
		memset( sdata, bank, 32 );
		value = write_mempak_address(controllerNumber, 0xE100, sdata );
        if(value!=0x00)
            return value;

		memset( sdata, 0x01, 32);
		value = write_mempak_address(controllerNumber, 0xA00C, sdata );
		if(value!=0x00)
            return value;

	} else if(gbcart->mapper == GB_MBC5 || gbcart->mapper == GB_MBC4) {
        //NOTE: untested don't own mbc5 cart ;_;

		int bank5=bank; //preset < 0x100

		if(bank>0xFF) {

			//set to setting mode :>
			memset( sdata, 0x01, 32);
			value = write_mempak_address(controllerNumber, 0xA00C, sdata );
			if(value!=0x00)
                return value;

			//0x00 rombanking mode 0x01 rambanking mode
			//disable ram
			memset( sdata, 0x00, 32 );
			value = write_mempak_address(controllerNumber, 0xE016, sdata ); //0x6000
			if(value!=0x00)
                return value;

			//set bit 9 enable
			//9 8765 4321
			//X 0000 0000
			memset( sdata, 0x01, 32 );
			value = write_mempak_address(controllerNumber, 0xC000, sdata ); //write at 0x4000
			if(value!=0x00)
                return value;

			bank5=bank-0x100; //set for lower bits
		}


		//lower bits
		memset( sdata, 0x00, 32);
		value = write_mempak_address(controllerNumber, 0xA00C, sdata );
		if(value!=0x00)
            return value;

		//set bit 1-8 even 0x00 if you like
		//XXXX XXXX
		memset( sdata, bank5, 32 );
		value = write_mempak_address(controllerNumber, 0xE100, sdata );
		if(value!=0x00)
            return value;

		memset( sdata, 0x01, 32);
		value = write_mempak_address(controllerNumber, 0xA00C, sdata );
		if(value!=0x00)
            return value;

		//TODO: disable high bit

	}

	gbcart->bank=bank;
	return 0;
}

int _get_gbRamAddr(const char controllerNumber, unsigned long addr, uint8_t *rdata){

	//mbc1,3,5 A000-BFFF
	//mbc2 A000-A1FF
	uint8_t sdata[32];
	//addr e.g. 0xC138
	int value=0;

	if((addr>=0xE000) && (addr<=0xFFFF)) {

        memset(sdata, 0x02, 32);
        value = write_mempak_address(controllerNumber, 0xA00C, sdata );//prepare for rambank read

        memset(rdata, 0x00, 32);
        value = read_mempak_address(controllerNumber, addr, rdata );

	} else {
		value=-1;
    }

	return value;
}


int _set_gbRamAddr(const char controllerNumber, unsigned long addr, uint8_t *sdata) {

	//mbc1,3,5 A000-BFFF
	//mbc2 A000-A1FF
	uint8_t sdata_[32];

	//addr e.g. 0xC138
	int value=0;

	if((addr>=0xE000) && (addr<=0xFFFF)) {

        memset(sdata_, 0x02, 32);
        value = write_mempak_address(controllerNumber, 0xA00C, sdata_ ); //prepare for rambank read/write

        value = write_mempak_address(controllerNumber, addr, sdata );

	} else {
        value=-1;
    }

	return value;
}


int _get_gbRomAddr(const char controllerNumber, unsigned long addr, uint8_t *rdata) {

	//addr e.g. 0xC138
	int value=0;

	if((addr>=0xC000) && (addr<=0xFFFF))
		value = read_mempak_address(controllerNumber, addr, rdata );
	else
		value=-1;

	return value;
}


GameboyCart initialiseCart(char controllerNumber) {
    GameboyCart cart = {};
    memset(data, 0, 32 );

    if(_set_gbPower(controllerNumber, 0) != 0) {
        cart.errorCode = -1;
        return cart;
    }

    //get power status 0=off 1=on
    if(_get_gbPower(controllerNumber) != 0) {
        cart.errorCode = -2;
        return cart;
    }

    //set power off 0=off 1=on
    if(_set_gbPower(controllerNumber, 1) != 0) {
        cart.errorCode = -3;
        return cart;
    }

    //get power status 0=off 1=on
    if(_get_gbPower(controllerNumber) != 1) {
        cart.errorCode = -4;
        return cart;
    }

    //double check is inserted is on?

    //get access mode
    if(_get_gbAccessState(controllerNumber) == -1) {
        cart.errorCode = -5;
        return cart;
    }

    //set mode 1
    if(_set_gbAccessState(controllerNumber, 1) != 0) {
        cart.errorCode = -6;
        return cart;
    }

    //set bank 0
    if(_set_gbRomBank(controllerNumber, &cart, 0x00) != 0) {
        cart.errorCode = -7;
        return cart;
    }

    //get rdata
    if(_get_gbRomAddr(controllerNumber, 0xC120, data) != 0) {
        //header offset title 0134
        cart.errorCode = -8;
        return cart;
    }

    //current bank
    cart.bank = 0;

    for( int i = 20; i < 31; i++ ) {
        //20=0x14 -> 0x120+0x14 only 11 from 16 chars are safe
        if(data[i]!=0)
            cart.title[i-20]=data[i];
        else
            cart.title[i-20]=0x20;
    }

    //get rdata
    if(_get_gbRomAddr(controllerNumber, 0xC140, data) !=0 ) {
        //header offset cart type
        cart.errorCode = -9;
        return cart;
    }

    cart.sgb = data[6]; //0x146 super gameboy functions 0x00=no 03=yes
    cart._romsize = data[8];
    cart._ramsize = data[9];

    /*
    00h -  32KByte (no ROM banking) - but small switching circuit ^^ 16KByte Banks!
    01h -  64KByte (4 banks)
    02h - 128KByte (8 banks)
    03h - 256KByte (16 banks)
    04h - 512KByte (32 banks)
    05h -   1MByte (64 banks)  - only 63 banks used by MBC1
    06h -   2MByte (128 banks) - only 125 banks used by MBC1 - 2048000=125*16*1024
    07h -   4MByte (256 banks)
    52h - 1.1MByte (72 banks)
    53h - 1.2MByte (80 banks)
    54h - 1.5MByte (96 banks) 1572864
     */

    switch (cart._romsize) {
        case 0x00: cart.rombanks = 2; break;
        case 0x01: cart.rombanks = 4; break;
        case 0x02: cart.rombanks = 8; break;
        case 0x03: cart.rombanks = 16; break;
        case 0x04: cart.rombanks = 32; break;
        case 0x05: cart.rombanks = 64; break;
        case 0x06: cart.rombanks = 128; break;
        case 0x07: cart.rombanks = 256; break;
        case 0x52: cart.rombanks = 72; break;
        case 0x53: cart.rombanks = 80; break;
        case 0x54: cart.rombanks = 96; break;
        default: cart.rombanks = 2; break;
    }

    //set romsize;
    cart.romsize = cart.rombanks * BANKSIZE;


    //0x147 cartridge type
    switch (data[7]) {
        case 0x00:
            cart.mapper = GB_NORM;
            cart.ram = FALSE;
            cart.battery = FALSE;
            cart.rtc = FALSE;
            cart.rumble = FALSE;
            break;
        case 0x01:
            cart.mapper = GB_MBC1;
            cart.ram = FALSE;
            cart.battery = FALSE;
            cart.rtc = FALSE;
            cart.rumble = FALSE;
            break;
        case 0x02:
            cart.mapper = GB_MBC1;
            cart.ram = TRUE;
            cart.battery = FALSE;
            cart.rtc = FALSE;
            cart.rumble = FALSE;
            break;
        case 0x03:
            cart.mapper = GB_MBC1;
            cart.ram = TRUE;
            cart.battery = TRUE;
            cart.rtc = FALSE;
            cart.rumble = FALSE;
            break;
        case 0x05:
            cart.mapper = GB_MBC2;
            cart.ram = FALSE;
            cart.battery = FALSE;
            cart.rtc = FALSE;
            cart.rumble = FALSE;
            break;
        case 0x06:
            cart.mapper = GB_MBC2;
            cart.ram = TRUE; //internal ram
            cart.battery = TRUE;
            cart.rtc = FALSE;
            cart.rumble = FALSE;
            break;
        case 0x08:
            cart.mapper = GB_NORM;
            cart.ram = TRUE;
            cart.battery = FALSE;
            cart.rtc = FALSE;
            cart.rumble = FALSE;
            break;
        case 0x09:
            cart.mapper = GB_NORM;
            cart.ram = TRUE;
            cart.battery = TRUE;
            cart.rtc = FALSE;
            cart.rumble = FALSE;
            break;
        case 0x0B:
            cart.mapper = GB_MMMO1;
            cart.ram = FALSE;
            cart.battery = FALSE;
            cart.rtc = FALSE;
            cart.rumble = FALSE;
            break;
        case 0x0C:
            cart.mapper = GB_MMMO1;
            cart.ram = TRUE;
            cart.battery = FALSE;
            cart.rtc = FALSE;
            cart.rumble = FALSE;
            break;
        case 0x0D:
            cart.mapper = GB_MMMO1;
            cart.ram = TRUE;
            cart.battery = TRUE;
            cart.rtc = FALSE;
            cart.rumble = FALSE;
            break;
        case 0x0F:
            cart.mapper = GB_MBC3;
            cart.ram = FALSE;
            cart.battery = TRUE;
            cart.rtc = TRUE;
            cart.rumble = FALSE;
            break;
        case 0x10:
            cart.mapper = GB_MBC3;
            cart.ram = TRUE;
            cart.battery = TRUE;
            cart.rtc = TRUE;
            cart.rumble = FALSE;
            break;
        case 0x11:
            cart.mapper = GB_MBC3;
            cart.ram = FALSE;
            cart.battery = FALSE;
            cart.rtc = FALSE;
            cart.rumble = FALSE;
            break;
        case 0x12:
            cart.mapper = GB_MBC3;
            cart.ram = TRUE;
            cart.battery = TRUE;
            cart.rtc = FALSE;
            cart.rumble = FALSE;
            break;
        case 0x13:
            cart.mapper = GB_MBC3;
            cart.ram = TRUE;
            cart.battery = TRUE;
            cart.rtc = FALSE;
            cart.rumble = FALSE;
            break;
            //experimental
        case 0x15:
            cart.mapper = GB_MBC4;
            cart.ram = FALSE;
            cart.battery = FALSE;
            cart.rtc = FALSE;
            cart.rumble = FALSE;
            break;
        case 0x16:
            cart.mapper = GB_MBC4;
            cart.ram = TRUE;
            cart.battery = FALSE;
            cart.rtc = FALSE;
            cart.rumble = FALSE;
            break;
        case 0x17:
            cart.mapper = GB_MBC4;
            cart.ram = TRUE;
            cart.battery = TRUE;
            cart.rtc = FALSE;
            cart.rumble = FALSE;
            break;
            //experimental end
        case 0x19:
            cart.mapper = GB_MBC5;
            cart.ram = FALSE;
            cart.battery = FALSE;
            cart.rtc = FALSE;
            cart.rumble = FALSE;
            break;
        case 0x1A:
            cart.mapper = GB_MBC5;
            cart.ram = TRUE;
            cart.battery = FALSE;
            cart.rtc = FALSE;
            cart.rumble = FALSE;
            break;
        case 0x1B:
            cart.mapper = GB_MBC5;
            cart.ram = TRUE;
            cart.battery = TRUE;
            cart.rtc = FALSE;
            cart.rumble = FALSE;
            break;
        case 0x1C:
            cart.mapper = GB_MBC5;
            cart.ram = FALSE;
            cart.battery = FALSE;
            cart.rtc = FALSE;
            cart.rumble = TRUE;
            break;
        case 0x1D:
            cart.mapper = GB_MBC5;
            cart.ram = TRUE;
            cart.battery = FALSE;
            cart.rtc = FALSE;
            cart.rumble = TRUE;
            break;
        case 0x1E:
            cart.mapper = GB_MBC5;
            cart.ram = TRUE;
            cart.battery = TRUE;
            cart.rtc = FALSE;
            cart.rumble = TRUE;
            break;
        case 0xFC: //camera
            cart.mapper = GB_CAMERA;
            cart.ram = TRUE;
            cart.battery = TRUE;
            cart.rtc = FALSE;
            cart.rumble = FALSE;
            break;

            //experimental
        case 0xFF:
            cart.mapper = GB_HUC1;
            cart.ram = TRUE;
            cart.battery = TRUE;
            cart.rtc = FALSE;
            cart.rumble = FALSE;
            break;
            //experimental end
        default:
            return cart;
    }

    // 00h - None
	// 01h - 2 KBytes
	// 02h - 8 Kbytes
	// 03h - 32 KBytes (4 banks of 8KBytes each)
	// 04h - 128 KBytes (16 banks of 8KBytes each) - only camera?

	switch (cart._ramsize) {
		case 0x00:
            if(cart.mapper == GB_MBC2) {
                cart.rambanks=1;
                cart.ramsize=512; //test
            } else {
                cart.rambanks=0;
                cart.ramsize=0;
            }
            break;
		case 0x01:
            cart.rambanks=1;
			cart.ramsize=2*1024;
			break;
		case 0x02:
		 	cart.rambanks=1;
			cart.ramsize=8*1024;
			break;
		case 0x03:
            cart.rambanks=4;
			cart.ramsize=4*8*1024;
			break;
		case 0x04:
            cart.rambanks=16;
			cart.ramsize=16*8*1024;
			break;

		default:
			cart.rambanks=0;
			cart.ramsize=0;
    }

    return cart;
}

char importRom(const char controllerNumber, GameboyCart gbcart, ByteArray* romData){
    const int SEGMENT_SIZE = 0x20; // 32

    byte segment[SEGMENT_SIZE];

    unsigned long address=0xC000;
    unsigned long offset=0x00;

    romData->Data = malloc(gbcart.romsize);
    memset(romData->Data, 0xFF, gbcart.romsize);

    //copy banks to sdram
    for(int bankCount = 0; bankCount < gbcart.rombanks; bankCount++) { //bank count
        //mbc1 exceptions
        if(gbcart.mapper == GB_MBC1 && (bankCount == 0x20 || bankCount == 0x40 || bankCount == 0x60))
            bankCount++;

        //huc1 exceptions
        if(gbcart.mapper == GB_HUC1 && (bankCount == 0x20 || bankCount == 0x40 || bankCount == 0x60))
            bankCount++;

        //get power status 0=off 1=on
        if(_get_gbPower(controllerNumber) != 1)
            return -4;

        //get access mode
        _get_gbAccessState(controllerNumber);

        //int 141 0x8D
        //10000000	0x80 OS_GBPAK_GBCART_ON
        //10001101	0x8d return zz
        //00001101	0xd =

        //00000100	0x4 OS_GBPAK_RSTB_DETECTION (reset byquery)
        //00001000	0x8 OS_GBPAK_RSTB_STATUS (reset by query)
        //00000001	0x1 OS_GBPAK_POWER

        if(_set_gbRomBank(controllerNumber, &gbcart, bankCount) != 0)
            return -1;

        for(unsigned long bankAddress = address; bankAddress <= 0xFFE0; bankAddress += SEGMENT_SIZE) { //bank offset
            memset(segment, 0xFF, SEGMENT_SIZE);

            if(_get_gbRomAddr(controllerNumber, bankAddress, segment) == 0) {
                //byte temp[offset];

                // Increase romData's size by 32
                //memcpy(temp, romData->Data, offset);
                //free(romData->Data);
                //romData->Data = malloc(offset + SEGMENT_SIZE);

                //memcpy(romData->Data, temp, offset);

                // Write tpak segment to RAM
                memcpy(romData->Data + offset, segment, SEGMENT_SIZE);

                offset += SEGMENT_SIZE;
            } else {
                return -1;
            }
        }
    }
    romData->Size = offset;
    return 0;
}

char exportSave(const char controllerNumber, GameboyCart gbcart, ByteArray* saveData) {
    return 0;
}
/*
// Broken as shit because I thought it was the IMPORT save function.
char exportSave(GameboyCart gbcart, ByteArray* saveData) {
    const int SEGMENT_SIZE = 0x20; // 32

    if(gbcart.ram != TRUE)
        return -1;

    //security off for camera by now
    //if(gbcart.mapper == GB_CAMERA)
    //return -1;

    //mbc1 - mbc3
    //ram in 8KByte blocks up to 32Kb

    //mbc2
    //512x4 bits RAM

    //mbc5
    //128KByte RAM

    byte segment[SEGMENT_SIZE];
    saveData->Data = malloc(SEGMENT_SIZE);
    memset(saveData->Data, 0xFF, SEGMENT_SIZE);
    unsigned long address = 0xE000;
    unsigned long offset = 0x00;

    //copy rambanks to sdram
    for(int bankCount = 0; bankCount < gbcart.rambanks; bankCount++) {

        //get power status 0=off 1=on
        if(_get_gbPower()!=1)
            return -4;

        //get access mode
        _get_gbAccessState();

        if (_set_gbRamBank(gbcart, bankCount) != 0)
            return -1;

        int bankSize = 0xFFE0;
        if(gbcart.mapper == GB_MBC2)
            bankSize = 0xE1E0;

        for(unsigned long bankOffset = address; bankOffset <= bankSize; bankOffset += SEGMENT_SIZE) {
            memset(segment, 0xFF, SEGMENT_SIZE);
            logInfo("looping (Save) %lu", offset);
            if(_set_gbRamAddr(bankOffset, segment) == 0) {

                byte temp[offset];

                // Increase ramData's size by 32
                memcpy(temp, saveData->Data, offset);
                free(saveData);
                saveData->Data = malloc(offset + SEGMENT_SIZE);

                memcpy(saveData->Data, temp, offset);

                // Write tpak segment to RAM
                memcpy(saveData->Data + offset, segment, SEGMENT_SIZE);

                offset += SEGMENT_SIZE;
            } else {
                return -1;
            }
        }
    }

    saveData->Size = offset;

    return 0;
}*/

char importSave(const char controllerNumber, const GameboyCart gbcart, ByteArray* saveData) {
    const int SEGMENT_SIZE = 0x20; // 32

    if(gbcart.ram != TRUE) {
        return -1;
    }

    //mbc1 - mbc3
    //ram in 8KByte blocks up to 32Kb

    //mbc2
    //512x4 bits RAM

    //mbc5
    //128KByte RAM

    byte segment[SEGMENT_SIZE];

    unsigned long address = 0xE000;
    unsigned long offset = 0x00;

    saveData->Data = malloc(SEGMENT_SIZE);
    memset(saveData->Data, 0xFF, SEGMENT_SIZE);

    //copy rambanks to sdram
    for(int bankCount = 0; bankCount < gbcart.rambanks; bankCount++) {

        //get power status 0=off 1=on
        if(_get_gbPower(controllerNumber) != 1)
            return -4;

        //get access mode
        _get_gbAccessState(controllerNumber);

        if(_set_gbRamBank(controllerNumber, gbcart, bankCount) != 0)
            return -1;

        int bankSize = 0xFFE0;
        if(gbcart.mapper == GB_MBC2)
            bankSize = 0xE1E0;

        for(unsigned long bankOffset = address; bankOffset <= bankSize; bankOffset += SEGMENT_SIZE) { //bank offset

            memset(segment, 0xFF, SEGMENT_SIZE);

            if(_get_gbRamAddr(controllerNumber, bankOffset, segment) == 0) {
                byte temp[offset];
                memcpy(temp, saveData->Data, offset);
                free(saveData->Data);
                saveData->Data = malloc(offset + SEGMENT_SIZE);
                memcpy(saveData->Data, temp, offset);

                // write segment of gameboy sd ram to memory
                memcpy(saveData->Data + offset, segment, SEGMENT_SIZE);
                offset += SEGMENT_SIZE;
            } else {
                return -1;
            }
        }
    }

    saveData->Size = offset;
    return 0;
}
