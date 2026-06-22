#include "runtime/runtime.h"

#include <efi.h>
#include <efiprot.h>

import zep.std.types;
import zep.boot.info;

extern "C" void kernel_init(BootInfo* boot_info);

static EFI_BOOT_SERVICES* boot_services_ptr = nullptr;

static void* uefi_allocate_pages(usize count) {
    if (boot_services_ptr == nullptr) {
        return nullptr;
    }

    EFI_PHYSICAL_ADDRESS addr = 0;
    EFI_STATUS status =
        boot_services_ptr->AllocatePages(AllocateAnyPages, EfiLoaderData, count, &addr);
    if (status != EFI_SUCCESS) {
        return nullptr;
    }

    return reinterpret_cast<void*>(addr);
}

static void uefi_free_pages(void* block, usize count) {
    if (boot_services_ptr != nullptr && block != nullptr) {
        boot_services_ptr->FreePages(reinterpret_cast<EFI_PHYSICAL_ADDRESS>(block), count);
    }
}

extern "C" {

EFI_STATUS _entry(void* image, EFI_SYSTEM_TABLE* system_table) {
    (void)image;

    boot_services_ptr = system_table->BootServices;

    BootInfo boot_info;
    boot_info.console = system_table->ConOut;
    boot_info.pages.allocate = uefi_allocate_pages;
    boot_info.pages.free = uefi_free_pages;

    constexpr usize HEAP_PAGES = 4096;
    constexpr usize HEAP_SIZE = HEAP_PAGES * 4096;

    void* heap_mem = uefi_allocate_pages(HEAP_PAGES);
    if (heap_mem != nullptr) {
        boot_info.heap.memory = heap_mem;
        boot_info.heap.size = HEAP_SIZE;
    }

    EFI_GUID gop_guid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
    EFI_GRAPHICS_OUTPUT_PROTOCOL* gop = nullptr;
    EFI_STATUS status = system_table->BootServices->LocateProtocol(&gop_guid, nullptr,
                                                                   reinterpret_cast<void**>(&gop));

    if (status == EFI_SUCCESS && gop != nullptr && gop->Mode != nullptr &&
        gop->Mode->Info != nullptr) {
        boot_info.framebuffer.address =
            reinterpret_cast<void*>(static_cast<uintptr>(gop->Mode->FrameBufferBase));
        boot_info.framebuffer.width = static_cast<u64>(gop->Mode->Info->HorizontalResolution);
        boot_info.framebuffer.height = static_cast<u64>(gop->Mode->Info->VerticalResolution);
        boot_info.framebuffer.pitch = static_cast<u64>(gop->Mode->Info->PixelsPerScanLine) * 4;
        boot_info.framebuffer.bpp = 32;
    }

    EFI_GUID block_io_guid = EFI_BLOCK_IO_PROTOCOL_GUID;
    EFI_GUID device_path_guid = EFI_DEVICE_PATH_PROTOCOL_GUID;
    UINTN num_handles = 0;
    EFI_HANDLE* handles = nullptr;

    status = system_table->BootServices->LocateHandleBuffer(ByProtocol, &block_io_guid, nullptr,
                                                            &num_handles, &handles);

    if (status == EFI_SUCCESS && handles != nullptr && num_handles > 0) {
        auto* disks = reinterpret_cast<BootDisk*>(
            uefi_allocate_pages((num_handles * sizeof(BootDisk) + 4095) / 4096));

        usize valid_nvme_disks = 0;

        if (disks != nullptr) {
            for (UINTN i = 0; i < num_handles; ++i) {
                EFI_DEVICE_PATH* dp = nullptr;
                status = system_table->BootServices->OpenProtocol(
                    handles[i], &device_path_guid, reinterpret_cast<void**>(&dp), nullptr, nullptr,
                    EFI_OPEN_PROTOCOL_GET_PROTOCOL);

                if (status != EFI_SUCCESS || dp == nullptr) {
                    continue;
                }

                bool is_nvme = false;
                EFI_DEVICE_PATH* node = dp;

                while (node != nullptr) {
                    if (node->Type == 0x7F) {
                        break;
                    }

                    if (node->Type == 3 && node->SubType == 23) {
                        is_nvme = true;
                    }

                    u16 node_len = static_cast<u16>((node->Length[1] << 8) | node->Length[0]);
                    if (node_len < sizeof(EFI_DEVICE_PATH)) {
                        break;
                    }

                    node =
                        reinterpret_cast<EFI_DEVICE_PATH*>(reinterpret_cast<u8*>(node) + node_len);
                }

                if (is_nvme) {
                    EFI_BLOCK_IO* block_io = nullptr;
                    status = system_table->BootServices->OpenProtocol(
                        handles[i], &block_io_guid, reinterpret_cast<void**>(&block_io), nullptr,
                        nullptr, EFI_OPEN_PROTOCOL_GET_PROTOCOL);

                    if (status == EFI_SUCCESS && block_io != nullptr) {
                        if (block_io->Media != nullptr && block_io->Media->MediaPresent) {
                            usize total_size = static_cast<usize>(block_io->Media->LastBlock + 1) *
                                               block_io->Media->BlockSize;
                            void* ram_disk = uefi_allocate_pages((total_size + 4095) / 4096);
                            if (ram_disk != nullptr) {
                                status = block_io->ReadBlocks(block_io, block_io->Media->MediaId, 0,
                                                              total_size, ram_disk);
                                if (status == EFI_SUCCESS) {
                                    disks[valid_nvme_disks].address = ram_disk;
                                    disks[valid_nvme_disks].size = total_size;
                                    valid_nvme_disks++;
                                } else {
                                    uefi_free_pages(ram_disk, (total_size + 4095) / 4096);
                                }
                            }
                        }
                    }
                }
            }

            boot_info.disks = disks;
            boot_info.disk_count = valid_nvme_disks;
        }
    }

    UINTN map_size = 0;
    UINTN descriptor_size = 0;
    UINT32 descriptor_version = 0;
    UINTN key = 0;

    system_table->BootServices->GetMemoryMap(&map_size, nullptr, &key, &descriptor_size,
                                             &descriptor_version);
    map_size += 2 * descriptor_size;

    void* raw_map = uefi_allocate_pages((map_size + 4095) / 4096);
    if (raw_map != nullptr) {
        status = system_table->BootServices->GetMemoryMap(
            &map_size, reinterpret_cast<EFI_MEMORY_DESCRIPTOR*>(raw_map), &key, &descriptor_size,
            &descriptor_version);

        if (status == EFI_SUCCESS) {
            usize count = map_size / descriptor_size;
            auto* entries = reinterpret_cast<MemoryMapEntry*>(
                uefi_allocate_pages((count * sizeof(MemoryMapEntry) + 4095) / 4096));

            if (entries != nullptr) {
                for (usize i = 0; i < count; ++i) {
                    auto* desc = reinterpret_cast<EFI_MEMORY_DESCRIPTOR*>(
                        reinterpret_cast<u8*>(raw_map) + i * descriptor_size);

                    entries[i].type = desc->Type;
                    entries[i].physical_start = desc->PhysicalStart;
                    entries[i].num_pages = desc->NumberOfPages;
                    entries[i].attribute = desc->Attribute;
                }

                boot_info.memory_map.entries = entries;
                boot_info.memory_map.count = count;
                boot_info.memory_map.key = key;
            }
        }
    }

    status = system_table->BootServices->GetMemoryMap(&map_size, nullptr, &key, &descriptor_size,
                                                      &descriptor_version);
    if (status == EFI_BUFFER_TOO_SMALL) {
        map_size += 2 * descriptor_size;
        void* temp_map = uefi_allocate_pages((map_size + 4095) / 4096);
        if (temp_map != nullptr) {
            status = system_table->BootServices->GetMemoryMap(
                &map_size, reinterpret_cast<EFI_MEMORY_DESCRIPTOR*>(temp_map), &key,
                &descriptor_size, &descriptor_version);
        }
    }

    system_table->BootServices->ExitBootServices(image, key);

    kernel_init(&boot_info);

    return EFI_SUCCESS;
}
}
