export module zep.serial;

import zep.std.types;

export class Serial {
    static constexpr u16 COM1 = 0x3F8;

    static void outb(u16 port, u8 value) {
        __asm__ volatile("outb %0, %1" : : "a"(value), "Nd"(port));
    }

    static u8 inb(u16 port) {
        u8 value;
        __asm__ volatile("inb %1, %0" : "=a"(value) : "Nd"(port));
        return value;
    }

    bool tx_ready() {
        return (inb(COM1 + 5) & 0x20) != 0;
    }

    void write_char(char byte) {
        while (!tx_ready()) {}
        outb(COM1, byte);
    }

  public:
    static constexpr u64 BASE = 0x3F8;
    static constexpr u32 BAUDRATE = 115200;

    explicit Serial(u64) {
        outb(COM1 + 1, 0x00);
        outb(COM1 + 3, 0x80);
        outb(COM1 + 0, 0x01);
        outb(COM1 + 1, 0x00);
        outb(COM1 + 3, 0x03);
        outb(COM1 + 2, 0xC7);
        outb(COM1 + 4, 0x0B);
    }

    void write(string str) {
        for (auto* character = str; *character != '\0'; ++character) {
            write_char(*character);
        }
    }
};

export extern "C" void zep_serial_write(Serial* serial, string str) {
    serial->write(str);
}
