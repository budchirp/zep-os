ARCH      ?= x86_64
PRESET    ?= debug-$(ARCH)
BUILD_DIR ?= cmake-build-$(PRESET)

ZEP_DIR    := kernel/zep
ZEP_OBJ    := $(ZEP_DIR)/build/$(ARCH)-unknown-none/objs/kernel.o

KERNEL_EFI := $(BUILD_DIR)/kernel/kernel.efi
ESP_DIR    := esp_$(ARCH)

QEMU_FLAGS := -m 256M -serial stdio

ifeq ($(ARCH),aarch64)
OBJCOPY      := aarch64-linux-gnu-objcopy
ARCH_SUFFIX  := AA64
QEMU_MACHINE := -M virt -cpu cortex-a72
QEMU_BIOS    := -bios /usr/share/edk2/aarch64/QEMU_EFI.fd
QEMU_DRIVE   := -drive file=fat:rw:$(ESP_DIR),format=raw,media=disk

else ifeq ($(ARCH),x86_64)
OBJCOPY      := objcopy
ARCH_SUFFIX  := X64
OVMF_CODE    := /usr/share/edk2/x64/OVMF_CODE.4m.fd
OVMF_VARS    := $(ESP_DIR)/OVMF_VARS.fd

QEMU_MACHINE := -M q35
QEMU_DRIVE   := \
	-drive if=pflash,format=raw,unit=0,file=$(OVMF_CODE),readonly=on \
	-drive if=pflash,format=raw,unit=1,file=$(OVMF_VARS) \
	-drive file=fat:rw:$(ESP_DIR),format=raw,media=disk
endif

BOOT_EFI := $(ESP_DIR)/EFI/BOOT/BOOT$(ARCH_SUFFIX).EFI

.PHONY: build run clean zep

build: zep
	cmake --preset $(PRESET)
	cmake --build $(BUILD_DIR)

zep:
	cd $(ZEP_DIR) && zep build --verbose
	$(OBJCOPY) \
		--globalize-symbol=serial \
		--globalize-symbol=terminal \
		--globalize-symbol=framebuffer \
		$(ZEP_OBJ)

run: build
	mkdir -p $(dir $(BOOT_EFI))
	cp $(KERNEL_EFI) $(BOOT_EFI)

ifeq ($(ARCH),x86_64)
	test -f $(OVMF_VARS) || cp /usr/share/edk2/x64/OVMF_VARS.4m.fd $(OVMF_VARS)
endif

	qemu-system-$(ARCH) \
		$(QEMU_MACHINE) \
		$(QEMU_FLAGS) \
		$(QEMU_BIOS) \
		$(QEMU_DRIVE)

clean:
	rm -rf cmake-build-* esp_*