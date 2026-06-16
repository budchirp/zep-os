module zep.std;

[[noreturn]] void halt() {
    while (true) {
        __asm__ volatile("hlt");
    }
}
