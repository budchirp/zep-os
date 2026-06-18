import device.gpu.framebuffer { Framebuffer }

public struct Terminal {
    public:
        var framebuffer: *Framebuffer
    
    private:
        var cursor_x: u64
        var cursor_y: u64

        var columns: u64
        var rows: u64

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
