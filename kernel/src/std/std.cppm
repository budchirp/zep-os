export module zep.std;

import zep.std.types;

export [[noreturn]] inline void panic(string str) {
    (void)str;
    while (true) {
        __builtin_trap();
    }
}

export [[noreturn]] void halt();

export extern "C" void zep_panic(string str) {
    panic(str);
}

export extern "C" void zep_halt() {
    halt();
}
