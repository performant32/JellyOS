ASM := nasm
ASM_FLAGS :=
OBJ_DIR:=bin/objs
BIN_DIR:=bin
ISO_DIR:=bin/iso
BOOTLOADER_SOURCES:= $(patsubst $(OBJ_DIR)/%.asm, $src/boot/%.asm, $(wildcard src/boot/*.asm))

.PHONY: build run

default: build run
make_files:
	echo $(wildcard src/boot/*.asm)
	mkdir -p $(OBJ_DIR)
	mkdir -p $(ISO_DIR)
build: make_files boot1
	mkisofs -o $(BIN_DIR)/silix.iso -b boot1.bin -no-emul-boot $(ISO_DIR)
boot1:
	$(ASM) $(ASM_FLAGS) $(BOOTLOADER_SOURCES) -o $(ISO_DIR)/boot1.bin
run:
	qemu-system-i386 -cdrom bin/silix.iso -boot d
