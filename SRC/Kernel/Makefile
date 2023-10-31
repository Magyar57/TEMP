ASM?=nasm

SRC_DIR:=.
BUILD_DIR?=../build

.PHONY: all clean kernel

all: kernel

kernel: $(BUILD_DIR)/kernel.bin

$(BUILD_DIR)/kernel.bin: $(SRC_DIR)/Main.asm
	$(ASM) $(SRC_DIR)/Main.asm -f bin -o $@

clean:
	rm -f $(BUILD_DIR)/kernel.bin
