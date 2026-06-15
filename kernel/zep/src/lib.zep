import std { halt }

import global { serial, terminal, logger }

import common.logger { Logger }

public fn main() -> never {
    *logger = Logger(serial, terminal)

    logger->print("Hello, World!")

    halt()
}