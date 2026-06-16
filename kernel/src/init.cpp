#include "boot/limine/info.h"
#include "std/runtime.h"

import zep.std.types;
import zep.std;
import zep.arch;
import zep.serial;
import zep.gfx.terminal;
import zep.gfx.color;
import zep.device.video.framebuffer;
import zep.device.memory;

alignas(Serial) static unsigned char serial_storage[sizeof(Serial)];

static Serial* serial_ptr = nullptr;
static Terminal* terminal_ptr = nullptr;

extern "C" {

extern void* serial;
void* serial __attribute__((weak));
void* terminal __attribute__((weak));

extern void main();
void main() __attribute__((weak));
void main() {}

extern void* terminal;

void init() {
    auto info = LimineInfo::get();

    auto uart_base = Serial::BASE;
    if (info.hhdm_offset != 0) {
        uart_base += info.hhdm_offset;
    }

    serial_ptr = new (serial_storage) Serial(uart_base);
    serial = serial_ptr;

    serial_ptr->write("Zep OS: serial up\n");

    init_arch();

    FramebufferDevice framebuffer_device(info);
    auto* framebuffer = framebuffer_device.init();

    if (framebuffer != nullptr) {
        terminal_ptr = new Terminal(*framebuffer);
        terminal_ptr->bg_color = Color::black();
        terminal_ptr->fg_color = Color::white();
        terminal_ptr->clear();

        serial_ptr->write("Zep OS: framebuffer up\n");
    } else {
        serial_ptr->write("Zep OS: no framebuffer\n");
    }

    terminal = terminal_ptr;

    MemoryDevice memory_device(info);
    memory_device.init();

    serial_ptr->write("Zep OS: boot complete\n");

    main();
}
}
