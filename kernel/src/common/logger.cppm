module;

#include "runtime/runtime.h"

export module zep.common.logger;

import zep.device.serial;
import zep.gfx.terminal;
import zep.std.types;
import zep.std.string_view;

export class Logger {
  private:
    bool use_terminal = false;

    void print(const StringView* str, usize length) const {
        for (usize i = 0; i < length; ++i) {
            print(str[i]);
        }
    }

  public:
    Serial* serial;
    Terminal* terminal;

    static constexpr StringView TAG{"ZepOS"};

    explicit Logger() : serial(nullptr), terminal(nullptr) {}

    void switch_to_graphics() {
        use_terminal = true;

        if (terminal != nullptr) {
            terminal->clear();
        }
    }

    void print(StringView str) const {
        if (use_terminal && terminal != nullptr) {
            terminal->print(str);
        } else {
            serial->write(str);
        }
    }

    void log(StringView str) const {
        StringView message[] = {TAG, StringView(": "), str, StringView("\n")};
        print(message, 4);
    }
};

alignas(Logger) static unsigned char logger_storage[sizeof(Logger)];

export Logger* init_logger() {
    return new (logger_storage) Logger();
}
