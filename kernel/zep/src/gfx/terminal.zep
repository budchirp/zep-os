public struct Terminal {
    public:
        fn clear() -> void {
            zep_terminal_clear(self)
        }

        fn print(str: cstr) -> void {
            zep_terminal_print(self, str)
        }
}

extern fn zep_terminal_clear(terminal: *Terminal) -> void
extern fn zep_terminal_print(terminal: *Terminal, str: cstr) -> void
