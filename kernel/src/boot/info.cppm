module;

export module zep.boot.info;

import zep.std.types;

export class MemoryMapEntry {
  public:
    u32 type = 0;
    u64 physical_start = 0;
    u64 num_pages = 0;
    u64 attribute = 0;
};

export class BootFramebuffer {
  public:
    void* address = nullptr;
    u64 width = 0;
    u64 height = 0;
    u64 pitch = 0;
    u16 bpp = 32;
};

export class BootMemoryMap {
  public:
    MemoryMapEntry* entries = nullptr;
    usize count = 0;
    usize key = 0;
};

export class BootHeap {
  public:
    void* memory = nullptr;
    usize size = 0;
};

export class BootPages {
  public:
    void* (*allocate)(usize count) = nullptr;
    void (*free)(void* block, usize count) = nullptr;
};

export class BootDisk {
  public:
    void* address = nullptr;
    usize size = 0;
};

export class BootInfo {
  public:
    BootFramebuffer framebuffer;
    BootMemoryMap memory_map;
    BootHeap heap;
    BootPages pages;

    BootDisk* disks = nullptr;
    usize disk_count = 0;

    void* console = nullptr;
};
