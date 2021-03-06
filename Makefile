N64_INST = /home/joeldipops/Projects/tools/n64inst
ROOTDIR = $(N64_INST)
GCCN64PREFIX = $(ROOTDIR)/bin/mips64-elf-
CHKSUM64PATH = /home/joeldipops/Projects/tools/libdragon/tools/chksum64
MKDFSPATH = /home/joeldipops/Projects/tools/libdragon/tools/mkdfs/mkdfs
HEADERPATH = $(ROOTDIR)/mips64-elf/lib
N64TOOL = /home/joeldipops/Projects/tools/libdragon/tools/n64tool
HEADERNAME = header
PROG_NAME = transferboy

PRE_LD_FILE = $(PROG_NAME).ld
LD_FILE = $(PROG_NAME)_generated.ld
LINK_FLAGS = -L$(ROOTDIR)/mips64-elf/lib -ldragon -lm -lc -ldragonsys -T./$(LD_FILE)

#O3FLAGS = -fgcse-after-reload -finline-functions -fipa-cp-clone -floop-interchange -floop-unroll-and-jam -fpredictive-commoning -fsplit-paths -ftree-loop-distribute-patterns -ftree-loop-distribution -ftree-loop-vectorize -ftree-partial-pre -ftree-slp-vectorize -funswitch-loops -fvect-cost-model -fversion-loops-for-strides
OPTIMISATION_FLAGS = -O3
CFLAGS = -std=gnu99 -march=vr4300 -mtune=vr4300 $(OPTIMISATION_FLAGS) -Wall -Wno-unused -Werror -I$(CURDIR) -I$(ROOTDIR)/mips64-elf/include 
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

all: $(PROG_NAME)$(ROM_EXTENSION) $(PROG_NAME).mips

$(CURDIR)/rsp/ppuDMG.o:
	make -C $(CURDIR)/rsp rsp

$(CURDIR)/rsp/renderer.o:
	make -C $(CURDIR)/rsp rsp

$(PROG_NAME)$(ROM_EXTENSION): $(PROG_NAME).elf $(PROG_NAME).dfs
	$(OBJCOPY) $(PROG_NAME).elf $(PROG_NAME).bin -O binary
	rm -f $(PROG_NAME)$(ROM_EXTENSION)
	$(N64TOOL) $(N64_FLAGS) -t "$(PROG_NAME)" -s 1M $(PROG_NAME).dfs
	$(CHKSUM64PATH) $(PROG_NAME)$(ROM_EXTENSION)

LD_OFILES =  $(CURDIR)/obj/core.o
LD_OFILES += $(CURDIR)/obj/debug.o
LD_OFILES += $(CURDIR)/obj/ppu.o
LD_OFILES +=  $(CURDIR)/obj/progressBar.o
LD_OFILES += $(CURDIR)/obj/fps.o
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
LD_OFILES += $(CURDIR)/obj/polyfill.o
LD_OFILES += $(CURDIR)/obj/rtc.o
LD_OFILES += $(CURDIR)/obj/rsp.o

# Produces the disassembly, with symbols included.
$(PROG_NAME).dsm: $(PROG_NAME).elf
	mips-linux-gnu-objdump $(PROG_NAME).elf -m mips -D > $(PROG_NAME).dsm

$(PROG_NAME).elf : $(CURDIR)/rsp/ppuDMG.o $(CURDIR)/rsp/ppuDMGData.o $(PROG_NAME).o $(LD_FILE)
	$(CC) $(CFLAGS) -c -o $(CURDIR)/obj/debug.o $(CURDIR)/debug.c
	$(CC) $(CFLAGS) -c -o $(CURDIR)/obj/ppu.o $(CURDIR)/ppu.c
	$(CC) $(CFLAGS) -c -o $(CURDIR)/obj/progressBar.o $(CURDIR)/progressBar.c
	$(CC) $(CFLAGS) -c -o $(CURDIR)/obj/fps.o $(CURDIR)/fps.c
	$(CC) $(CFLAGS) -c -o $(CURDIR)/obj/rtc.o $(CURDIR)/rtc.c
	$(CC) $(CFLAGS) -c -o $(CURDIR)/obj/polyfill.o $(CURDIR)/polyfill.c
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
	$(CC) $(CFLAGS) -c -o $(CURDIR)/obj/c_gbz80ops.o $(CURDIR)/gbz80ops.c
	$(CC) $(CFLAGS) -c -x assembler-with-cpp \
					   -o $(CURDIR)/obj/s_gbz80ops.o $(CURDIR)/gbz80ops.mips
	$(CC) $(CFLAGS) -c -o $(CURDIR)/obj/cpu.o $(CURDIR)/cpu.c
	$(CC) $(CFLAGS) -c -o $(CURDIR)/obj/emu.o $(CURDIR)/emu.c
	$(CC) $(CFLAGS) -c -o $(CURDIR)/obj/lcd.o $(CURDIR)/lcd.c
	$(CC) $(CFLAGS) -c -o $(CURDIR)/obj/mmu.o $(CURDIR)/mmu.c
	$(CC) $(CFLAGS) -c -o $(CURDIR)/obj/gbc_state.o $(CURDIR)/gbc_state.c
	$(CC) $(CFLAGS) -c -o $(CURDIR)/obj/rsp.o $(CURDIR)/rsp.c
	$(CC) $(CFLAGS) -c -o $(CURDIR)/obj/transferboy.o $(CURDIR)/transferboy.c

	$(LD) -o $(PROG_NAME).elf $(CURDIR)/rsp/ppuDMG.o $(CURDIR)/rsp/ppuDMGData.o $(CURDIR)/rsp/renderer.o $(CURDIR)/obj/$(PROG_NAME).o $(LD_OFILES) $(LINK_FLAGS)

$(LD_FILE) : $(PRE_LD_FILE)
	cpp $(PRE_LD_FILE) | grep -v '^#'	>>$(LD_FILE)

$(PROG_NAME).dfs:
	./mksprite.sh
	cp ./assets/*.gb ./filesystem/
	$(MKDFSPATH) $(PROG_NAME).dfs ./filesystem/

clean:
	rm -f *.v64 *.z64 *.elf *.o *.bin *.dfs $(LD_FILE)
	rm -f ./obj/*.o
	$(MAKE) -C ./rsp/ clean
