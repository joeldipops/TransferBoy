include ../config.mk

# Define a variable called RSP_SCRIPTS that lists every file you want to make into a ucode.

OUTPUT := $(addsuffix .text, $(basename $(RSP_SCRIPTS))) $(addsuffix .data, $(basename $(RSP_SCRIPTS)))

all: $(OUTPUT)

# First assemble the script into an object file as if it were a normal executable.
%.o:  %.rsp
	$(CC) -x assembler-with-cpp -c -o $@ $^

# But DMEM and IMEM of the RSP are separated, so these two recipies pull the data and instruction sections out into invidual bin files
%.bin: %.o
	$(OBJCOPY) -O binary -j .text $^ $@

%_data.bin: %.o
	$(OBJCOPY) -O binary -j .data $^ $@

# Which are then reassembled back in to separate object files.
# We simplify the label names here for easier referencing from CPU code
%.data: %_data.bin
	$(OBJCOPY) -I binary -O elf32-bigmips -B mips4300 --redefine-sym _binary_$(basename $@)_data_bin_start=$(basename $@)_data_start --redefine-sym _binary_$(basename $@)_data_bin_end=$(basename $@)_data_end --redefine-sym _binary_$(basename $@)_data_bin_size=$(basename $@)_data_size $^ $@

# The text section is also relabelled as a data section, since as far as the CPU is concerned, it is.
%.text: %.bin
	$(OBJCOPY) -I binary -O elf32-bigmips -B mips4300 --redefine-sym _binary_$(basename $@)_bin_start=$(basename $@)_code_start --redefine-sym _binary_$(basename $@)_bin_end=$(basename $@)_code_end --redefine-sym _binary_$(basename $@)_bin_size=$(basename $@)_code_size --rename-section .text=.data $^ $@
	

# Produces a disassembly to help with debugging
%.dsm: %.o
	mips-linux-gnu-objdump $^ -m mips -D > $@

clean:
	rm -f *.o *.bin *.data *.text

.PHONY: all
