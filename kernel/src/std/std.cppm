export module zep.std;

import zep.std.types;
import zep.context;

export void print(string str) {
    auto context = get_context();

    context->logger->print(str);
}

export extern "C" [[noreturn]] void panic(string str) {
    print("!!! PANIC !!!");
    print(str);

    while (true) {
        __builtin_trap();
    }
}
