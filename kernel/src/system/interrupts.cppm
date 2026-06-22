module;

#include "runtime/runtime.h"

export module zep.system.interrupts;

import zep.std.types;
import zep.std.string_view;
import zep.std;
import zep.arch;
import zep.device.keyboard;

export class [[gnu::packed]] IdtEntry {
  public:
    u16 offset_low = 0;
    u16 selector = 0x08;
    u8 ist = 0;
    u8 type_attributes = 0;
    u16 offset_mid = 0;
    u32 offset_high = 0;
    u32 reserved = 0;

    void set_offset(u64 offset) {
        offset_low = static_cast<u16>(offset & 0xFFFF);
        offset_mid = static_cast<u16>((offset >> 16) & 0xFFFF);
        offset_high = static_cast<u32>((offset >> 32) & 0xFFFFFFFF);
    }
};

export class [[gnu::packed]] IdtDescriptor {
  public:
    u16 limit = 0;
    u64 base = 0;
};

export class InterruptFrame {
  public:
    u64 ip = 0;
    u64 cs = 0;
    u64 flags = 0;
    u64 sp = 0;
    u64 ss = 0;
};

alignas(16) static IdtEntry idt[256];
static IdtDescriptor idtr;

static void remap_pic() {
    outb(0x20, 0x11);
    outb(0xA0, 0x11);

    outb(0x21, 0x20);
    outb(0xA1, 0x28);

    outb(0x21, 0x04);
    outb(0xA1, 0x02);

    outb(0x21, 0x01);
    outb(0xA1, 0x01);

    outb(0x21, 0xFD);
    outb(0xA1, 0xFF);
}

extern "C" __attribute__((interrupt)) void keyboard_handler(InterruptFrame* frame) {
    (void)frame;
    u8 scancode = inb(0x60);
    Keyboard::handle_scancode(scancode);
    outb(0x20, 0x20);
}

static void print_hex(u64 val) {
    char buf[19];
    buf[0] = '0';
    buf[1] = 'x';
    for (int i = 0; i < 16; ++i) {
        u8 nibble = (val >> ((15 - i) * 4)) & 0xF;
        buf[2 + i] = (nibble < 10) ? ('0' + nibble) : ('A' + (nibble - 10));
    }
    buf[18] = '\0';
    print(StringView(buf, 18));
}

extern "C" __attribute__((interrupt)) void page_fault_handler(InterruptFrame* frame,
                                                              u64 error_code) {
    (void)frame;
    u64 fault_addr = read_cr2();

    print(StringView("!!! PAGE FAULT at "));
    print_hex(fault_addr);
    print(StringView(" error "));
    print_hex(error_code);
    print(StringView(" !!!\n"));

    print(StringView("!!! UNHANDLED PAGE FAULT - halting !!!\n"));

    halt();
}

export class InterruptManager {
  public:
    static void init() {
        remap_pic();

        for (usize i = 0; i < 256; ++i) {
            idt[i].type_attributes = 0;
            idt[i].set_offset(0);
        }

        set_handler(14, reinterpret_cast<u64>(page_fault_handler), 0x8E);
        set_handler(33, reinterpret_cast<u64>(keyboard_handler), 0x8E);

        idtr.limit = sizeof(idt) - 1;
        idtr.base = reinterpret_cast<u64>(idt);

        load_idt(&idtr);
        enable_interrupts();
    }

    static void set_handler(u8 vector, u64 handler, u8 type_attr) {
        idt[vector].set_offset(handler);
        idt[vector].type_attributes = type_attr;
    }
};
