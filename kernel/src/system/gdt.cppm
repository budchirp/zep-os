module;

#include "runtime/runtime.h"

export module zep.system.gdt;

import zep.std.types;

export class [[gnu::packed]] GdtEntry {
  public:
    u16 limit_low = 0;
    u16 base_low = 0;
    u8 base_mid = 0;
    u8 access = 0;
    u8 flags = 0;
    u8 base_high = 0;
};

export class [[gnu::packed]] TssEntry {
  public:
    u16 limit_low = 0;
    u16 base_low = 0;
    u8 base_mid = 0;
    u8 access = 0;
    u8 flags = 0;
    u8 base_high = 0;
    u32 base_upper = 0;
    u32 reserved = 0;
};

export class [[gnu::packed]] GdtTable {
  public:
    GdtEntry null_desc{};
    GdtEntry kernel_code{};
    GdtEntry kernel_data{};
    GdtEntry user_code{};
    GdtEntry user_data{};
    TssEntry tss_desc{};
};

export class [[gnu::packed]] GdtDescriptor {
  public:
    u16 limit = 0;
    u64 base = 0;
};

export class [[gnu::packed]] TaskStateSegment {
  public:
    u32 reserved0 = 0;
    u64 rsp0 = 0;
    u64 rsp1 = 0;
    u64 rsp2 = 0;
    u64 reserved1 = 0;
    u64 ist[7] = {0};
    u64 reserved2 = 0;
    u16 reserved3 = 0;
    u16 iopb_offset = 0;
};

alignas(16) static GdtTable gdt_table;
alignas(16) static TaskStateSegment tss;
static GdtDescriptor gdtr;
static u8 tss_stack[16384];

export class GdtManager {
  public:
    static void init() {
        gdt_table.kernel_code.access = 0x9A;
        gdt_table.kernel_code.flags = 0x20;

        gdt_table.kernel_data.access = 0x92;

        gdt_table.user_code.access = 0xFA;
        gdt_table.user_code.flags = 0x20;

        gdt_table.user_data.access = 0xF2;

        u64 tss_base = reinterpret_cast<u64>(&tss);
        u32 tss_limit = sizeof(TaskStateSegment) - 1;

        tss.rsp0 = reinterpret_cast<u64>(tss_stack) + sizeof(tss_stack);

        gdt_table.tss_desc.limit_low = static_cast<u16>(tss_limit & 0xFFFF);
        gdt_table.tss_desc.base_low = static_cast<u16>(tss_base & 0xFFFF);
        gdt_table.tss_desc.base_mid = static_cast<u8>((tss_base >> 16) & 0xFF);
        gdt_table.tss_desc.access = 0x89;
        gdt_table.tss_desc.flags = 0x00;
        gdt_table.tss_desc.base_high = static_cast<u8>((tss_base >> 24) & 0xFF);
        gdt_table.tss_desc.base_upper = static_cast<u32>((tss_base >> 32) & 0xFFFFFFFF);

        gdtr.limit = sizeof(GdtTable) - 1;
        gdtr.base = reinterpret_cast<u64>(&gdt_table);

        __asm__ volatile("lgdt %0" : : "m"(gdtr));

        __asm__ volatile(
            "mov $0x10, %%ax\n"
            "mov %%ax, %%ds\n"
            "mov %%ax, %%es\n"
            "mov %%ax, %%fs\n"
            "mov %%ax, %%gs\n"
            "mov %%ax, %%ss\n"
            "pushq $0x08\n"
            "leaq 1f(%%rip), %%rax\n"
            "pushq %%rax\n"
            "lretq\n"
            "1:\n"
            : : : "rax", "memory"
        );

        __asm__ volatile("ltr %%ax" : : "a"(static_cast<u16>(0x28)));
    }
};
