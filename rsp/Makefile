include ../config.mk

OUTPUT := $(addsuffix .text, $(basename $(RSP_SCRIPTS))) $(addsuffix .data, $(basename $(RSP_SCRIPTS)))

all: $(OUTPUT)

%.dsm: %.o
	mips-linux-gnu-objdump $^ -m mips -D > $@

%.o:  %.rsp
	$(CC) -x assembler-with-cpp -c -o $@ $^

%.bin: %.o
	$(OBJCOPY) -O binary -j .text $^ $@

%_data.bin: %.o
	$(OBJCOPY) -O binary -j .data $^ $@

%.data: %_data.bin
	$(OBJCOPY) -I binary -O elf32-bigmips -B mips4300 --redefine-sym _binary_$(basename $@)_data_bin_start=$(basename $@)_data_start --redefine-sym _binary_$(basename $@)_data_bin_end=$(basename $@)_data_end --redefine-sym _binary_$(basename $@)_data_bin_size=$(basename $@)_data_size $^ $@

%.text: %.bin
	$(OBJCOPY) -I binary -O elf32-bigmips -B mips4300 --redefine-sym _binary_$(basename $@)_bin_start=$(basename $@)_code_start --redefine-sym _binary_$(basename $@)_bin_end=$(basename $@)_code_end --redefine-sym _binary_$(basename $@)_bin_size=$(basename $@)_code_size --rename-section .text=.data $^ $@

clean:
	rm -f *.o *.bin *.data *.text

.PHONY: all