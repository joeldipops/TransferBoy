#ifndef TPAKIO_INCLUDED
#define TPAKIO_INCLUDED

#include "core.h"

// Must be packed into a single byte to fit in the header.
typedef enum __attribute__ ((packed)) {
    GBC_NOT_SUPPORTED = 0x00, 
    GBC_DMG_SUPPORTED = 0x80,
    GBC_ONLY_SUPPORTED = 0xC0 
} GbcSupport;

// Must be packed into a single byte to fit in the header
typedef enum __attribute__ ((packed)) {
    // 32kB ROM
    ROM_ONLY = 0x00,

    // MBC1 - max 2MB ROM and/or 32kB RAM)
    MBC1 = 0x01,
    MBC1_RAM = 0x02, 
    MBC1_BATTERY = 0x03,
    
    // MBC2 - max 256kB ROM and 256B RAM
    MBC2 = 0x05,
    MBC2_BATTERY = 0x06,
    MBC2_RAM = 0x08,
    MBC2_RAM_BATTERY = 0x09,

    // MMO1 - max 8MB ROM and 128kB RAM
    MMM01 = 0x0B,
    MMM01_RAM = 0x0C,
    MMM01_RAM_BATTERY = 0x0D,

    // MBC3 - max 2MB ROM and/or 32kB RAM and Timer
    MBC3_TIMER_BATTERY = 0x0f,
    MBC3_TIMER_RAM_BATTERY = 0x10,
    MBC3 = 0x11,
    MBC3_RAM = 0x012,
    MBC3_RAM_BATTERY = 0x013,

    // MBC5 - max 8MB ROM and/or 128kB RAM
    MBC5 = 0x19,
    MBC5_RAM = 0x1A,
    MBC5_RAM_BATTERY = 0x1B,
    MBC5_RUMBLE = 0x1C,
    MBC5_RUMBLE_RAM = 0x1D,
    MBC5_RUMBLE_RAM_BATTERY = 0x1E,

    // MBC6 - who knows?
    MBC6 = 0x20,

    // MBC7 - max 8MB ROM or 256kB RAM and Accelerometer
    MBC7_SENSOR_RUMBLE_RAM_BATTERY = 0x22    
} CartridgeType;

typedef struct {
    byte entryScript[4];
    byte logo[48];
    union {
        byte DMGTitle[16];
        struct {
            byte DMGTitle[15];
            GbcSupport GbcSupport;
        } OldCGBTitle;
        struct {
            byte CGBTitle[11];
            byte ManufacturerCode[4];
            GbcSupport GbcSupport;            
        } CGBTitle;
    };
    natural NewLicenseeCode;
    bool IsSgbSupported;
    CartridgeType CartridgeType;
    byte RomSizeCode;
    byte RamSizeCode;
    byte Destination;
    byte OldLicenseeCode;
    byte VersionNumber;
    byte HeaderChecksum;
    natural GlobalChecksum;
    byte overflow[16];
} CartridgeHeader;

typedef struct {
    ByteArray Rom;
    ByteArray Ram;
    CartridgeHeader Header;
    natural RomBankCount;
    byte RamBankCount;
    natural RamBankSize;
} GameBoyCartridge;

/**
 * Imports the entire cartridge in to RAM as CartridgeData
 * @param controllerNumber get from T-Pak plugged in to this controller slot.
 * @out catridge GB/GBC catridge rom/ram 
 * @returns Error Code
 **   0 - Successful
 **  -1 - Invalid controller slot (must be 1-4)
 **  -2 - Cartridge header contains invalid values.
 **  -3 - Header failed checksum
 ** -10 - Controller not plugged in.
 ** -20 - Transfer pak not detected.
 ** -30 - Data corruption.
 ** -40 - No cartridge in Tpak.
 ** -50 - Tpak not behaving as expected.
 */
sByte importCartridge(byte controllerNumber, GameBoyCartridge* cartridge);

/**
 * Gets the complete ROM data from the cartridge a transfer pak.
 * @param controllerNumber T-Pak plugged in to this controller slot.
 * @param cartridge Structure to copy ROM to.
 * @returns Error codes
 ** 0  - Successful
 ** -1 - Error
 */
sByte _importRom(const byte controllerNumber, GameBoyCartridge* cartridge);

/**
 * Gets the complete ROM data from the cartridge in a transfer pak.
 * @param controllerNumber T-Pak plugged in to this controller slot.
 * @param cartridge Structure to copy RAM to.
 * @returns Error codes
 **  0 - Successful
 ** -1 - Error
 */
sByte _importRam(const byte controllerNumber, GameBoyCartridge* cartridge);


/*********************************************************************************************
 * Saturnu's (translated) writeup from 
 * https://circuit-board.de/forum/index.php/Thread/13481-N64-Transfer-Pak-PIF-commands-Reverse-Engineering/

The whole works similar to the controller Pak. 
First you check if a transfer-Pak is inserted and afterwards if it is switched on. 
Then it will be turned on and checked again. 
Bank 0 should actually be preset, so that the header can be read out. 
Now you can use the header to determine the ROM size and which banking needs to be applied. 
See here gbdev.gg8.se/wiki/articles/The...ge_Header#0148_-_ROM_Size 
The Transfer-Pak should support MBC1 through MBC5. 

Version 0.1 : D maybe something has to be corrected again ... 

To read the RAM with libdragon now at 0xA000 in Gameboy-Cartspace, you have to guess so 
just set the banking correctly. (0xe000 bank2?) 
The information of the RTC in Pokemon games are probably just in RAM as bytes. 

Short: 
command 00 - joystick status, if a transferpak exists 
command 03-8001 - set: transfer pak power - on / off (0x84 = on) (0xFE = off) 
command 02-8001 - query: transfer pak power - on / off (0x84 = on) 
command 02-B010 - query enable state-Data (0x89 / 0x84) mode 1/0 - (0x00) gb cart missing? 
command 03-B010 - set access mode 0x00 0x01? for what? 
command 03-A00C - set: bankswitching via data area - first byte-> banknumber - better fill all 
command 02-CXXX to FXXX-GB read ROM in 32kbyte chunks - XXX bank-offset 

Ex. Romoffset for C123: 
fileoffset = ((C123 & 0xFFE0) - 0xC000) + ((bank & 3) * 0x4000); 

EDIT: Bankswitching works a little differently depending on the mbc and 0xB000 also contains bitwise information as to whether the TPAK has been removed or has power. 
It is reset by a 0xB000 query. Ram Bankswitching works much the same way, but you have to change the source bank, and then the whole game goes on in 8Kb Banks instead of 16s. 

Here are details how the PIF needs to be addressed. 
hcs64.com/files/n64-hw.dox 


In libdragon, this looks more or less like this: 
-man can simply take the internal mempak functions, since these are very suitable and 
the same checksum procedure must be used. 
From there you can continue via SI (Serial Interface) to the PIF (Peripheral Interface) and the data 
possibly from the PIF-Ram (microcontroller Ram) 
copied to the RDRAM (N64 Ram). The available PIF RAM area is very small, so you can in 
Read out 32byte blocks and reassemble it in the RDRAM. ^^
So it takes 1,2-2,0ms, per block. :> 
1024 * 1024/32 * 1,5 / 1000 = 49,152 ~~ approx. Ne minute per Mb is that possible? ^^

memset (data, 0x84, 32); 
ret = write_mempak_address (0, 0x8001, data); 
ret = read_mempak_address (0, 0x8001, data); 
memset (data, 0x84, 32); 
ret = write_mempak_address (0, 0x8001, data); 
ret = read_mempak_address (0, 0xB010, data); 
memset (data, 0x01, 32); 
ret = write_mempak_address (0, 0xB010, data); 
memset (data, 0x00, 32); 
ret = write_mempak_address (0, 0xA00C, data); 
ret = read_mempak_address (0, 0xC138, data); 


RAM area address 0xa000. - gb cartspace

1. joystick query
gbpak present?
FF 01 03 00-FF
query is pak is inserted into joystick 0

2. write set enable. #it is used to enable/disable the GB cart FE=disable 84=enable
FF 23 01 03-80 01
32bytes * FE at 0x8001 -> disable pak
write ok with crc?

4. read #is enable? is used to query the enable state. 1 = enabled, 0 = disabled.
FF 03 21 02-80 01 
32bytes
-> recv 32*00 -> pak is disabled
-> recv 00=disabled 84=enabled

5. write set enable? #it is used to enable/disable the GB cart
FF 23 01 03-80 01
0x23*84 -> enable the cart
->recv crc 1e

6.read # is power on? is enable? is used to query the enable state. 1 = enabled, 0 = disabled.
FF 03 21 02-80 01
32 byte
recv 0x84 + 1e crc -> cart now enable
-> recv 00=disabled 84=enabled

7. status query joystick
01 03 00 FF
rec: 05-00 01 FE
query is pak is inserted into joystick 0

8. read #is used to query the enable state. 1 = enabled, 0 = disabled.
FF 03 21 02-80 01
recv x84er + 1e crc -> cart now enable (doublecheck?)
-> recv 00=disabled 84=enabled

9.read #query the access mode
FF 03 21 02-B0 10 
recv 32*0x80 +crc 2d -> access mode 0 is set
89=mode 1
80=mode 0
40=no cart inserted

10. query controller 01 03 00 FF
query is pak is inserted into joystick 0

11. read 0x8001 #is used to query the enable state. 1 = enabled, 0 = disabled.
FF 03 21 02-80 01
recv: 32*0x80 + crc b8 -> 0x80 left from access mode query

12. query controller 01 03 00 FF
query is pak is inserted into joystick 0

13. write 0xb010 #is used to set some access mode of some sort. It's not known what this is used for.
FF 23 01 03-B0 10
0x23*01 -> 01 or 00
recv: + crc eb

14. write 0xa00c #is used to set the current TPak bank. This is used when reading/writing the GB Cart.
data[0]=banknr
FF 23 01 03-A0 0C
0x23*0x00 -> set bank 0
recv: 00?

15. read 0xc10d #read cart romspace
FF 03 21 02-C1 0D
recv: 0x21 bytes
at gb-rom offset 0x101
-> 0xcXXX 0xdXXX 0xeXXX 0xfXXX
->->((dwAddress & 0xFFE0) - 0xC000) + ((tPak->iCurrentBankNo & 3) * 0x4000) );

16. read 0xc138 #read cart romspace
FF 03 21 02-C1 38
recv: 0x21 bytes
e.g. rom name in header

17. read 0xc152 #read cart romspace
FF 03 21 02-C1 52

18. read 0x8001 #is used to query the enable state. 1 = enabled, 0 = disabled.
FF 03 21 02-80 01
recv: 0x21*0x84 +crc

19. read 0xb010 #query the cart enable state
FF 03 21 02-B0 10
cart enable?
recv: 0x89er
Returns 0x89 when access mode = 1, 0x80 when mode = 0
Sets bit 2 of the first return value if the access mode was recently changed. 
Will not set bit 2 again untill access mode is changed again. 

*/
#endif
