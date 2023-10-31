ASM:=nasm
CC:=gcc
CC16:=/usr/bin/watcom/binl64/wcc
LD16:=/usr/bin/watcom/binl64/wlink

ASMFLAGS:=
CFLAGS:=-g -Wall
CFLAGS16:=

SRC_DIR:=SRC
SRC_TOOLS_DIR:=Tools
BUILD_DIR:=build
BUILD_TOOLS_DIR:=$(BUILD_DIR)/tools

MAKE_FLAGS:=ASM=$(ASM) CC=$(CC) CC16=$(CC16) LD16=$(LD16) BUILD_DIR=$(abspath $(BUILD_DIR)) --no-print-directory

.PHONY: all clean
.PHONY: floppy bootloader kernel
.PHONY: tools tools_fat

all: floppy tools

#
# Floppy image
#
floppy: $(BUILD_DIR)/floppy.img

$(BUILD_DIR)/floppy.img: $(BUILD_DIR)/bootloader-first-stage.bin $(BUILD_DIR)/bootloader-second-stage.bin $(BUILD_DIR)/kernel.bin | $(BUILD_DIR)
	dd if=/dev/zero of=$@ bs=512 count=2880
	mkfs.fat -F 12 -n "MUGOS" $@
	dd if=$(BUILD_DIR)/bootloader-first-stage.bin of=$@ conv=notrunc
	mcopy -i $@ $(BUILD_DIR)/bootloader-second-stage.bin "::2ndStage.bin"
	mcopy -i $@ $(BUILD_DIR)/kernel.bin "::kernel.bin"
	printf "This is a test file :D\n" > $(BUILD_DIR)/test.txt
	mcopy -i $@ $(BUILD_DIR)/test.txt "::test.txt"

#
# Bootloader
#
bootloader: $(BUILD_DIR)/bootloader-first-stage.bin $(BUILD_DIR)/bootloader-second-stage.bin

FIRST_STAGE_FILES:=$(SRC_DIR)/Bootloader/FirstStage/*.asm
SECOND_STAGE_FILES:=$(SRC_DIR)/Bootloader/SecondStage/*.c $(SRC_DIR)/Bootloader/SecondStage/*.asm

$(BUILD_DIR)/bootloader-first-stage.bin: $(FIRST_STAGE_FILES) | $(BUILD_DIR)
	$(MAKE) -C $(SRC_DIR)/Bootloader/FirstStage $(MAKE_FLAGS)

$(BUILD_DIR)/bootloader-second-stage.bin: $(SECOND_STAGE_FILES) | $(BUILD_DIR)
	$(MAKE) -C $(SRC_DIR)/Bootloader/SecondStage $(MAKE_FLAGS)

#
# Kernel
#
kernel: $(BUILD_DIR)/kernel.bin

KERNEL_FILES:=$(SRC_DIR)/Kernel/*.asm

$(BUILD_DIR)/kernel.bin: $(KERNEL_FILES) | $(BUILD_DIR)
	$(MAKE) -C $(SRC_DIR)/Kernel $(MAKE_FLAGS)

#
# Tools
#
tools: tools_fat
tools_fat: $(BUILD_TOOLS_DIR)/fat

$(BUILD_TOOLS_DIR)/fat: $(SRC_TOOLS_DIR)/FAT/Fat.c | $(BUILD_TOOLS_DIR)
	$(CC) $(CFLAGS) $(SRC_TOOLS_DIR)/FAT/Fat.c -o $@

#
# Build directories
#

$(BUILD_DIR):
	mkdir -p $@

$(BUILD_TOOLS_DIR): | $(BUILD_DIR)
	mkdir -p $@

#
# Clean
#
clean:
	$(MAKE) -C $(SRC_DIR)/Bootloader/FirstStage clean $(MAKE_FLAGS)
	$(MAKE) -C $(SRC_DIR)/Bootloader/SecondStage clean $(MAKE_FLAGS)
	$(MAKE) -C $(SRC_DIR)/Kernel clean $(MAKE_FLAGS)
	rm -rf $(BUILD_DIR)/*
