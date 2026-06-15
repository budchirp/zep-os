export module zep.serial;

import zep.std.types;

export namespace zep {

class Serial {
    u8* base;

    static constexpr u64 UART_DR = 0x000;
    static constexpr u64 UART_FR = 0x018;
    static constexpr u64 UART_IBRD = 0x024;
    static constexpr u64 UART_FBRD = 0x028;
    static constexpr u64 UART_LCR_H = 0x02C;
    static constexpr u64 UART_CR = 0x030;

    static constexpr u32 UART_FR_TXFF = 0x20;

    bool tx_ready() {
        volatile auto* fr = reinterpret_cast<volatile u32*>(base + UART_FR);
        return (*fr & UART_FR_TXFF) == 0;
    }

    void write_char(char byte) {
        while (!tx_ready()) {}

        volatile auto* dr = reinterpret_cast<volatile u32*>(base + UART_DR);
        *dr = byte;
    }

  public:
    static constexpr u64 PL011_BASE = 0x09000000;
    static constexpr u32 UARTCLK = 24000000;
    static constexpr u32 DEFAULT_BAUD = 115200;

    explicit Serial(u64 mmio_base) : base(reinterpret_cast<u8*>(mmio_base)) {
        volatile auto* cr = reinterpret_cast<volatile u32*>(base + UART_CR);
        *cr = 0;

        auto baud_divisor = static_cast<u32>(UARTCLK / (16 * DEFAULT_BAUD));
        auto frac = static_cast<u32>(((static_cast<u64>(UARTCLK) * 4) / DEFAULT_BAUD + 1) / 2) % 64;

        volatile auto* ibrd = reinterpret_cast<volatile u32*>(base + UART_IBRD);
        volatile auto* fbrd = reinterpret_cast<volatile u32*>(base + UART_FBRD);
        volatile auto* lcr = reinterpret_cast<volatile u32*>(base + UART_LCR_H);

        *ibrd = baud_divisor;
        *fbrd = frac;
        *lcr = 0x70;

        *cr = 0x301;
    }

    void write(string str) {
        for (auto* character = str; *character != '\0'; ++character) {
            write_char(*character);
        }
    }
};

extern "C" void zep_serial_write(Serial* serial, string str) {
    serial->write(str);
}

} // namespace zep
