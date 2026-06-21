module;

#include "runtime/runtime.h"

#include <efi.h>

export module zep.memory;

import zep.std.types;
import zep.device;

extern "C" [[noreturn]] void panic(string str);

export class PageAllocator {
  private:
    EFI_BOOT_SERVICES* boot_services = nullptr;

  public:
    explicit PageAllocator(EFI_BOOT_SERVICES* boot_services) : boot_services(boot_services) {}

    void* allocate_pages(usize count) {
        if (boot_services == nullptr) {
            return nullptr;
        }

        EFI_PHYSICAL_ADDRESS physical_address = 0;

        EFI_STATUS status =
            boot_services->AllocatePages(AllocateAnyPages, EfiLoaderData, count, &physical_address);

        if (status != EFI_SUCCESS) {
            return nullptr;
        }

        return reinterpret_cast<void*>(physical_address);
    }

    void free_pages(void* block, usize count) {
        if (boot_services == nullptr || block == nullptr) {
            return;
        }

        boot_services->FreePages(reinterpret_cast<EFI_PHYSICAL_ADDRESS>(block), count);
    }
};

constexpr u32 BLOCK_MAGIC_ALLOCATED = 0xA110CA7E;
constexpr u32 BLOCK_MAGIC_FREE = 0xFEEDFACE;
constexpr usize HEAP_GROWTH_PAGES = 256;

class alignas(16) MemoryBlock {
  public:
    u32 magic = 0;
    usize size = 0;
    bool is_free = true;
    MemoryBlock* next = nullptr;
    MemoryBlock* prev = nullptr;
};

export class HeapAllocator {
  private:
    MemoryBlock* head = nullptr;
    PageAllocator* page_allocator = nullptr;

    MemoryBlock* find_tail() {
        MemoryBlock* curr = head;

        while (curr != nullptr && curr->next != nullptr) {
            curr = curr->next;
        }

        return curr;
    }

    bool grow() {
        if (page_allocator == nullptr) {
            return false;
        }

        void* new_region = page_allocator->allocate_pages(HEAP_GROWTH_PAGES);
        if (new_region == nullptr) {
            return false;
        }

        usize region_size = HEAP_GROWTH_PAGES * 4096;

        auto* new_block = reinterpret_cast<MemoryBlock*>(new_region);
        new_block->magic = BLOCK_MAGIC_FREE;
        new_block->size = region_size - sizeof(MemoryBlock);
        new_block->is_free = true;
        new_block->next = nullptr;
        new_block->prev = nullptr;

        MemoryBlock* tail = find_tail();
        if (tail != nullptr) {
            tail->next = new_block;
            new_block->prev = tail;
        } else {
            head = new_block;
        }

        return true;
    }

  public:
    explicit HeapAllocator(void* memory_start, usize memory_size, PageAllocator* page_allocator)
        : page_allocator(page_allocator) {
        if (memory_start != nullptr && memory_size > sizeof(MemoryBlock)) {
            head = reinterpret_cast<MemoryBlock*>(memory_start);
            head->magic = BLOCK_MAGIC_FREE;
            head->size = memory_size - sizeof(MemoryBlock);
            head->is_free = true;
            head->next = nullptr;
            head->prev = nullptr;
        }
    }

    void* allocate(usize size) {
        if (head == nullptr || size == 0) {
            return nullptr;
        }

        usize aligned_size = (size + 15) & ~static_cast<usize>(15);

        for (usize attempt = 0; attempt < 2; ++attempt) {
            MemoryBlock* curr = head;

            while (curr != nullptr) {
                if (curr->is_free && curr->size >= aligned_size) {
                    if (curr->size >= aligned_size + sizeof(MemoryBlock) + 16) {
                        auto* next_block = reinterpret_cast<MemoryBlock*>(
                            reinterpret_cast<u8*>(curr) + sizeof(MemoryBlock) + aligned_size);
                        next_block->magic = BLOCK_MAGIC_FREE;
                        next_block->size = curr->size - aligned_size - sizeof(MemoryBlock);
                        next_block->is_free = true;
                        next_block->next = curr->next;
                        next_block->prev = curr;

                        if (curr->next != nullptr) {
                            curr->next->prev = next_block;
                        }

                        curr->next = next_block;
                        curr->size = aligned_size;
                    }

                    curr->is_free = false;
                    curr->magic = BLOCK_MAGIC_ALLOCATED;

                    return reinterpret_cast<void*>(reinterpret_cast<u8*>(curr) +
                                                   sizeof(MemoryBlock));
                }

                curr = curr->next;
            }

            if (attempt == 0 && !grow()) {
                return nullptr;
            }
        }

        return nullptr;
    }

    void free(void* block) {
        if (block == nullptr || head == nullptr) {
            return;
        }

        auto* curr =
            reinterpret_cast<MemoryBlock*>(reinterpret_cast<u8*>(block) - sizeof(MemoryBlock));

        if (curr->magic == BLOCK_MAGIC_FREE) {
            panic("double free detected");
        }

        if (curr->magic != BLOCK_MAGIC_ALLOCATED) {
            panic("heap corruption: invalid block magic");
        }

        curr->is_free = true;
        curr->magic = BLOCK_MAGIC_FREE;

        if (curr->next != nullptr && curr->next->is_free) {
            curr->size += sizeof(MemoryBlock) + curr->next->size;
            curr->next = curr->next->next;

            if (curr->next != nullptr) {
                curr->next->prev = curr;
            }
        }

        if (curr->prev != nullptr && curr->prev->is_free) {
            curr->prev->size += sizeof(MemoryBlock) + curr->size;
            curr->prev->next = curr->next;

            if (curr->next != nullptr) {
                curr->next->prev = curr->prev;
            }
        }
    }
};

static HeapAllocator* global_heap = nullptr;

alignas(HeapAllocator) static u8 heap_allocator_storage[sizeof(HeapAllocator)];

export void init_heap(void* memory_start, usize memory_size, PageAllocator* page_allocator) {
    global_heap =
        new (heap_allocator_storage) HeapAllocator(memory_start, memory_size, page_allocator);
}

export HeapAllocator* get_heap() {
    return global_heap;
}

export extern "C" void* kernel_allocate(usize size) {
    if (global_heap == nullptr) {
        panic("kernel_allocate called before heap init");
    }

    void* result = global_heap->allocate(size);
    if (result == nullptr) {
        panic("out of memory");
    }

    return result;
}

export extern "C" void kernel_deallocate(void* block) {
    if (global_heap == nullptr || block == nullptr) {
        return;
    }

    global_heap->free(block);
}

export class MemoryMapEntry {
  public:
    u32 type = 0;
    u64 physical_start = 0;
    u64 num_pages = 0;
    u64 attribute = 0;
};

export class MemoryMap {
  private:
    MemoryMapEntry* entries = nullptr;
    usize entry_count = 0;
    usize map_key = 0;

  public:
    MemoryMap() = default;

    bool capture(EFI_SYSTEM_TABLE* system_table) {
        UINTN map_size = 0;
        UINTN descriptor_size = 0;
        UINT32 descriptor_version = 0;
        UINTN key = 0;

        system_table->BootServices->GetMemoryMap(&map_size, nullptr, &key, &descriptor_size,
                                                 &descriptor_version);

        map_size += 2 * descriptor_size;

        auto* raw_map = new u8[map_size];
        if (raw_map == nullptr) {
            return false;
        }

        EFI_STATUS status = system_table->BootServices->GetMemoryMap(
            &map_size, reinterpret_cast<EFI_MEMORY_DESCRIPTOR*>(raw_map), &key, &descriptor_size,
            &descriptor_version);

        if (status != EFI_SUCCESS) {
            delete[] raw_map;
            return false;
        }

        map_key = key;
        entry_count = map_size / descriptor_size;

        entries = new MemoryMapEntry[entry_count];
        if (entries == nullptr) {
            delete[] raw_map;
            return false;
        }

        for (usize i = 0; i < entry_count; ++i) {
            auto* desc = reinterpret_cast<EFI_MEMORY_DESCRIPTOR*>(raw_map + i * descriptor_size);
            entries[i].type = desc->Type;
            entries[i].physical_start = desc->PhysicalStart;
            entries[i].num_pages = desc->NumberOfPages;
            entries[i].attribute = desc->Attribute;
        }

        delete[] raw_map;

        return true;
    }

    usize count() const { return entry_count; }

    usize key() const { return map_key; }

    MemoryMapEntry* get(usize index) {
        if (index >= entry_count || entries == nullptr) {
            return nullptr;
        }

        return &entries[index];
    }
};

alignas(MemoryMap) static u8 memory_map_storage[sizeof(MemoryMap)];
static MemoryMap* global_memory_map = nullptr;

export MemoryMap* init_memory_map(EFI_SYSTEM_TABLE* system_table) {
    auto* map = new (memory_map_storage) MemoryMap();

    if (!map->capture(system_table)) {
        return nullptr;
    }

    return map;
}

static PageAllocator* global_page_allocator = nullptr;
alignas(PageAllocator) static u8 page_alloc_storage[sizeof(PageAllocator)];

export PageAllocator* init_page_allocator(EFI_BOOT_SERVICES* boot_services) {
    global_page_allocator = new (page_alloc_storage) PageAllocator(boot_services);
    return global_page_allocator;
}

export PageAllocator* get_page_allocator() {
    return global_page_allocator;
}

export MemoryMap* get_memory_map() {
    return global_memory_map;
}

export void init_memory(EFI_SYSTEM_TABLE* system_table) {
    constexpr usize HEAP_PAGES = 4096;
    constexpr usize HEAP_SIZE = HEAP_PAGES * 4096;

    PageAllocator* page_allocator = init_page_allocator(system_table->BootServices);
    if (page_allocator == nullptr) {
        panic("Failed to initialize PageAllocator");
    }

    void* heap_mem = page_allocator->allocate_pages(HEAP_PAGES);
    if (heap_mem == nullptr) {
        panic("Failed to allocate heap memory");
    }

    init_heap(heap_mem, HEAP_SIZE, page_allocator);

    global_memory_map = init_memory_map(system_table);
    if (global_memory_map == nullptr) {
        panic("Failed to capture UEFI memory map");
    }
}


