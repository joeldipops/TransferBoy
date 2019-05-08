# Transfer Pak Reference

I had a bit of trouble finding information on how to use the Transfer Pak over libdragon, so I wanted to collect what I've learnt in one place so the next person that comes along can get something up and running a little faster than I can.  There will be incomplete and possibly erraneous information on here so forgive me if things don't work or don't make much sense.

## Notes
* Data is written to and read from the Transfer Pak in 32B blocks.  When we only care about a single byte, such as when switching banks, or checking the access mode, the block would contain that value repeated 32 times.  For simplicity, I'll just say "Reading from 0xB000 will return *0x84*" rather than "will return 0x8484848484848484848484848484848484848484848484848484848484848484"

* Memory Bank Controller I/O works the same way as in a Game Boy.  So to change to Rom bank 12, for exmaple, you'll need to write 0x0C to 0x2000 in *Game Boy* address space. To access 0x2000, the Transfer Pak bank should be 0x00 and you'll write to 0xE000 of *Transfer Pak* address space.  But then to read from ROM bank 12, you'll then need to switch the Transfer Pak bank to 0x01.  Ya dig?

## Memory Map

### 0x0000 - 0x7FFF *Controller Pak Data*

When using a Controller Pak/Mempak the actual data stored on the Pak is read from/written to these 32kBs of address space. From what I can tell so far, The Transfer Pak does not use these addresses at all - perhaps there were plans for a combination Controller Pak/Transfer Pak or Controller Pak/Rumble Pak that never game to fruition, and they were reserved for that purpose.

### 0x8000 - 0x8FFF *On/Off Switch*

Write to 0x8000 (or presumably any address in this range) to power the Transfer Pak on or off.

**Values**
* *0x80*
* *0x84* enables Transfer Pak
* *0xFE* disable Transfer Pak

### 0xA000 - 0xAFFF *Address Bank Switch*

The gameboy has 64kB of address space but the Transfer Pak only maps it in 16kB chunks. Writing to this address will determine which chunk is mapped to at a given time.

**Values**
* *0x00* 0x0000 - 0x3FFF The fixed "ROM0" section of cartridge ROM ie. where the cartridge header, interrupt and reset handlers, entry point and main game logic lives.  For MBC carts, write-only addresses to switch between ROM banks will also be in this bank.

* *0x01* 0x4000 - 0x7FFF The switchable "ROMX" ROM address space.  You'll also need to access this bank for certain MBC operations such as RAM bank switching.

* *0x02* 0x8000 - 0xBFFF Addresses for gameboy *Video Ram* (0x8000 - 0x9FFF) which will probably be irrelevant to the TPak and *External RAM* (0xA000 - 0xBFFF) which you are likely to be interested in because it's where saved games are stored and peripherals like the clocks in Pokemon expose their values.

* *0x03* 0xC000 - 0xFFFF These addresses are used for various important things on an actual gameboy, none of which make much sense when reading from a cartridge.  Includes Work RAM, I/O registers and Object Attribute Memory.

### 0xB000 - 0xBFFF *Cartridge detection and 'Access Mode' Switch*

Firstly, if you read from this address space and there's no cartridge inserted into the Tpak, the return value will be *0x40*.

Otherwise, the value we depend on the access mode.

The value set here is referred to as the "Access Mode" in a few places, and has a complicated system of determining when and how it has been set, but so far I've been unable to establish what it actually changes.

Here are the values involved

* *0x00* Mode 0?
After changing to ode 0, The first time you read from 0xB000, it will return *0x84*
Subsequent times, it will return *0x80*

* *0x01* Mode 1? 
After changind to Mode 1, The first time you read from 0xB000, it will return *0x8D*
Subsequent times, it will return *0x89*

### 0xC000 - 0xFFFF *Cartridge Access*

Here is where you can read from and write to the cartridge in the transfer pak, as long as you selected the appropriate 16kB bank by setting a value to 0xA000.

So if you want to read start-up code and cartridge header.

1) write 0x00 to 0xA000
2) read a 32B block from 0xC100 for the startup code and the check-logo
3) read a 32B block from 0xC120 for the game title and the manufacturer code
4) read a 32B block from 0xC140 for the remainder of the the header.









