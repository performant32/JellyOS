ASM := nasm
ASM_FLAGS := -f bin
OBJ_DIR:=bin/objs
BIN_DIR:=bin
ISO_DIR:=bin/iso
BOOT_SRC:=src/boot
KERNEL_SRC:=src/kernel
BOOTLOADER_SOURCES:= $(patsubst $(OBJ_DIR)/%.asm, $src/boot/%.asm, $(wildcard src/boot/*.asm))

.PHONY: default build run boot1 run_nographic clean fat12

default: build run

clean:
	rm -rf bin
make_files:
	echo $(wildcard src/boot/*.asm)
	mkdir -p $(OBJ_DIR)
	mkdir -p $(ISO_DIR)
build: make_files boot1
	dd if=/dev/zero of=bin/silix.floppy bs=512 count=2734
	mkfs.fat -F 12 bin/silix.floppy
	# Making fat12 fs
	mkfs.fat -F 12 -C $(BIN_DIR)/esp.img 10
	mcopy -i $(BIN_DIR)/esp.img $(BIN_DIR)/boot1.efi ::\EFI\BOOT\BOOTx64.EFI
	mkisofs -o $(BIN_DIR)/silix.iso -r -b boot1.efi -no-emul-boot \
		-boot-load-size 4 \
		-boot-info-table \
		-eltorito-alt-boot \
		-e boot1.efi\
		-isohybrid-gpt-basdat \
		$(ISO_DIR)
boot1:
	$(ASM) $(ASM_FLAGS) $(BOOT_SRC)/boot1.asm -o $(OBJ_DIR)/boot1.bin
run_nographic:
	qemu-system-x86_64 -cdrom bin/silix.iso -boot d -nographic
run:
	qemu-system-x86_64 -cdrom bin/silix.iso -boot d 
fat12:
	$(MAKE) -C tools/fat12/ fat12 OUTPUT_DIR=$(abspath bin/tools)
