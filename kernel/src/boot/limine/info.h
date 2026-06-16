#pragma once

#include <stddef.h>
#include <stdint.h>

class LimineInfo {
  public:
    class Framebuffer {
      public:
        void* address;
        uint64_t width;
        uint64_t height;
        uint64_t pitch;
        uint16_t bpp;
    };

    class MemoryRange {
      public:
        uint64_t base;
        uint64_t length;
        enum class Type : uint32_t {
            Usable,
            Reserved,
            AcpiReclaimable,
            AcpiNvs,
            BadMemory,
            BootloaderReclaimable,
            KernelModule,
            Framebuffer,
        };
        Type type;
    };

    static LimineInfo get();

    explicit LimineInfo()
        : framebuffer{}, memory_map(nullptr), memory_map_count(0), cmdline(nullptr), rsdp(nullptr),
          dtb(nullptr), hhdm_offset(0) {}

    Framebuffer framebuffer;
    MemoryRange* memory_map;
    uint64_t memory_map_count;
    const char* cmdline;
    void* rsdp;
    void* dtb;
    uint64_t hhdm_offset;
};
