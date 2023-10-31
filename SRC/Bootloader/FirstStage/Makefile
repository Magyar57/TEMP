ASM?=nasm

SRC_DIR:=.
BUILD_DIR?=../../../build

.PHONY: all clean first-stage

all: first-stage

first-stage: $(BUILD_DIR)/bootloader-first-stage.bin

$(BUILD_DIR)/bootloader-first-stage.bin: $(SRC_DIR)/Boot.asm
	$(ASM) $(SRC_DIR)/Boot.asm -f bin -o $@

clean:
	rm -f $(BUILD_DIR)/bootloader-first-stage.bin
