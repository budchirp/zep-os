module;
#include "../boot/info.h"

export module zep.memory.physical_memory;

import zep.std.types;
import zep.std;
import zep.memory.memory_map;

export namespace zep::memory {

class PhysicalMemory {
    u64* bitmap;
    u64 total_pages;
    u64 free_pages;
    u64 base_address;

    static constexpr u64 page_size = 4096;
    static constexpr u64 bits_per_entry = 64;

    void set_bit(u64 index, bool used) {
        u64 entry = index / bits_per_entry;
        u64 bit = index % bits_per_entry;

        if (used) {
            bitmap[entry] |= (static_cast<u64>(1) << bit);
        } else {
            bitmap[entry] &= ~(static_cast<u64>(1) << bit);
        }
    }

    bool get_bit(u64 index) const {
        u64 entry = index / bits_per_entry;
        u64 bit = index % bits_per_entry;
        return (bitmap[entry] & (static_cast<u64>(1) << bit)) != 0;
    }

    u64 find_first_free() const {
        for (u64 entry = 0; entry < (total_pages + bits_per_entry - 1) / bits_per_entry; ++entry) {
            if (bitmap[entry] != ~static_cast<u64>(0)) {
                for (u64 bit = 0; bit < bits_per_entry; ++bit) {
                    if ((bitmap[entry] & (static_cast<u64>(1) << bit)) == 0) {
                        u64 index = entry * bits_per_entry + bit;
                        if (index < total_pages) {
                            return index;
                        }
                    }
                }
            }
        }

        panic("PhysicalMemory: find_first_free called with no free pages");
    }

  public:
    explicit PhysicalMemory() : bitmap(nullptr), total_pages(0), free_pages(0), base_address(0) {}

    bool init(MemoryMap& memory_map) {
        auto* first_range = memory_map.find_usable_range(page_size * 2);
        if (first_range == nullptr) {
            return false;
        }

        auto usable = memory_map.usable_bytes();
        total_pages = usable / page_size;
        u64 bitmap_size_bytes = (total_pages + bits_per_entry - 1) / bits_per_entry * sizeof(u64);
        u64 bitmap_size_pages = (bitmap_size_bytes + page_size - 1) / page_size;

        base_address = first_range->base + bitmap_size_pages * page_size;
        total_pages -= bitmap_size_pages;

        bitmap = reinterpret_cast<u64*>(first_range->base);

        for (u64 i = 0; i < (bitmap_size_bytes + sizeof(u64) - 1) / sizeof(u64); ++i) {
            bitmap[i] = 0;
        }

        free_pages = total_pages;

        auto* ranges = memory_map.ranges;
        auto count = memory_map.count;

        for (u64 i = 0; i < count; ++i) {
            if (ranges[i].type != BootInfo::MemoryRange::Type::Usable) {
                mark_used(ranges[i].base, ranges[i].length);
            }
        }

        mark_used(reinterpret_cast<u64>(bitmap), bitmap_size_bytes);

        return true;
    }

    u64 alloc_page() {
        if (free_pages == 0) {
            panic("PhysicalMemory: out of memory");
        }

        u64 index = find_first_free();
        set_bit(index, true);
        --free_pages;

        return base_address + index * page_size;
    }

    void free_page(u64 address) {
        if (address < base_address) {
            return;
        }

        u64 index = (address - base_address) / page_size;
        if (index >= total_pages) {
            return;
        }

        if (get_bit(index)) {
            set_bit(index, false);
            ++free_pages;
        }
    }

    void mark_used(u64 address, u64 length) {
        u64 start_page = 0;
        u64 end_address = address + length;
        u64 aligned_start = (address + page_size - 1) / page_size * page_size;

        if (aligned_start < base_address) {
            start_page = 0;
        } else {
            start_page = (aligned_start - base_address) / page_size;
        }

        if (end_address <= base_address) {
            return;
        }

        u64 end_page = (end_address - base_address) / page_size;
        if (end_page > total_pages) {
            end_page = total_pages;
        }

        for (u64 i = start_page; i < end_page; ++i) {
            if (!get_bit(i)) {
                set_bit(i, true);
                --free_pages;
            }
        }
    }
};

} // namespace zep::memory
