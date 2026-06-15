module;
#include "../boot/info.h"

export module zep.memory.memory_map;

import zep.std.types;

export namespace zep::memory {

class MemoryMap {
  public:
    BootInfo::MemoryRange* ranges;
    u64 count;

    explicit MemoryMap(BootInfo::MemoryRange* ranges, u64 count) : ranges(ranges), count(count) {}

    const BootInfo::MemoryRange* find_usable_range(u64 min_size) const {
        for (u64 i = 0; i < count; ++i) {
            if (ranges[i].type == BootInfo::MemoryRange::Type::Usable &&
                ranges[i].length >= min_size) {
                return &ranges[i];
            }
        }
        return nullptr;
    }

    const BootInfo::MemoryRange* range_containing(u64 address) const {
        for (u64 i = 0; i < count; ++i) {
            if (address >= ranges[i].base && address < ranges[i].base + ranges[i].length) {
                return &ranges[i];
            }
        }
        return nullptr;
    }

    u64 usable_bytes() const {
        u64 total = 0;
        for (u64 i = 0; i < count; ++i) {
            if (ranges[i].type == BootInfo::MemoryRange::Type::Usable) {
                total += ranges[i].length;
            }
        }
        return total;
    }
};

} // namespace zep::memory
