export module zep.std;

import zep.std.types;
import zep.std.string_view;
import zep.context;
import zep.gfx.color;

extern "C" void* kernel_allocate(usize size);
extern "C" void kernel_deallocate(void* ptr);

export extern "C" void print(StringView str) {
    auto* context = get_context();

    context->logger->print(str);
}

export extern "C" void log(StringView str) {
    auto* context = get_context();

    context->logger->log(str);
}

export extern "C" void error(StringView str) {
    auto* context = get_context();
    auto* terminal = context->logger->terminal;

    if (terminal != nullptr) {
        terminal->print(str, Color::red());
    } else {
        context->logger->print(str);
    }
}

export extern "C" [[noreturn]] void panic(StringView str) {
    print(StringView("!!! PANIC !!!"));
    print(str);

    while (true) {
        __builtin_trap();
    }
}

export extern "C" void* allocate(usize size) {
    return kernel_allocate(size);
}

export extern "C" void free(void* ptr) {
    kernel_deallocate(ptr);
}

export extern "C" [[noreturn]] void halt();
