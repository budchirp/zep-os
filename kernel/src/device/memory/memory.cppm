module;

#include "runtime/runtime.h"

#include <efi.h>

export module zep.memory;

import zep.std.types;
import zep.device;

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

class MemoryBlock {
  private:
  public:
    usize size = 0;
    bool is_free = true;
    MemoryBlock* next = nullptr;
    MemoryBlock* prev = nullptr;
};

export class HeapAllocator {
  private:
    MemoryBlock* head = nullptr;

  public:
    explicit HeapAllocator(void* memory_start, usize memory_size) {
        if (memory_start != nullptr && memory_size > sizeof(MemoryBlock)) {
            head = reinterpret_cast<MemoryBlock*>(memory_start);
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

        MemoryBlock* curr = head;

        while (curr != nullptr) {
            if (curr->is_free && curr->size >= aligned_size) {
                if (curr->size >= aligned_size + sizeof(MemoryBlock) + 16) {
                    MemoryBlock* next_block = reinterpret_cast<MemoryBlock*>(
                        reinterpret_cast<u8*>(curr) + sizeof(MemoryBlock) + aligned_size);
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

                return reinterpret_cast<void*>(reinterpret_cast<u8*>(curr) + sizeof(MemoryBlock));
            }

            curr = curr->next;
        }

        return nullptr;
    }

    void free(void* block) {
        if (block == nullptr || head == nullptr) {
            return;
        }

        MemoryBlock* curr =
            reinterpret_cast<MemoryBlock*>(reinterpret_cast<u8*>(block) - sizeof(MemoryBlock));
        curr->is_free = true;

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

export void init_heap(void* memory_start, usize memory_size) {
    global_heap = new (heap_allocator_storage) HeapAllocator(memory_start, memory_size);
}

export HeapAllocator* get_heap() {
    return global_heap;
}

export extern "C" void* kernel_allocate(usize size) {
    if (global_heap == nullptr) {
        return nullptr;
    }

    return global_heap->allocate(size);
}

export extern "C" void kernel_deallocate(void* block) {
    if (global_heap == nullptr || block == nullptr) {
        return;
    }

    global_heap->free(block);
}

export class MemoryDevice : public Device {
  private:
    u8* buffer = nullptr;
    usize buffer_size = 0;

  public:
    explicit MemoryDevice(u8* buffer, usize buffer_size)
        : buffer(buffer), buffer_size(buffer_size) {}

    ~MemoryDevice() override = default;

    string name() override { return "ram0"; }

    u8* get_buffer() { return buffer; }

    usize get_size() const { return buffer_size; }

    usize read(usize offset, u8* dest, usize size) override {
        if (offset >= buffer_size || dest == nullptr) {
            return 0;
        }

        usize bytes_to_read = size;

        if (offset + size > buffer_size) {
            bytes_to_read = buffer_size - offset;
        }

        for (usize i = 0; i < bytes_to_read; ++i) {
            dest[i] = buffer[offset + i];
        }

        return bytes_to_read;
    }

    usize write(usize offset, const u8* src, usize size) override {
        if (offset >= buffer_size || src == nullptr) {
            return 0;
        }

        usize bytes_to_write = size;

        if (offset + size > buffer_size) {
            bytes_to_write = buffer_size - offset;
        }

        for (usize i = 0; i < bytes_to_write; ++i) {
            buffer[offset + i] = src[i];
        }

        return bytes_to_write;
    }
};
