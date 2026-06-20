module;

#include "runtime/runtime.h"

#include <efi.h>

export module zep.device.serial;

import zep.device;
import zep.std.types;

export class Serial : public Device {
  private:
    SIMPLE_TEXT_OUTPUT_INTERFACE* con_out;

    void write_char(char byte) {
        CHAR16 buf[3];
        u8 len = 0;

        if (byte == '\n') {
            buf[len++] = u'\r';
        }

        buf[len++] = static_cast<CHAR16>(byte);
        buf[len] = u'\0';

        con_out->OutputString(con_out, buf);
    }

  public:
    using Device::write;

    explicit Serial(SIMPLE_TEXT_OUTPUT_INTERFACE* con_out) : con_out(con_out) {}

    string name() override { return "serial"; }

    void write(string str) {
        for (auto* character = str; *character != '\0'; ++character) {
            write_char(*character);
        }
    }
};

alignas(Serial) static unsigned char serial_storage[sizeof(Serial)];

export Serial* init_serial(EFI_SYSTEM_TABLE* system_table) {
    return new (serial_storage) Serial(system_table->ConOut);
}

export extern "C" void zep_serial_write(Serial* serial, string str) {
    serial->write(str);
}
