N64_INST = /home/joeldipops/Projects/tools/n64inst
N64PREFIX = $(N64_INST)/bin/mips64-elf-
CC = $(N64PREFIX)gcc
AS = $(N64PREFIX)as
LD = $(N64PREFIX)ld
AR = $(N64PREFIX)ar
OBJCOPY = $(N64PREFIX)objcopy

all: rsp
clean: rsp-clean

rsp: rsp.o

rsp.bin: rsp.S
	$(CC) -c -o tmp.o rsp.S
	$(OBJCOPY) -O binary -j .text tmp.o rsp.bin

rsp.o: rsp.bin
	$(OBJCOPY) -I binary -O elf32-bigmips -B mips4300 --redefine-sym _binary_basic_bin_start=rsp_code_start --redefine-sym _binary_basic_bin_end=rsp_code_end --redefine-sym _binary_basic_bin_size=rsp_cpde_size --rename-section .text=.data rsp.bin rsp.o

rsp-clean:
	rm -f *.o *.bin

.PHONY : rsp rsp-clean