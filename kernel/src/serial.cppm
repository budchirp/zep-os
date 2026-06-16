module;

#include <efi.h>

export module zep.serial;

import zep.std.types;

export class Serial {
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
    explicit Serial(SIMPLE_TEXT_OUTPUT_INTERFACE* con_out)
        : con_out(con_out) {}

    void write(string str) {
        for (auto* character = str; *character != '\0'; ++character) {
            write_char(*character);
        }
    }

    void write_line(string str) {
        write(str);
        write_char('\n');
    }
};

export extern "C" void zep_serial_write(Serial* serial, string str) {
    serial->write(str);
}
