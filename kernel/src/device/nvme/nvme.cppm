module;

#include "runtime/runtime.h"

export module zep.device.nvme;

import zep.device;
import zep.std.types;
import zep.std.string_view;

export class NvmeDriver : public Device {
  private:
    u8* ram_disk = nullptr;
    usize ram_disk_size = 0;
    usize block_size = 512;
    u8* bounce_buffer = nullptr;

  public:
    explicit NvmeDriver(void* ram_disk, usize ram_disk_size)
        : ram_disk(reinterpret_cast<u8*>(ram_disk)), ram_disk_size(ram_disk_size) {
        bounce_buffer = new u8[block_size];
    }

    ~NvmeDriver() override { delete[] bounce_buffer; }

    StringView name() override { return StringView("nvme"); }

    usize read(usize offset, u8* dest, usize size) override {
        if (size == 0 || ram_disk == nullptr) {
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
            usize read_offset = lba * block_size;
            if (read_offset + block_size > ram_disk_size) {
                return bytes_read;
            }

            for (usize i = 0; i < block_size; ++i) {
                bounce_buffer[i] = ram_disk[read_offset + i];
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
            usize read_offset = lba * block_size;
            if (read_offset + read_size > ram_disk_size) {
                return bytes_read;
            }

            for (usize i = 0; i < read_size; ++i) {
                current_dest[i] = ram_disk[read_offset + i];
            }

            bytes_read += read_size;
            current_offset += read_size;
            current_dest += read_size;
        }

        usize suffix_size = size - bytes_read;
        if (suffix_size > 0) {
            usize lba = current_offset / block_size;
            usize read_offset = lba * block_size;
            if (read_offset + block_size > ram_disk_size) {
                return bytes_read;
            }

            for (usize i = 0; i < block_size; ++i) {
                bounce_buffer[i] = ram_disk[read_offset + i];
            }

            for (usize i = 0; i < suffix_size; ++i) {
                current_dest[i] = bounce_buffer[i];
            }

            bytes_read += suffix_size;
        }

        return bytes_read;
    }

    usize write(usize offset, const u8* src, usize size) override {
        if (size == 0 || ram_disk == nullptr) {
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
            usize write_offset = lba * block_size;
            if (write_offset + block_size > ram_disk_size) {
                return bytes_written;
            }

            for (usize i = 0; i < block_size; ++i) {
                bounce_buffer[i] = ram_disk[write_offset + i];
            }

            for (usize i = 0; i < prefix_size; ++i) {
                bounce_buffer[prefix_offset + i] = current_src[i];
            }

            for (usize i = 0; i < block_size; ++i) {
                ram_disk[write_offset + i] = bounce_buffer[i];
            }

            bytes_written += prefix_size;
            current_offset += prefix_size;
            current_src += prefix_size;
        }

        usize aligned_blocks = (size - bytes_written) / block_size;
        if (aligned_blocks > 0) {
            usize write_size = aligned_blocks * block_size;
            usize lba = current_offset / block_size;
            usize write_offset = lba * block_size;
            if (write_offset + write_size > ram_disk_size) {
                return bytes_written;
            }

            for (usize i = 0; i < write_size; ++i) {
                ram_disk[write_offset + i] = current_src[i];
            }

            bytes_written += write_size;
            current_offset += write_size;
            current_src += write_size;
        }

        usize suffix_size = size - bytes_written;
        if (suffix_size > 0) {
            usize lba = current_offset / block_size;
            usize write_offset = lba * block_size;
            if (write_offset + block_size > ram_disk_size) {
                return bytes_written;
            }

            for (usize i = 0; i < block_size; ++i) {
                bounce_buffer[i] = ram_disk[write_offset + i];
            }

            for (usize i = 0; i < suffix_size; ++i) {
                bounce_buffer[i] = current_src[i];
            }

            for (usize i = 0; i < block_size; ++i) {
                ram_disk[write_offset + i] = bounce_buffer[i];
            }

            bytes_written += suffix_size;
        }

        return bytes_written;
    }
};

alignas(NvmeDriver) static unsigned char nvme_storage[sizeof(NvmeDriver)];

export NvmeDriver* init_nvme(void* ram_disk, usize ram_disk_size) {
    if (ram_disk == nullptr) {
        return nullptr;
    }

    return new (nvme_storage) NvmeDriver(ram_disk, ram_disk_size);
}
