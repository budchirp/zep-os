module;

#include "runtime/runtime.h"

export module zep.memory.vmm;

import zep.std.types;
import zep.context;
import zep.memory;
import zep.boot.info;

export class PageTable {
  public:
    u64 entries[512] = {0};
};

static PageTable* allocate_page_table() {
    void* raw = kernel_allocate(sizeof(PageTable) + 4095);
    u64 addr = reinterpret_cast<u64>(raw);
    u64 aligned_addr = (addr + 4095) & ~static_cast<u64>(4095);

    auto* table = reinterpret_cast<PageTable*>(aligned_addr);
    for (usize i = 0; i < 512; ++i) {
        table->entries[i] = 0;
    }

    return table;
}

export class PageTableManager {
  private:
    PageTable* pml4 = nullptr;
    static inline PageTable* active_pml4 = nullptr;

  public:
    explicit PageTableManager(PageTable* pml4) : pml4(pml4) {}

    static PageTable* get_active_pml4() {
        return active_pml4;
    }

    static void set_active_pml4(PageTable* table) {
        active_pml4 = table;
    }

    void map_page(u64 virtual_address, u64 physical_address, u64 flags) {
        usize pml4_index = (virtual_address >> 39) & 0x1FF;
        usize pdpt_index = (virtual_address >> 30) & 0x1FF;
        usize pd_index = (virtual_address >> 21) & 0x1FF;
        usize pt_index = (virtual_address >> 12) & 0x1FF;

        if ((pml4->entries[pml4_index] & 1) == 0) {
            auto* new_table = allocate_page_table();
            pml4->entries[pml4_index] = reinterpret_cast<u64>(new_table) | 3 | (flags & 4);
        }
        auto* pdpt = reinterpret_cast<PageTable*>(pml4->entries[pml4_index] & ~static_cast<u64>(0xFFF));

        if ((pdpt->entries[pdpt_index] & 1) == 0) {
            auto* new_table = allocate_page_table();
            pdpt->entries[pdpt_index] = reinterpret_cast<u64>(new_table) | 3 | (flags & 4);
        }
        auto* pd = reinterpret_cast<PageTable*>(pdpt->entries[pdpt_index] & ~static_cast<u64>(0xFFF));

        if ((pd->entries[pd_index] & 1) == 0) {
            auto* new_table = allocate_page_table();
            pd->entries[pd_index] = reinterpret_cast<u64>(new_table) | 3 | (flags & 4);
        }
        auto* pt = reinterpret_cast<PageTable*>(pd->entries[pd_index] & ~static_cast<u64>(0xFFF));

        pt->entries[pt_index] = physical_address | flags;
    }
};

export PageTable* get_active_pml4() {
    return PageTableManager::get_active_pml4();
}

static bool is_safe_ram_type(u32 type) {
    return type == 1 || type == 2 || type == 3 || type == 4 ||
           type == 5 || type == 6 || type == 7 || type == 9 || type == 10;
}

export void init_vmm(BootInfo* boot_info) {
    auto* logger = get_context()->logger;

    logger->log("VMM: Allocating active PML4...");
    auto* pml4 = allocate_page_table();
    PageTableManager::set_active_pml4(pml4);

    PageTableManager manager(pml4);

    logger->log("VMM: Mapping memory map entries...");
    auto* memory_map = get_memory_map();
    for (usize i = 0; i < memory_map->count(); ++i) {
        auto* entry = memory_map->get(i);
        if (entry == nullptr) {
            continue;
        }

        if (!is_safe_ram_type(entry->type)) {
            continue;
        }

        u64 base = entry->physical_start;
        u64 pages = entry->num_pages;

        for (u64 p = 0; p < pages; ++p) {
            u64 addr = base + p * 4096;
            manager.map_page(addr, addr, 3);
        }
    }

    logger->log("VMM: Mapping framebuffer...");
    u64 fb_addr = reinterpret_cast<u64>(boot_info->framebuffer.address);
    u64 fb_size = boot_info->framebuffer.height * boot_info->framebuffer.pitch;
    u64 fb_pages = (fb_size + 4095) / 4096;

    for (u64 p = 0; p < fb_pages; ++p) {
        u64 addr = fb_addr + p * 4096;
        manager.map_page(addr, addr, 3);
    }

    logger->log("VMM: Loading CR3...");
    u64 active_pml4_phys = reinterpret_cast<u64>(pml4);
    __asm__ volatile("mov %0, %%cr3" : : "r"(active_pml4_phys) : "memory");
    logger->log("VMM: CR3 loaded successfully");
}
