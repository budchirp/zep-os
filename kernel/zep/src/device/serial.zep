public struct Serial {
    private:
        var base: *u8

    public:
        fn write(str: cstr) -> void {
            zep_serial_write(self, str)
        }
}

extern fn zep_serial_write(serial: *Serial, str: cstr) -> void
