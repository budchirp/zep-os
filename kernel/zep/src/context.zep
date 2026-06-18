import common.logger { Logger }
import device.serial { Serial }
import gfx.terminal { Terminal }

public struct Context {
    public:
        var terminal: *Terminal
        var logger: *Logger
}