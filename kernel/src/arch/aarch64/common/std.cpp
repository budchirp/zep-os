module zep.std;

extern "C" [[noreturn]] void halt() {
    while (true) {
        __asm__ volatile("wfi");
    }
}

extern "C" void switch_context(unsigned long long* old_rsp, unsigned long long new_rsp) {
    (void)old_rsp;
    (void)new_rsp;
}
