ARCH      ?= x86_64
PRESET    ?= debug-$(ARCH)
BUILD_DIR ?= cmake-build-$(PRESET)

USER_DIR   := examples/app
LIBC_DIR   := libc
USER_ELF   := $(BUILD_DIR)/user.elf

KERNEL_EFI := $(BUILD_DIR)/kernel/kernel.efi
ESP_DIR    := esp_$(ARCH)

QEMU_FLAGS := -m 256M -serial stdio -drive file=fat32.img,if=none,id=drv0,format=raw -device nvme,drive=drv0,serial=1234

ifeq ($(ARCH),aarch64)
ARCH_SUFFIX  := AA64
QEMU_MACHINE := -M virt -cpu cortex-a72
QEMU_BIOS    := -bios /usr/share/edk2/aarch64/QEMU_EFI.fd
QEMU_DRIVE   := -drive file=fat:rw:$(ESP_DIR),format=raw,media=disk

else ifeq ($(ARCH),x86_64)
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

.PHONY: build run clean userspace

build:
	cmake --preset $(PRESET)
	cmake --build $(BUILD_DIR)
	$(MAKE) userspace

userspace:
	cd $(USER_DIR) && zep build
	mkdir -p $(dir $(USER_ELF))
	ld.lld -e main -Ttext 0x400000 -static -nostdlib -o $(USER_ELF) $(USER_DIR)/build/$(ARCH)-unknown-none/objs/user_app.o $(BUILD_DIR)/libc/libzep_libc.a

run: build fat32.img
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

fat32.img: userspace
	dd if=/dev/zero of=fat32.img bs=1M count=33
	mformat -F -i fat32.img ::
	mmd -i fat32.img ::System
	mmd -i fat32.img ::System/Binaries
	mmd -i fat32.img ::System/Libraries
	mmd -i fat32.img ::Users
	mmd -i fat32.img ::Users/budchirp
	mmd -i fat32.img ::Users/budchirp/Binaries
	mmd -i fat32.img ::Users/budchirp/Home
	mmd -i fat32.img ::Users/budchirp/Applications
	mmd -i fat32.img ::Config
	echo "Hello from FAT32 NVMe!" > hello_nvme.txt
	mcopy -i fat32.img hello_nvme.txt ::HELLO.TXT
	rm hello_nvme.txt
	echo -e "root:0\nbudchirp:1000" > users.txt
	mcopy -i fat32.img users.txt ::Config/users
	rm users.txt
	mcopy -i fat32.img $(BUILD_DIR)/bin/init/init ::System/Binaries/init
	mcopy -i fat32.img $(BUILD_DIR)/bin/login/login ::System/Binaries/login
	mcopy -i fat32.img $(BUILD_DIR)/bin/shell/shell ::System/Binaries/shell
	mcopy -i fat32.img $(BUILD_DIR)/bin/cat/cat ::System/Binaries/cat
	mcopy -i fat32.img $(BUILD_DIR)/bin/whoami/whoami ::System/Binaries/whoami
	mcopy -i fat32.img $(USER_ELF) ::Users/budchirp/Binaries/example

clean:
	rm -rf cmake-build-* esp_* fat32.img