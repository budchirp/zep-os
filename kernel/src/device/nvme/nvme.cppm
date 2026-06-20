module;

#include "runtime/runtime.h"

#include <efi.h>
#include <efiprot.h>

export module zep.device.nvme;

import zep.device;
import zep.std.types;

export class NvmeDriver : public Device {
  private:
    EFI_BLOCK_IO* block_io = nullptr;
    u32 media_id = 0;
    usize block_size = 512;
    u8* bounce_buffer = nullptr;

  public:
    explicit NvmeDriver(EFI_BLOCK_IO* block_io);
    ~NvmeDriver() override;

    string name() override;
    usize read(usize offset, u8* dest, usize size) override;
    usize write(usize offset, const u8* src, usize size) override;
};

alignas(NvmeDriver) static unsigned char nvme_storage[sizeof(NvmeDriver)];

NvmeDriver::NvmeDriver(EFI_BLOCK_IO* block_io) : block_io(block_io) {
    media_id = block_io->Media->MediaId;
    block_size = block_io->Media->BlockSize;
    bounce_buffer = new u8[block_size];
}

NvmeDriver::~NvmeDriver() {
    delete[] bounce_buffer;
}

string NvmeDriver::name() {
    return "nvme";
}

usize NvmeDriver::read(usize offset, u8* dest, usize size) {
    if (size == 0) {
        return 0;
    }

    usize bytes_read = 0;
    usize current_offset = offset;
    u8* current_dest = dest;

    usize prefix_offset = current_offset % block_size;
    if (prefix_offset != 0) {
        usize prefix_size = block_size - prefix_offset;
        if (prefix_size > size - bytes_read) {
            prefix_size = size - bytes_read;
        }

        usize lba = current_offset / block_size;
        EFI_STATUS status = block_io->ReadBlocks(block_io, media_id, lba, block_size, bounce_buffer);
        if (static_cast<INTN>(status) < 0) {
            return bytes_read;
        }

        for (usize i = 0; i < prefix_size; ++i) {
            current_dest[i] = bounce_buffer[prefix_offset + i];
        }

        bytes_read += prefix_size;
        current_offset += prefix_size;
        current_dest += prefix_size;
    }

    usize aligned_blocks = (size - bytes_read) / block_size;
    if (aligned_blocks > 0) {
        usize read_size = aligned_blocks * block_size;
        usize lba = current_offset / block_size;
        EFI_STATUS status = block_io->ReadBlocks(block_io, media_id, lba, read_size, current_dest);
        if (static_cast<INTN>(status) < 0) {
            return bytes_read;
        }

        bytes_read += read_size;
        current_offset += read_size;
        current_dest += read_size;
    }

    usize suffix_size = size - bytes_read;
    if (suffix_size > 0) {
        usize lba = current_offset / block_size;
        EFI_STATUS status = block_io->ReadBlocks(block_io, media_id, lba, block_size, bounce_buffer);
        if (static_cast<INTN>(status) < 0) {
            return bytes_read;
        }

        for (usize i = 0; i < suffix_size; ++i) {
            current_dest[i] = bounce_buffer[i];
        }

        bytes_read += suffix_size;
    }

    return bytes_read;
}

usize NvmeDriver::write(usize offset, const u8* src, usize size) {
    if (size == 0) {
        return 0;
    }

    usize bytes_written = 0;
    usize current_offset = offset;
    const u8* current_src = src;

    usize prefix_offset = current_offset % block_size;
    if (prefix_offset != 0) {
        usize prefix_size = block_size - prefix_offset;
        if (prefix_size > size - bytes_written) {
            prefix_size = size - bytes_written;
        }

        usize lba = current_offset / block_size;
        EFI_STATUS status = block_io->ReadBlocks(block_io, media_id, lba, block_size, bounce_buffer);
        if (static_cast<INTN>(status) < 0) {
            return bytes_written;
        }

        for (usize i = 0; i < prefix_size; ++i) {
            bounce_buffer[prefix_offset + i] = current_src[i];
        }

        status = block_io->WriteBlocks(block_io, media_id, lba, block_size, bounce_buffer);
        if (static_cast<INTN>(status) < 0) {
            return bytes_written;
        }

        bytes_written += prefix_size;
        current_offset += prefix_size;
        current_src += prefix_size;
    }

    usize aligned_blocks = (size - bytes_written) / block_size;
    if (aligned_blocks > 0) {
        usize write_size = aligned_blocks * block_size;
        usize lba = current_offset / block_size;
        EFI_STATUS status = block_io->WriteBlocks(block_io, media_id, lba, write_size, const_cast<u8*>(current_src));
        if (static_cast<INTN>(status) < 0) {
            return bytes_written;
        }

        bytes_written += write_size;
        current_offset += write_size;
        current_src += write_size;
    }

    usize suffix_size = size - bytes_written;
    if (suffix_size > 0) {
        usize lba = current_offset / block_size;
        EFI_STATUS status = block_io->ReadBlocks(block_io, media_id, lba, block_size, bounce_buffer);
        if (static_cast<INTN>(status) < 0) {
            return bytes_written;
        }

        for (usize i = 0; i < suffix_size; ++i) {
            bounce_buffer[i] = current_src[i];
        }

        status = block_io->WriteBlocks(block_io, media_id, lba, block_size, bounce_buffer);
        if (static_cast<INTN>(status) < 0) {
            return bytes_written;
        }

        bytes_written += suffix_size;
    }

    block_io->FlushBlocks(block_io);

    return bytes_written;
}

export NvmeDriver* init_nvme(EFI_SYSTEM_TABLE* system_table) {
    EFI_GUID block_io_guid = EFI_BLOCK_IO_PROTOCOL_GUID;
    EFI_GUID device_path_guid = EFI_DEVICE_PATH_PROTOCOL_GUID;

    UINTN num_handles = 0;
    EFI_HANDLE* handles = nullptr;

    EFI_STATUS status = system_table->BootServices->LocateHandleBuffer(
        ByProtocol,
        &block_io_guid,
        nullptr,
        &num_handles,
        &handles
    );

    if (static_cast<INTN>(status) < 0 || handles == nullptr) {
        return nullptr;
    }

    EFI_BLOCK_IO* selected_block_io = nullptr;

    for (UINTN i = 0; i < num_handles; ++i) {
        EFI_DEVICE_PATH* dp = nullptr;
        status = system_table->BootServices->OpenProtocol(
            handles[i],
            &device_path_guid,
            reinterpret_cast<void**>(&dp),
            nullptr,
            nullptr,
            EFI_OPEN_PROTOCOL_GET_PROTOCOL
        );

        if (static_cast<INTN>(status) < 0 || dp == nullptr) {
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

            node = reinterpret_cast<EFI_DEVICE_PATH*>(reinterpret_cast<u8*>(node) + node_len);
        }

        if (is_nvme) {
            EFI_BLOCK_IO* block_io = nullptr;
            status = system_table->BootServices->OpenProtocol(
                handles[i],
                &block_io_guid,
                reinterpret_cast<void**>(&block_io),
                nullptr,
                nullptr,
                EFI_OPEN_PROTOCOL_GET_PROTOCOL
            );

            if (static_cast<INTN>(status) >= 0 && block_io != nullptr) {
                if (block_io->Media != nullptr && block_io->Media->MediaPresent) {
                    selected_block_io = block_io;
                    break;
                }
            }
        }
    }

    if (selected_block_io == nullptr) {
        return nullptr;
    }

    return new (nvme_storage) NvmeDriver(selected_block_io);
}
