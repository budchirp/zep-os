module;

#include "runtime/runtime.h"

#include <efi.h>

export module zep.device.serial;

import zep.device;
import zep.std.types;

export class Serial : public Device {
  private:
    void write_raw(char byte) {
#if defined(__x86_64__)
        __asm__ volatile("outb %0, %1"
                         :
                         : "a"(static_cast<u8>(byte)), "Nd"(static_cast<u16>(0x3f8)));
#elif defined(__aarch64__)
        *reinterpret_cast<volatile u32*>(0x09000000) = static_cast<u32>(byte);
#else
        (void)byte;
#endif
    }

    void write_char(char byte) {
        if (byte == '\n') {
            write_raw('\r');
        }
        write_raw(byte);
    }

  public:
    using Device::write;

    explicit Serial(void* con_out) { (void)con_out; }

    string name() override { return "serial"; }

    void write(string str) {
        for (auto* character = str; *character != '\0'; ++character) {
            write_char(*character);
        }
    }
};

alignas(Serial) static unsigned char serial_storage[sizeof(Serial)];

export Serial* init_serial(void* con_out) {
    return new (serial_storage) Serial(con_out);
}
