export module zep.std;

import zep.std.types;

export namespace zep {

[[noreturn]] inline void panic(string str) {
    (void)str;

    while (true) {
        __builtin_trap();
    }
}

[[noreturn]] void halt();

extern "C" void zep_panic() {
    panic("");
}

extern "C" void zep_halt() {
    halt();
}
} // namespace zep
