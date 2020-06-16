N64_INST = /home/joeldipops/Projects/tools/n64inst
N64PREFIX = $(N64_INST)/bin/mips64-elf-
CC = $(N64PREFIX)gcc
AS = $(N64PREFIX)as
LD = $(N64PREFIX)ld
AR = $(N64PREFIX)ar
OBJCOPY = $(N64PREFIX)objcopy

all: rsp
clean: rsp-clean

rsp: ppuDMG.o ppuDMGData.o renderer.o

ppuDMG: ppuDMG.o

ppuDMG.dsm: ppuDMGText.o
	mips-linux-gnu-objdump ppuDMGText.o -m mips -D > ppuDMG.dsm

ppuDMGText.o:  ppuDMG.rsp
	$(CC) -x assembler-with-cpp -c -o ppuDMGText.o ppuDMG.rsp

ppuDMG.bin: ppuDMGText.o
	$(OBJCOPY) -O binary -j .data ppuDMGText.o ppuDMGData.bin
	$(OBJCOPY) -O binary -j .text ppuDMGText.o ppuDMG.bin

ppuDMGData.o: ppuDMGData.bin
	$(OBJCOPY) -I binary -O elf32-bigmips -B mips4300 --redefine-sym _binary_ppuDMGData_bin_start=ppuDMG_data_start --redefine-sym _binary_ppuDMGData_bin_end=ppuDMG_data_end --redefine-sym _binary_ppuDMGData_bin_size=ppuDMG_data_size ppuDMGData.bin ppuDMGData.o

ppuDMG.o: ppuDMG.bin
	$(OBJCOPY) -I binary -O elf32-bigmips -B mips4300 --redefine-sym _binary_ppuDMG_bin_start=ppuDMG_code_start --redefine-sym _binary_ppuDMG_bin_end=ppuDMG_code_end --redefine-sym _binary_ppuDMG_bin_size=ppuDMG_code_size --rename-section .text=.data ppuDMG.bin ppuDMG.o

ppuDMG: ppuDMG.o

### LEGACY RENDERER SO CAN 'SUPPORT' GBC FOR NOW
rendererText.o:  renderer.rsp
	$(CC) -x assembler-with-cpp -c -o rendererText.o renderer.rsp

renderer.bin: rendererText.o
	$(OBJCOPY) -O binary -j .data rendererText.o rendererData.bin
	$(OBJCOPY) -O binary -j .text rendererText.o renderer.bin

renderer.o: renderer.bin
	$(OBJCOPY) -I binary -O elf32-bigmips -B mips4300 --redefine-sym _binary_renderer_bin_start=renderer_code_start --redefine-sym _binary_renderer_bin_end=renderer_code_end --redefine-sym _binary_renderer_bin_size=renderer_code_size --rename-section .text=.data renderer.bin renderer.o

rsp-clean:
	rm -f *.o *.bin

.PHONY : rsp rsp-clean