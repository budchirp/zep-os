ARCH      ?= aarch64
PRESET    ?= debug-$(ARCH)
BUILD_DIR ?= cmake-build-$(PRESET)

KERNEL_ELF        := $(BUILD_DIR)/kernel/zep_kernel
KERNEL_DIRECT_ELF := $(BUILD_DIR)/kernel/zep_kernel_direct
ZEP_DIR           := kernel/zep
ZEP_OBJ           := $(ZEP_DIR)/build/$(ARCH)-unknown-none/objs/kernel.o
LIMINE_DIR        := iso_root
ISO_DIR           := iso_root_$(ARCH)
ISO_NAME          := zep-os-$(ARCH).iso

QEMU_FLAGS := -m 256M -serial stdio

QEMU_AARCH64_FLAGS := $(QEMU_FLAGS) -M virt -cpu cortex-a72
QEMU_X86_64_FLAGS  := $(QEMU_FLAGS) -M q35

ifeq ($(ARCH),aarch64)
QEMU_FLAGS := $(QEMU_AARCH64_FLAGS)
OBJCOPY := aarch64-linux-gnu-objcopy
EFI_BOOT := BOOTAA64.EFI
QEMU_BIOS := -bios /usr/share/edk2/aarch64/QEMU_EFI.fd
else ifeq ($(ARCH),x86_64)
QEMU_FLAGS := $(QEMU_X86_64_FLAGS)
OBJCOPY := llvm-objcopy
EFI_BOOT := BOOTX64.EFI
QEMU_BIOS :=
endif

.PHONY: build iso run run-direct debug clean

build: zep
	cmake --preset $(PRESET)
	cmake --build $(BUILD_DIR)

zep:
	cd $(ZEP_DIR) && zep build --verbose
	$(OBJCOPY) --globalize-symbol=serial \
		--globalize-symbol=terminal \
		--globalize-symbol=framebuffer \
		$(ZEP_OBJ) $(ZEP_OBJ)

iso: build
	mkdir -p $(ISO_DIR)/boot $(ISO_DIR)/EFI/BOOT
	cp $(KERNEL_ELF) $(ISO_DIR)/boot/
	cp scripts/limine.conf $(ISO_DIR)/boot/
	cp $(LIMINE_DIR)/$(EFI_BOOT) $(ISO_DIR)/EFI/BOOT/ 2>/dev/null || true
	cp $(LIMINE_DIR)/limine-bios-cd.bin $(LIMINE_DIR)/limine-uefi-cd.bin $(ISO_DIR)/boot/ 2>/dev/null || true
	xorriso -as mkisofs -b boot/limine-bios-cd.bin -no-emul-boot \
		-boot-load-size 4 -boot-info-table \
		--efi-boot boot/limine-uefi-cd.bin -efi-boot-part \
		--efi-boot-image --protective-msdos-label \
		$(ISO_DIR) -o /tmp/$(ISO_NAME) 2>/dev/null

run: iso
	qemu-system-$(ARCH) $(QEMU_FLAGS) $(QEMU_BIOS) -cdrom /tmp/$(ISO_NAME) -display gtk

run-direct: build
	qemu-system-$(ARCH) $(QEMU_FLAGS) -kernel $(KERNEL_DIRECT_ELF)

debug: build
	qemu-system-$(ARCH) $(QEMU_FLAGS) -kernel $(KERNEL_DIRECT_ELF) -s -S

clean:
	rm -rf cmake-build-* /tmp/$(ISO_NAME)
