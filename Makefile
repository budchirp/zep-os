ARCH      ?= aarch64
PRESET    ?= debug
BUILD_DIR ?= cmake-build-$(PRESET)

KERNEL_ELF        := $(BUILD_DIR)/kernel/zep_kernel
KERNEL_DIRECT_ELF := $(BUILD_DIR)/kernel/zep_kernel_direct
LIMINE_DIR        := iso_root
ISO_DIR           := $(LIMINE_DIR)
ISO_NAME          := zep-os.iso

QEMU_FLAGS := -M virt -cpu cortex-a72 -m 256M -serial stdio -no-reboot -no-shutdown

.PHONY: build iso run run-iso debug clean

build:
	cmake --preset $(PRESET)
	cmake --build $(BUILD_DIR)

iso: build
	mkdir -p $(ISO_DIR)/boot $(ISO_DIR)/EFI/BOOT
	cp $(KERNEL_ELF) $(ISO_DIR)/boot/
	cp scripts/limine.conf $(ISO_DIR)/boot/
	cp $(LIMINE_DIR)/BOOTAA64.EFI $(ISO_DIR)/EFI/BOOT/ 2>/dev/null || true
	cp $(LIMINE_DIR)/limine-bios-cd.bin $(LIMINE_DIR)/limine-uefi-cd.bin $(ISO_DIR)/boot/ 2>/dev/null || true
	xorriso -as mkisofs -b boot/limine-bios-cd.bin -no-emul-boot \
		-boot-load-size 4 -boot-info-table \
		--efi-boot boot/limine-uefi-cd.bin -efi-boot-part \
		--efi-boot-image --protective-msdos-label \
		$(ISO_DIR) -o /tmp/$(ISO_NAME) 2>/dev/null

run: build
	qemu-system-$(ARCH) $(QEMU_FLAGS) -kernel $(KERNEL_DIRECT_ELF)

run-iso: iso
	qemu-system-$(ARCH) $(QEMU_FLAGS) -bios /usr/share/edk2/aarch64/QEMU_EFI.fd \
		-cdrom /tmp/$(ISO_NAME) -device ramfb -display gtk

debug: build
	qemu-system-$(ARCH) $(QEMU_FLAGS) -kernel $(KERNEL_DIRECT_ELF) -s -S

clean:
	rm -rf cmake-build-* /tmp/$(ISO_NAME)
