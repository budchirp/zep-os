export module zep.std;

import zep.std.types;

export [[noreturn]] inline void panic() {
    while (true) {
        __builtin_trap();
    }
}

export [[noreturn]] void halt();

export extern "C" void zep_panic() {
    panic();
}

export extern "C" void zep_halt() {
    halt();
}
