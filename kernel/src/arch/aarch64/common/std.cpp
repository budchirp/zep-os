module zep.std;

extern "C" [[noreturn]] void halt() {
    while (true) {
        __asm__ volatile("wfi");
    }
}
