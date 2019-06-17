N64_INST = /home/joeldipops/Projects/tools/n64inst
ROOTDIR = $(N64_INST)
GCCN64PREFIX = $(ROOTDIR)/bin/mips64-elf-
CHKSUM64PATH = /home/joeldipops/Projects/tools/libdragon/tools/chksum64
MKDFSPATH = /home/joeldipops/Projects/tools/libdragon/tools/mkdfs/mkdfs
HEADERPATH = $(ROOTDIR)/mips64-elf/lib
N64TOOL = /home/joeldipops/Projects/tools/libdragon/tools/n64tool
HEADERNAME = header
LINK_FLAGS = -L$(ROOTDIR)/mips64-elf/lib -ldragon -lm -lc -ldragonsys -Tn64ld.x
PROG_NAME = transferboy64
CFLAGS = -std=gnu99 -march=vr4300 -mtune=vr4300 -Os -Wall -Wno-unused -Werror -I$(CURDIR) -I$(ROOTDIR)/mips64-elf/include 
ASFLAGS = -mtune=vr4300 -march=vr4300
CC = $(GCCN64PREFIX)gcc
AS = $(GCCN64PREFIX)as
LD = $(GCCN64PREFIX)ld
OBJCOPY = $(GCCN64PREFIX)objcopy

ifeq ($(N64_BYTE_SWAP),true)
ROM_EXTENSION = .v64
N64_FLAGS = -b -l 2M -h $(HEADERPATH)/$(HEADERNAME) -o $(PROG_NAME)$(ROM_EXTENSION) $(PROG_NAME).bin
else
ROM_EXTENSION = .z64
N64_FLAGS = -l 2M -h $(HEADERPATH)/$(HEADERNAME) -o $(PROG_NAME)$(ROM_EXTENSION) $(PROG_NAME).bin
endif

LD_OFILES =  $(CURDIR)/obj/core.o
LD_OFILES += $(CURDIR)/obj/resources.o
LD_OFILES += $(CURDIR)/obj/init.o
LD_OFILES += $(CURDIR)/obj/play.o
LD_OFILES += $(CURDIR)/obj/menu.o
LD_OFILES += $(CURDIR)/obj/options.o
LD_OFILES += $(CURDIR)/obj/logger.o
LD_OFILES += $(CURDIR)/obj/text.o
LD_OFILES += $(CURDIR)/obj/controller.o
LD_OFILES += $(CURDIR)/obj/screen.o
LD_OFILES += $(CURDIR)/obj/state.o
LD_OFILES += $(CURDIR)/obj/eeprom.o
LD_OFILES += $(CURDIR)/obj/link.o
LD_OFILES += $(CURDIR)/obj/sound.o
LD_OFILES += $(CURDIR)/obj/tpakio.o
LD_OFILES += $(CURDIR)/obj/superGameboy.o
LD_OFILES += $(CURDIR)/obj/cpu.o
LD_OFILES += $(CURDIR)/obj/emu.o
LD_OFILES += $(CURDIR)/obj/lcd.o
LD_OFILES += $(CURDIR)/obj/mmu.o
LD_OFILES += $(CURDIR)/obj/c_gbz80ops.o
LD_OFILES += $(CURDIR)/obj/s_gbz80ops.o
LD_OFILES += $(CURDIR)/obj/gbc_state.o

$(PROG_NAME)$(ROM_EXTENSION): $(PROG_NAME).elf transferboy64.dfs
	$(OBJCOPY) $(PROG_NAME).elf $(PROG_NAME).bin -O binary
	rm -f $(PROG_NAME)$(ROM_EXTENSION)
	$(N64TOOL) $(N64_FLAGS) -t "transferboy64" -s 1M transferboy64.dfs
	$(CHKSUM64PATH) $(PROG_NAME)$(ROM_EXTENSION)

$(PROG_NAME).elf :
	$(CC) $(CFLAGS) -c -o $(CURDIR)/obj/core.o $(CURDIR)/core.c
	$(CC) $(CFLAGS) -c -o $(CURDIR)/obj/resources.o $(CURDIR)/resources.c
	$(CC) $(CFLAGS) -c -o $(CURDIR)/obj/init.o $(CURDIR)/init.c
	$(CC) $(CFLAGS) -c -o $(CURDIR)/obj/play.o $(CURDIR)/play.c
	$(CC) $(CFLAGS) -c -o $(CURDIR)/obj/menu.o $(CURDIR)/menu.c
	$(CC) $(CFLAGS) -c -o $(CURDIR)/obj/options.o $(CURDIR)/options.c
	$(CC) $(CFLAGS) -c -o $(CURDIR)/obj/logger.o $(CURDIR)/logger.c
	$(CC) $(CFLAGS) -c -o $(CURDIR)/obj/text.o $(CURDIR)/text.c
	$(CC) $(CFLAGS) -c -o $(CURDIR)/obj/screen.o $(CURDIR)/screen.c
	$(CC) $(CFLAGS) -c -o $(CURDIR)/obj/state.o $(CURDIR)/state.c
	$(CC) $(CFLAGS) -c -o $(CURDIR)/obj/eeprom.o $(CURDIR)/eeprom.c
	$(CC) $(CFLAGS) -c -o $(CURDIR)/obj/link.o $(CURDIR)/link.c
	$(CC) $(CFLAGS) -c -o $(CURDIR)/obj/sound.o $(CURDIR)/sound.c
	$(CC) $(CFLAGS) -c -o $(CURDIR)/obj/tpakio.o $(CURDIR)/tpakio.c
	$(CC) $(CFLAGS) -c -o $(CURDIR)/obj/superGameboy.o $(CURDIR)/superGameboy.c
	$(CC) $(CFLAGS) -c -o $(CURDIR)/obj/controller.o $(CURDIR)/controller.c	
	$(CC) $(CFLAGS) -c -o $(CURDIR)/obj/c_gbz80ops.o $(CURDIR)/include/gbC/gbz80ops.c	
	$(CC) $(CFLAGS) -c -o $(CURDIR)/obj/s_gbz80ops.o $(CURDIR)/include/gbC/gbz80ops.S
	$(CC) $(CFLAGS) -c -o $(CURDIR)/obj/cpu.o $(CURDIR)/include/gbC/cpu.c
	$(CC) $(CFLAGS) -c -o $(CURDIR)/obj/emu.o $(CURDIR)/include/gbC/emu.c
	$(CC) $(CFLAGS) -c -o $(CURDIR)/obj/lcd.o $(CURDIR)/include/gbC/lcd.c
	$(CC) $(CFLAGS) -c -o $(CURDIR)/obj/mmu.o $(CURDIR)/include/gbC/mmu.c
	$(CC) $(CFLAGS) -c -o $(CURDIR)/obj/gbc_state.o $(CURDIR)/include/gbC/gbc_state.c
	$(CC) $(CFLAGS) -c -o $(CURDIR)/obj/transferboy64.o $(CURDIR)/transferboy64.c									

	$(LD) -o $(PROG_NAME).elf $(CURDIR)/obj/$(PROG_NAME).o $(LD_OFILES) $(LINK_FLAGS)

transferboy64.dfs:
	$(MKDFSPATH) transferboy64.dfs ./filesystem/

all: $(PROG_NAME)$(ROM_EXTENSION)

clean:
	rm -f *.v64 *.z64 *.elf *.o *.bin *.dfs
	rm -f ./obj/*.o
