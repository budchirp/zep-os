import serial { Serial }
import gfx.terminal { Terminal }
import common.logger { Logger }

public extern var serial: *Serial
public extern var terminal: *Terminal

public var mut logger: Logger