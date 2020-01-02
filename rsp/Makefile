N64_INST = /home/joeldipops/Projects/tools/n64inst
N64PREFIX = $(N64_INST)/bin/mips64-elf-
CC = $(N64PREFIX)gcc
AS = $(N64PREFIX)as
LD = $(N64PREFIX)ld
AR = $(N64PREFIX)ar
OBJCOPY = $(N64PREFIX)objcopy

all: rsp
clean: rsp-clean

rsp: renderer.o ppu.o

ppu: ppu.o

renderer: renderer.o

ppu.bin: ppu.S
	$(CC) -c -o tmp.o ppu.S
	$(OBJCOPY) -O binary -j .text tmp.o ppu.bin

ppu.o: ppu.bin
	$(OBJCOPY) -I binary -O elf32-bigmips -B mips4300 --redefine-sym _binary_ppu_bin_start=ppu_code_start --redefine-sym _binary_ppu_bin_end=ppu_code_end --redefine-sym _binary_ppu_bin_size=ppu_code_size --rename-section .text=.data ppu.bin ppu.o

renderer.bin: renderer.S
	$(CC) -c -o tmp.o renderer.S
	$(OBJCOPY) -O binary -j .text tmp.o renderer.bin

renderer.o: renderer.bin
	$(OBJCOPY) -I binary -O elf32-bigmips -B mips4300 --redefine-sym _binary_renderer_bin_start=renderer_code_start --redefine-sym _binary_renderer_bin_end=renderer_code_end --redefine-sym _binary_renderer_bin_size=renderer_code_size --rename-section .text=.data renderer.bin renderer.o

rsp-clean:
	rm -f *.o *.bin

.PHONY : rsp rsp-clean