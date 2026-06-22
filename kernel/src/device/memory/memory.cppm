module;

#include "runtime/runtime.h"

export module zep.memory;

import zep.std.types;
import zep.device;
import zep.boot.info;

extern "C" [[noreturn]] void panic(string str);

export class PageAllocator {
  private:
    void* (*alloc_cb)(usize) = nullptr;
    void (*free_cb)(void*, usize) = nullptr;

  public:
    explicit PageAllocator(void* (*alloc_cb)(usize), void (*free_cb)(void*, usize))
        : alloc_cb(alloc_cb), free_cb(free_cb) {}

    void* allocate_pages(usize count) {
        if (alloc_cb == nullptr) {
            return nullptr;
        }
        return alloc_cb(count);
    }

    void free_pages(void* block, usize count) {
        if (free_cb == nullptr || block == nullptr) {
            return;
        }
        free_cb(block, count);
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

export class MemoryMap {
  private:
    MemoryMapEntry* entries = nullptr;
    usize entry_count = 0;
    usize map_key = 0;

  public:
    MemoryMap() = default;

    void init(MemoryMapEntry* entries_ptr, usize count, usize key) {
        entries = entries_ptr;
        entry_count = count;
        map_key = key;
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

alignas(HeapAllocator) static u8 heap_allocator_storage[sizeof(HeapAllocator)];
alignas(MemoryMap) static u8 memory_map_storage[sizeof(MemoryMap)];
alignas(PageAllocator) static u8 page_alloc_storage[sizeof(PageAllocator)];

export HeapAllocator* get_heap() {
    return reinterpret_cast<HeapAllocator*>(heap_allocator_storage);
}

export extern "C" void* kernel_allocate(usize size) {
    auto* heap = get_heap();

    void* result = heap->allocate(size);

    if (result == nullptr) {
        panic("out of memory");
    }

    return result;
}

export extern "C" void kernel_deallocate(void* block) {
    auto* heap = get_heap();
    if (block == nullptr) {
        return;
    }

    heap->free(block);
}

export MemoryMap* get_memory_map() {
    return reinterpret_cast<MemoryMap*>(memory_map_storage);
}

export PageAllocator* get_page_allocator() {
    return reinterpret_cast<PageAllocator*>(page_alloc_storage);
}

export void init_memory(BootInfo* boot_info) {
    new (page_alloc_storage) PageAllocator(boot_info->pages.allocate, boot_info->pages.free);

    if (boot_info->heap.memory == nullptr) {
        panic("Failed to get heap memory from loader");
    }

    new (heap_allocator_storage) HeapAllocator(boot_info->heap.memory, boot_info->heap.size, get_page_allocator());

    auto* memory_map = new (memory_map_storage) MemoryMap();
    memory_map->init(boot_info->memory_map.entries, boot_info->memory_map.count,
                     boot_info->memory_map.key);
}
