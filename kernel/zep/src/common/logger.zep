import device.serial { Serial }
import gfx.terminal { Terminal }

public struct Logger {
    private:
        var serial: *Serial
        var terminal: *Terminal

    public: 
        fn Logger(serial: *Serial, terminal: *Terminal) -> Logger {
            if (terminal != null) {
                terminal->clear()
            }

            return Logger {
                serial: serial,
                terminal: terminal
            }
        }

        fn print(str: cstr) -> void {
            if (terminal != null) {
                terminal->print(str)
            } else {
                serial->write(str)
            }
        }
}