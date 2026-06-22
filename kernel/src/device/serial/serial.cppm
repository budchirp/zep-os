module;

#include "runtime/runtime.h"

#include <efi.h>

export module zep.device.serial;

import zep.device;
import zep.std.types;
import zep.std.string_view;
import zep.arch;

export class Serial : public Device {
  private:
    void write_raw(char byte) {
        arch_serial_write(byte);
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

    StringView name() override { return StringView("serial"); }

    void write(StringView str) {
        for (usize i = 0; i < str.length(); ++i) {
            write_char(str.data[i]);
        }
    }
};

alignas(Serial) static unsigned char serial_storage[sizeof(Serial)];

export Serial* init_serial(void* con_out) {
    return new (serial_storage) Serial(con_out);
}
