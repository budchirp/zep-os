module zep.std;

extern "C" [[noreturn]] void halt() {
    while (true) {
        __asm__ volatile("hlt");
    }
}

extern "C" __attribute__((naked)) void switch_context(unsigned long long* old_rsp, unsigned long long new_rsp) {
    __asm__ volatile(
        "pushq %rbx\n"
        "pushq %rbp\n"
        "pushq %r12\n"
        "pushq %r13\n"
        "pushq %r14\n"
        "pushq %r15\n"
        "movq %rsp, (%rdi)\n"
        "movq %rsi, %rsp\n"
        "popq %r15\n"
        "popq %r14\n"
        "popq %r13\n"
        "popq %r12\n"
        "popq %rbp\n"
        "popq %rbx\n"
        "retq\n"
    );
}

