extern "C" unsigned long long make_syscall(unsigned long long num, unsigned long long arg1, unsigned long long arg2, unsigned long long arg3, unsigned long long arg4) {
    unsigned long long ret;
    register unsigned long long r10 asm("r10") = arg4;
    __asm__ volatile(
        "syscall"
        : "=a"(ret)
        : "a"(num), "D"(arg1), "S"(arg2), "d"(arg3), "r"(r10)
        : "rcx", "r11", "memory"
    );
    return ret;
}
