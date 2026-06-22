module;

#include "runtime/runtime.h"

#include <efi.h>

export module zep.fs.fat;

import zep.std.types;
import zep.std.string_view;
import zep.device;
import zep.fs;

export class [[gnu::packed]] Fat32Bpb {
  public:
    u8 jmp[3] = {0};
    u8 oem_name[8] = {0};
    u16 bytes_per_sector = 0;
    u8 sectors_per_cluster = 0;
    u16 reserved_sectors = 0;
    u8 num_fats = 0;
    u16 root_entries = 0;
    u16 total_sectors_small = 0;
    u8 media_type = 0;
    u16 sectors_per_fat_small = 0;
    u16 sectors_per_track = 0;
    u16 num_heads = 0;
    u32 hidden_sectors = 0;
    u32 total_sectors_large = 0;
    u32 sectors_per_fat_large = 0;
    u16 ext_flags = 0;
    u16 fs_version = 0;
    u32 root_cluster = 0;
    u16 fs_info = 0;
    u16 backup_boot_sector = 0;
    u8 reserved[12] = {0};
    u8 drive_number = 0;
    u8 reserved1 = 0;
    u8 boot_signature = 0;
    u32 volume_id = 0;
    u8 volume_label[11] = {0};
    u8 file_system_type[8] = {0};
};

export class [[gnu::packed]] FatDirEntry {
  public:
    u8 name[8] = {0};
    u8 extension[3] = {0};
    u8 attributes = 0;
    u8 reserved = 0;
    u8 creation_time_tenth = 0;
    u16 creation_time = 0;
    u16 creation_date = 0;
    u16 last_access_date = 0;
    u16 first_cluster_high = 0;
    u16 write_time = 0;
    u16 write_date = 0;
    u16 first_cluster_low = 0;
    u32 file_size = 0;
};

export class Fat32FileSystem : public FileSystemDriver {
  private:
    void parse_filename(StringView path, u8* dest_name, u8* dest_ext) {
        StringView filename = path;

        if (path.starts_with(StringView("/"))) {
            filename = StringView(path.data + 1, path.length() - 1);
        }

        usize dot_index = filename.length();

        for (usize i = 0; i < filename.length(); ++i) {
            if (filename.data[i] == '.') {
                dot_index = i;
                break;
            }
        }

        for (usize i = 0; i < 8; ++i) {
            dest_name[i] = ' ';
        }

        for (usize i = 0; i < 3; ++i) {
            dest_ext[i] = ' ';
        }

        usize name_len = dot_index;

        if (name_len > 8) {
            name_len = 8;
        }

        for (usize i = 0; i < name_len; ++i) {
            dest_name[i] = static_cast<u8>(filename.data[i]);
        }

        if (dot_index < filename.length()) {
            usize ext_len = filename.length() - dot_index - 1;

            if (ext_len > 3) {
                ext_len = 3;
            }

            for (usize i = 0; i < ext_len; ++i) {
                dest_ext[i] = static_cast<u8>(filename.data[dot_index + 1 + i]);
            }
        }
    }

    void format_name(const u8* raw_name, const u8* raw_ext, char* dest_node_name) {
        usize index = 0;
        usize name_len = 8;

        while (name_len > 0 && raw_name[name_len - 1] == ' ') {
            name_len--;
        }

        for (usize i = 0; i < name_len; ++i) {
            dest_node_name[index++] = static_cast<char>(raw_name[i]);
        }

        usize ext_len = 3;

        while (ext_len > 0 && raw_ext[ext_len - 1] == ' ') {
            ext_len--;
        }

        if (ext_len > 0) {
            dest_node_name[index++] = '.';

            for (usize i = 0; i < ext_len; ++i) {
                dest_node_name[index++] = static_cast<char>(raw_ext[i]);
            }
        }

        dest_node_name[index] = '\0';
    }

    bool find_dir_entry(StringView name, FatDirEntry& out_entry, usize& out_sector,
                        usize& out_offset) {
        if (device == nullptr) {
            return false;
        }

        u8 sector_buffer[512];
        u32 current_cluster = bpb.root_cluster;

        while (current_cluster < 0x0FFFFFF8) {
            for (usize sector_offset = 0; sector_offset < sectors_per_cluster; ++sector_offset) {
                usize current_sector =
                    data_start_sector + (current_cluster - 2) * sectors_per_cluster + sector_offset;

                if (device->read(current_sector * bytes_per_sector, sector_buffer,
                                 bytes_per_sector) != bytes_per_sector) {
                    return false;
                }

                auto* entries = reinterpret_cast<FatDirEntry*>(sector_buffer);
                usize entries_per_sector = bytes_per_sector / sizeof(FatDirEntry);

                for (usize i = 0; i < entries_per_sector; ++i) {
                    if (entries[i].name[0] == 0x00) {
                        return false;
                    }

                    if (entries[i].name[0] == 0xE5) {
                        continue;
                    }

                    char node_name[16] = {0};
                    format_name(entries[i].name, entries[i].extension, node_name);

                    if (StringView(node_name).equals(name)) {
                        out_entry = entries[i];
                        out_sector = current_sector;
                        out_offset = i * sizeof(FatDirEntry);
                        return true;
                    }
                }
            }

            current_cluster = read_fat_entry(current_cluster);
        }

        return false;
    }

  public:
    Device* device = nullptr;
    Fat32Bpb bpb;
    usize fat_start_sector = 1;
    usize data_start_sector = 0;
    usize sectors_per_cluster = 1;
    usize bytes_per_sector = 512;

    explicit Fat32FileSystem(Device* device) : device(device) {}
    ~Fat32FileSystem() override = default;

    bool init() {
        if (device == nullptr) {
            return false;
        }

        u8 sector_buffer[512];

        if (device->read(0, sector_buffer, 512) != 512) {
            return false;
        }

        auto* raw_bpb = reinterpret_cast<Fat32Bpb*>(sector_buffer);
        bpb = *raw_bpb;

        bytes_per_sector = bpb.bytes_per_sector;
        sectors_per_cluster = bpb.sectors_per_cluster;

        fat_start_sector = bpb.reserved_sectors;
        data_start_sector = fat_start_sector + bpb.num_fats * bpb.sectors_per_fat_large;

        return true;
    }

    u32 read_fat_entry(u32 cluster) {
        if (device == nullptr) {
            return 0x0FFFFFFF;
        }

        usize fat_offset = cluster * 4;
        usize sector = fat_start_sector + (fat_offset / bytes_per_sector);
        usize offset_in_sector = fat_offset % bytes_per_sector;

        u8 sector_buffer[512];

        if (device->read(sector * bytes_per_sector, sector_buffer, 512) != 512) {
            return 0x0FFFFFFF;
        }

        return (*reinterpret_cast<u32*>(sector_buffer + offset_in_sector)) & 0x0FFFFFFF;
    }

    bool write_fat_entry(u32 cluster, u32 value) {
        if (device == nullptr) {
            return false;
        }

        usize fat_offset = cluster * 4;
        usize sector = fat_start_sector + (fat_offset / bytes_per_sector);
        usize offset_in_sector = fat_offset % bytes_per_sector;

        u8 sector_buffer[512];

        if (device->read(sector * bytes_per_sector, sector_buffer, 512) != 512) {
            return false;
        }

        u32 original = *reinterpret_cast<u32*>(sector_buffer + offset_in_sector);
        u32 updated = (original & 0xF0000000) | (value & 0x0FFFFFFF);
        *reinterpret_cast<u32*>(sector_buffer + offset_in_sector) = updated;

        return device->write(sector * bytes_per_sector, sector_buffer, 512) == 512;
    }

    usize read_file(StringView path, usize offset, u8* buffer, usize size) override {
        StringView filename = path;

        if (path.starts_with(StringView("/"))) {
            filename = StringView(path.data + 1, path.length() - 1);
        }

        FatDirEntry entry;
        usize dir_sector = 0;
        usize dir_offset = 0;

        if (!find_dir_entry(filename, entry, dir_sector, dir_offset)) {
            return 0;
        }

        if ((entry.attributes & 0x10) != 0 || offset >= entry.file_size || size == 0 ||
            buffer == nullptr) {
            return 0;
        }

        usize bytes_to_read = size;

        if (offset + size > entry.file_size) {
            bytes_to_read = entry.file_size - offset;
        }

        usize cluster_size = sectors_per_cluster * bytes_per_sector;
        u32 current_cluster =
            (static_cast<u32>(entry.first_cluster_high) << 16) | entry.first_cluster_low;

        usize skip_clusters = offset / cluster_size;

        for (usize i = 0; i < skip_clusters; ++i) {
            current_cluster = read_fat_entry(current_cluster);

            if (current_cluster >= 0x0FFFFFF8) {
                return 0;
            }
        }

        usize start_offset_in_cluster = offset % cluster_size;
        usize bytes_read = 0;

        while (bytes_read < bytes_to_read && current_cluster < 0x0FFFFFF8) {
            usize sector = data_start_sector + (current_cluster - 2) * sectors_per_cluster;
            usize read_len = cluster_size - start_offset_in_cluster;

            if (read_len > bytes_to_read - bytes_read) {
                read_len = bytes_to_read - bytes_read;
            }

            if (device->read(sector * bytes_per_sector + start_offset_in_cluster,
                             buffer + bytes_read, read_len) != read_len) {
                break;
            }

            bytes_read += read_len;
            start_offset_in_cluster = 0;

            if (bytes_read < bytes_to_read) {
                current_cluster = read_fat_entry(current_cluster);
            }
        }

        return bytes_read;
    }

    bool create_file(StringView path) override {
        if (device == nullptr) {
            return false;
        }

        StringView filename = path;

        if (path.starts_with(StringView("/"))) {
            filename = StringView(path.data + 1, path.length() - 1);
        }

        FatDirEntry entry;
        usize dir_sector = 0;
        usize dir_offset = 0;

        if (find_dir_entry(filename, entry, dir_sector, dir_offset)) {
            return false;
        }

        u32 free_cluster = 0;

        for (u32 c = 3; c < 65536; ++c) {
            if (read_fat_entry(c) == 0) {
                free_cluster = c;
                break;
            }
        }

        if (free_cluster == 0) {
            return false;
        }

        if (!write_fat_entry(free_cluster, 0x0FFFFFFF)) {
            return false;
        }

        u8 dest_name[8];
        u8 dest_ext[3];
        parse_filename(path, dest_name, dest_ext);

        u8 sector_buffer[512];
        u32 current_cluster = bpb.root_cluster;

        while (current_cluster < 0x0FFFFFF8) {
            for (usize sector_offset = 0; sector_offset < sectors_per_cluster; ++sector_offset) {
                usize current_sector =
                    data_start_sector + (current_cluster - 2) * sectors_per_cluster + sector_offset;

                if (device->read(current_sector * bytes_per_sector, sector_buffer,
                                 bytes_per_sector) != bytes_per_sector) {
                    return false;
                }

                auto* entries = reinterpret_cast<FatDirEntry*>(sector_buffer);
                usize entries_per_sector = bytes_per_sector / sizeof(FatDirEntry);

                for (usize i = 0; i < entries_per_sector; ++i) {
                    if (entries[i].name[0] == 0x00 || entries[i].name[0] == 0xE5) {
                        for (usize k = 0; k < 8; ++k) {
                            entries[i].name[k] = dest_name[k];
                        }

                        for (usize k = 0; k < 3; ++k) {
                            entries[i].extension[k] = dest_ext[k];
                        }

                        entries[i].attributes = 0x20;
                        entries[i].reserved = 0;
                        entries[i].creation_time_tenth = 0;
                        entries[i].creation_time = 0;
                        entries[i].creation_date = 0;
                        entries[i].last_access_date = 0;
                        entries[i].first_cluster_high =
                            static_cast<u16>((free_cluster >> 16) & 0xFFFF);
                        entries[i].write_time = 0;
                        entries[i].write_date = 0;
                        entries[i].first_cluster_low = static_cast<u16>(free_cluster & 0xFFFF);
                        entries[i].file_size = 0;

                        return device->write(current_sector * bytes_per_sector, sector_buffer,
                                             bytes_per_sector) == bytes_per_sector;
                    }
                }
            }

            u32 next_cluster = read_fat_entry(current_cluster);
            if (next_cluster >= 0x0FFFFFF8) {
                break;
            }
            current_cluster = next_cluster;
        }

        return false;
    }

    bool delete_file(StringView path) override {
        if (device == nullptr) {
            return false;
        }

        StringView filename = path;

        if (path.starts_with(StringView("/"))) {
            filename = StringView(path.data + 1, path.length() - 1);
        }

        FatDirEntry entry;
        usize dir_sector = 0;
        usize dir_offset = 0;

        if (!find_dir_entry(filename, entry, dir_sector, dir_offset)) {
            return false;
        }

        u32 curr_cluster =
            (static_cast<u32>(entry.first_cluster_high) << 16) | entry.first_cluster_low;

        while (curr_cluster > 0 && curr_cluster < 0x0FFFFFF8) {
            u32 next_cluster = read_fat_entry(curr_cluster);
            write_fat_entry(curr_cluster, 0x00000000);
            curr_cluster = next_cluster;
        }

        u8 sector_buffer[512];

        if (device->read(dir_sector * bytes_per_sector, sector_buffer, bytes_per_sector) ==
            bytes_per_sector) {
            auto* raw_entry = reinterpret_cast<FatDirEntry*>(&sector_buffer[dir_offset]);
            raw_entry->name[0] = 0xE5;
            return device->write(dir_sector * bytes_per_sector, sector_buffer, bytes_per_sector) ==
                   bytes_per_sector;
        }

        return false;
    }
};
