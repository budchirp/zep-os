module;
#include "../../boot/limine/info.h"

export module zep.device.memory.memory_map;

import zep.std.types;

export class MemoryMap {
  public:
    LimineInfo::MemoryRange* ranges;
    u64 count;

    explicit MemoryMap(LimineInfo::MemoryRange* ranges, u64 count) : ranges(ranges), count(count) {}

    const LimineInfo::MemoryRange* find_usable_range(u64 min_size) const {
        for (u64 i = 0; i < count; ++i) {
            if (ranges[i].type == LimineInfo::MemoryRange::Type::Usable &&
                ranges[i].length >= min_size) {
                return &ranges[i];
            }
        }
        return nullptr;
    }

    const LimineInfo::MemoryRange* range_containing(u64 address) const {
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
            if (ranges[i].type == LimineInfo::MemoryRange::Type::Usable) {
                total += ranges[i].length;
            }
        }
        return total;
    }
};
