import std { halt }

import global { serial, terminal, logger }

import common.logger { Logger }

public fn main() -> never {
    logger = Logger(serial, terminal)
    logger.print("\nZep OS\n")

    halt()
}