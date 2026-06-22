extern "C" unsigned long long make_syscall(unsigned long long num, unsigned long long arg1, unsigned long long arg2, unsigned long long arg3, unsigned long long arg4) {
    register unsigned long long x0 asm("x0") = arg1;
    register unsigned long long x1 asm("x1") = arg2;
    register unsigned long long x2 asm("x2") = arg3;
    register unsigned long long x3 asm("x3") = arg4;
    register unsigned long long x8 asm("x8") = num;
    __asm__ volatile(
        "svc #0"
        : "+r"(x0)
        : "r"(x1), "r"(x2), "r"(x3), "r"(x8)
        : "memory"
    );
    return x0;
}
