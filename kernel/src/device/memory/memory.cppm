module;
#include "../../std/runtime.h"
#include "boot/limine/info.h"

export module zep.device.memory;

import zep.std.types;
import zep.device.memory.memory_map;
import zep.device.memory.physical_memory;
import zep.device;

export class MemoryDevice : Device<void*> {
  private:
    LimineInfo& info;

  public:
    MemoryMap* memory_map;
    PhysicalMemory* physical_memory;

    explicit MemoryDevice(LimineInfo& info)
        : info(info), memory_map(nullptr), physical_memory(nullptr) {}

    void* init() override {
        if (info.memory_map == nullptr || info.memory_map_count == 0) {
            return nullptr;
        }

        memory_map = new MemoryMap(info.memory_map, info.memory_map_count);

        physical_memory = new PhysicalMemory();
        physical_memory->init(*memory_map);

        return nullptr;
    }
};
