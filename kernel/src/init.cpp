#include "boot/info.h"
#include "std/runtime.h"

import zep.std.types;
import zep.std;
import zep.arch;
import zep.serial;
import zep.gfx.framebuffer;
import zep.gfx.terminal;
import zep.memory.memory_map;
import zep.memory.physical_memory;

namespace zep {
alignas(Serial) static unsigned char serial_storage[sizeof(Serial)];
alignas(gfx::Framebuffer) static unsigned char framebuffer_storage[sizeof(gfx::Framebuffer)];
alignas(gfx::Terminal) static unsigned char terminal_storage[sizeof(gfx::Terminal)];
alignas(memory::MemoryMap) static unsigned char memory_map_storage[sizeof(memory::MemoryMap)];

extern "C" {
static Serial* serial = nullptr;
static gfx::Framebuffer* framebuffer = nullptr;
static gfx::Terminal* terminal = nullptr;
static memory::MemoryMap* memory_map = nullptr;
static memory::PhysicalMemory physical_memory;
}

extern "C" void init() {
    auto info = BootInfo::get();

    auto uart_base = Serial::PL011_BASE;
    if (info.hhdm_offset != 0) {
        uart_base += info.hhdm_offset;
    }

    serial = new (serial_storage) Serial(uart_base);
    serial->write("Zep OS: serial up\n");

    arch::init();

    if (info.framebuffer.address != nullptr) {
        framebuffer = new (framebuffer_storage)
            gfx::Framebuffer(info.framebuffer.address, info.framebuffer.width,
                             info.framebuffer.height, info.framebuffer.pitch, info.framebuffer.bpp);
        serial->write("Zep OS: framebuffer up\n");

        terminal = new (terminal_storage) gfx::Terminal(*framebuffer);
        terminal->bg_color = 0x000000;
        terminal->fg_color = 0xFFFFFF;
        terminal->clear();

        serial->write("Zep OS: terminal up\n");
    } else {
        serial->write("Zep OS: no framebuffer\n");
    }

    if (info.memory_map != nullptr && info.memory_map_count > 0) {
        memory_map =
            new (memory_map_storage) memory::MemoryMap(info.memory_map, info.memory_map_count);
        serial->write("Zep OS: memory map up\n");

        if (physical_memory.init(*memory_map)) {
            serial->write("Zep OS: physical memory up\n");
        } else {
            serial->write("Zep OS: physica _memory init failed\n");
        }
    }

    serial->write("Zep OS: boot complete\n");

    if (terminal != nullptr) {
        terminal->print("Zep OS\n");
    }

    halt();
}

} // namespace zep
