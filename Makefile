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
CFLAGS = -std=gnu99 -march=vr4300 -mtune=vr4300 -O2 -Wall -Werror -I$(CURDIR) -I$(ROOTDIR)/mips64-elf/include 
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

$(PROG_NAME)$(ROM_EXTENSION): $(PROG_NAME).elf transferboy64.dfs
	$(OBJCOPY) $(PROG_NAME).elf $(PROG_NAME).bin -O binary
	rm -f $(PROG_NAME)$(ROM_EXTENSION)
	$(N64TOOL) $(N64_FLAGS) -t "transferboy64" -s 1M transferboy64.dfs
	$(CHKSUM64PATH) $(PROG_NAME)$(ROM_EXTENSION)

$(PROG_NAME).elf : $(PROG_NAME).o
	$(LD) -o $(PROG_NAME).elf $(PROG_NAME).o $(LINK_FLAGS)

transferboy64.dfs:
	$(MKDFSPATH) transferboy64.dfs ./filesystem/

all: $(PROG_NAME)$(ROM_EXTENSION)

clean:
	rm -f *.v64 *.z64 *.elf *.o *.bin *.dfs
