import serial { Serial }
import gfx.terminal { Terminal }
import common.logger { Logger }

@name("__serial")
public extern var serial: *Serial

@name("__terminal")
public extern var terminal: *Terminal

public var mut logger: Logger