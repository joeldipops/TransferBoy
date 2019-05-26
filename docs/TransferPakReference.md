# Transfer Pak Reference

I had a bit of trouble finding information on how to use the Transfer Pak over libdragon, so I wanted to collect what I've learnt in one place so the next person that comes along can get something up and running a little faster than I can.  There will be incomplete and possibly erraneous information on here so forgive me if things don't work or don't make much sense.

## Notes
* These results were confirmed in the following environment:
    * A PAL N64 purchased in Australia - NUS-001(EUR)
    * Transfer Pak - NUS-019
    * Official Controller - NUS-005
    * Official Expansion Pak - NUS-007
    * An Everdrive 64 v2.5
    * A test ROM I wrote, built with libdragon.

* Data is written to and read from the Transfer Pak in 32B blocks.  When we only care about a single byte, such as when switching banks, or checking the access mode, the block would contain that value repeated 32 times.  For simplicity, I'll just say "Reading from 0xB000 will return *0x84*" rather than "will return 0x8484848484848484848484848484848484848484848484848484848484848484"

* Memory Bank Controller I/O works the same way as in a Game Boy.  So to change to Rom bank 12, for exmaple, you'll need to write 0x0C to 0x2000 in *Game Boy* address space. To access 0x2000, the Transfer Pak bank should be 0x00 and you'll write to 0xE000 of *Transfer Pak* address space.  But then to read from ROM bank 12, you'll then need to switch the Transfer Pak bank to 0x01. Got that?

## Memory Addresses and Checksum
Although TPak addresses are presented as 16bit, the lower five bits are not part of the address but are instead reserved for a CRC Checksum.  This is why the device is read from and written to in 32byte chunks because that's as precise as the address can get.  I've determined that calculating the checksum is not actually necessary to do I/O with the Transfer Pak.  From https://github.com/sanni/cartreader/blob/master/Cart_Reader/N64.ino#L512 the algorithm to calculate the crc seems to be 

```
for bits 15 -> 5
    if the bit is set
        xor the checksum with the corresponding value in the below table
```        
15|14|13|12|11|10|9|8|7|6|5
--|-|-|-|-|-|-|-|-|-|-
 0x01|0x1A|0x0D|0x1C|0x0E|0x07|0x19|0x16|0x0B|0x1F|0x15

I haven't tested this yet...


## Memory Map

Address|Transfer Pak                      | Rumble Pak       | Controller Pak
-------|----------------------------------|------------------|-------------------
0x0000 | Echo of 0x8000 - 0x9FFF | Ununsed (0x00 0x00 ...) | Save Data area
0x1000 | ... | ... | ...
0x2000 | Echo of 0xA000 - 0xAFFF | ... | ...
0x3000 | Echo of 0xB000 - 0xBFFF | ... | ...
0x4000 | Unused.  (0x00 0x00 ...) | ... | ...
0x5000 | ... | ... | ...
0x6000 | ... | ... | ...
0x7000 | ... | ... | ...
0x8000 | Power On/Off Switch | Identifies Rumble Pak (0x80 0x80 ...) | Unused (0x00 0x00 ...)
0x9000 | ... | ... | ...
0xA000 | Bank Switch | ... | ...
0xB000 | Status and Control | ... | ...
0xC000 | Banked Cartridge Memory Access | Rumble Control. Write only.  1 to start and 0 to stop. (0x00 0x00...) | ... 
0xD000 | ... | ... | ...
0xE000 | ... | ... | ...
0xF000 | ... | ... | ...

On start up, immediately after enabling power to the Tpak, I found that the entire memory space looks like this:

Address|Value|Address|Value
-------|-----|-------|------
0x0000 | 0x84 0x84 ... | 0x8000 | 0x84 0x84 ...
0x1000 | ...           | 0x9000 | 0x84 0x84 ...
0x2000 | 0x03 0x03 ... | 0xA000 | 0x00 0x00 ...
0x3000 | 0x80 0x80 ... | 0xB000 | 0x80 0x80 ...
0x4000 | 0x00 0x00 ... | 0xC000 | 0x00 0x00 ...
0x5000 | ...           | 0xD000 | ...
0x6000 | ...           | 0xE000 | ...
0x7000 | ...           | 0xF000 | ...

### 0x0000 - 0x7FFF *Controller Pak Data*

When using a Controller Pak/Mempak the actual data stored on the Pak is read from/written to these 32kBs of address space. From what I can tell so far, The Transfer Pak does not use these addresses at all - perhaps there were plans for a combination Controller Pak/Transfer Pak or Controller Pak/Rumble Pak that never game to fruition, and they were reserved for that purpose.

When read from a Transfer pak:
* 0x0000 - 0x1FFF is probably an echo of 0x8000 - 0x9FFF 
* 0x2000 - 0x2FFF (*todo - is this an echo of 0xA000?*)
* 0x3000 - 0x3FFF is probably an echo of 0xB000 -0xBFFF
* 0x4000 - 0x7FFF is all zeroes

### 0x8000 - 0x8FFF *Power On/Off Switch*

Write to 0x8000 to power the Transfer Pak and Game Boy Cartridge on or off.

**Values**
* *0x84* enable the power
* *0xFE* disable the power

While the Transfer Pak is disabled, all addresses will return 0x00.

After enabling it, reading from 0x8000 will return 0x84.

### 0xA000 - 0xAFFF *Address Bank Switch*

The gameboy has 64kB of address space but the Transfer Pak only maps it in 16kB chunks which can be accessed at addresses 0xC000 - 0xFFFF. Writing to 0xA000 will determine which chunk, or bank, is mapped there.

**Values**
* 0x00 *0x0000 - 0x3FFF* The fixed "ROM0" section of cartridge ROM.  That is, where the cartridge header, interrupt and reset handlers, entry point, etc live.  For MBC carts, write-only addresses to switch between ROM banks will also be in this bank.

* 0x01 *0x4000 - 0x7FFF* The switchable "ROMX" ROM address space.  You'll also need to access this bank for certain MBC operations such as RAM bank switching.

* 0x02 *0x8000 - 0xBFFF* Addresses for gameboy *Video Ram* (0x8000 - 0x9FFF) which will probably be irrelevant to the TPak and *External RAM* (0xA000 - 0xBFFF) which you *are* likely to be interested in, since it's where saved games are stored, and peripherals like the clocks in Pokemon, expose their values.  Writing to this space will update the save data stored on the cartridge.  Reading from anywhere in the VRAM space should just return 0xFF.

* 0x03 *0xC000 - 0xFFFF* These addresses are used for various important things on an actual gameboy, none of which make much sense when using a TPak. Includes Work RAM, I/O registers and Object Attribute Memory.  Reading from any of these addresses should just return 0xFF

Reading back from 0xA000 will return the currently set bank number\*.  If you set a bank other than 0-3, reading this address will return 0.

\*Elsewhere, I have read that this is not the case and there's no way of reading the current bank number.  Having tested on a real n64, and I definitely get the bank number back.

### 0xB000 - 0xBFFF *Transfer Pak Status and Control*

This is a tricky one that I don't fully understand yet.  Information on what it's for and how it works is scarce, but here's what I've pulled together so far.

I believe that the data at this address are a bunch of status flags that can be used to ensure that the TPak is working properly. By writing to this address, you can also enable and disable I/O to the Game Boy cartridge.

If you just want to be able to use the TPak, all you need to know is:

1. That before you can read from the cartridge, you need to write 0x01 to 0xB000.

2. These status codes mean the Tpak isn't working
* 0x40 - no cartridge detected
* 0x88, 0x84, 0x80 - access to the cart is not enabled

3. These mean you should be good to go
* 0x081, 0x8D, 0x89 - access to the cart is enabled

#### Bit 0
has been referred to in a few places as the "Access Mode" flag.  Writing 0x00 sets Access mode 0, and 0x01 sets access mode 1

*Values*
* 0x00 - In Mode 0, all reads from Cartridge space (ie 0xC000 - 0xFFFF) are 0. *Todo - double check that writing is also disabled*
* 0x01 - In Mode 1, The Cartridge can be read from and written to as intended.

#### Bit 1
In all my tests so far, this has always been 0

#### Bits 2 & 3
These bits are used to detect if the cartridge has been reset. I gather that 'resetting' the cartridge involves setting the access mode to 0 and then to 1 again.  After a byte with these bits set has been read, they will be reset again.

The cen64 source will set bit 2 if this is the first time reading from 0xB000 after bit 0 is flipped.  Otherwise it will be reset.  My tests show different behaviour.

Here's the pattern I observed:

1. wrote 1000 0100 to   0x8000 // turns on the power
2. read  1000 0000 from 0xB000 // bits reset
3. wrote 0000 0001 to   0xB000 // enable reading from cart
4. read  1000 0001 from 0xB000 // bits 2 and 3 reset
5. read  1000 1101             // bits 2 and 3 BOTH set
6. read  1000 1001             // bit 3 set, bit 2 reset
7. read  1000 1001             // same for subsequent reads

8. wrote 0000 0000             // disable reading from cart
9. read  1000 1000             // bit 3 set, bit 2 reset
10. read  1000 0100             // bit 2 set, bit 3 reset
11. read  1000 0000             // both reset

My current theory is that because I tried reading this immediately after updating the access mode, the cartridge was still going through the reset, and the bits hadn't been set yet, leading to messier behaviour.

*Todo: confirm this*

#### Bits 4 & 5
I have never seen these bits as anything other than 0.

#### Bit 6
This bit is set if there is no Cartridge detected in the Transfer Pak.

#### Bit 7
This bit is set if power to the cartridge is enabled.

### 0xC000 - 0xFFFF *Cartridge Memory Access*

Here is where you can read from and write to the cartridge in the transfer pak, as long as you selected the appropriate 16kB bank by setting a value to 0xA000.

For examaple, if you want to read the cartridge header at 0x0100 of gameboy address space

1) Write 0x00 to 0xA000 to ensure you're on bank 0
2) read a 32B block from 0xC100 for the startup code and the check-logo
3) read a 32B block from 0xC120 for the game title and the manufacturer code
4) read a 32B block from 0xC140 for the remainder of the the header.

## An Example.

Let's assume that we have an MBC1 cartridge plugged into a transfer pak in controller slot 0, and we want to read something from a user's save file located at address 0xB000 of SRAM bank 2.

```
void write_value_to_tpak(u16 address, u8 value) {
    u8 block[32];
    memset(block, value, 32);
    write_mempak_address(0, address, block);
}

// First, enable power to the cartridge
write_value_to_tpak(0x8000, 0x84);

// Then enable cartridge access mode
write_value_to_tpak(0xB000, 0x01);

// Ensure bank 0 is selected
write_value_to_tpak(0xA000, 0x00);

// Enable RAM by writing to 0x000 of cartridge space
write_value_to_tpak(0xC000, 0x0A);

// Switch to bank 1 - we need to select "RAM" mode and set a different bank
write_value_to_tpak(0xA000, 0x01);

// cart address 0x6000 for ram banking mode.
write_value_to_tpak(0xE000, 0x01);
// Set RAM bank 2 at cart address 0x4000
write_value_to_tpak(0xC000, 0x02);

// Switch to bank 2 to read from SRAM/external RAM
write_value_to_tpak(0xA000, 0x02);

// And finally we can read from SRAM
u8 someSaveDatum[32]
read_mempak_address(0, 0xD000, someSaveDatum); // 0xD000 = 0xB000 of cart space.
```

## The Controller Pak
When I started writing this, I figured that the software in the controller pak would be at a similar level of complexity to the Transfer Pak, but it turns out there is no bank switching, control commands or anything like that. The first 32kB of address space is read-writeable memory, and the remaining 32kB is unused and returns 0x00.  Save data tends to follow a common pattern, and there may be some enforced structure on the pak which I'll explore when time permits.
This *was* tested on an OEM Pak, so take all of this with a grain of doubt until I get my hands on an official one.

## The Rumble Pak
The rumble pak is pretty simple, but it never needed to be complex.  Read from 0x8000 and you get back 0x80 to determine what you're looking at is indeed a rumble pak.  Write 0x01 to 0xC000 to make the pak rumble, and 0x00 to stop it.
