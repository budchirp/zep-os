module;

#include "runtime/runtime.h"

export module zep.common.logger;

import zep.device.serial;
import zep.gfx.terminal;
import zep.std.types;

export class Logger {
  private:
    bool use_terminal = false;

    void print(string str[], usize length) const {
        for (usize i = 0; i < length; ++i) {
            print(str[i]);
        }
    }

  public:
    Serial* serial;
    Terminal* terminal;

    static constexpr string TAG = "ZepOS";

    explicit Logger() : serial(nullptr), terminal(nullptr) {}

    void switch_to_graphics() {
        use_terminal = true;

        if (terminal != nullptr) {
            terminal->clear();
        }
    }

    void print(string str) const {
        if (use_terminal && terminal != nullptr) {
            terminal->print(str);
        } else {
            serial->write(str);
        }
    }

    void log(string str) const {
        string message[] = {TAG, ": ", str, "\n"};
        print(message, 4);
    }
};

alignas(Logger) static unsigned char logger_storage[sizeof(Logger)];

export Logger* init_logger() {
    return new (logger_storage) Logger();
}
