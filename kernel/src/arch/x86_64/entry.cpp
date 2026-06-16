extern "C" void init();

extern "C" unsigned char stack_top[];

__attribute__((naked, section(".text._start"))) extern "C" void _start() {
    __asm__ volatile(
        "movq   %[stack_top], %%rsp\n"
        "subq   $8, %%rsp\n"

        "fninit\n"

        "leaq   __bss_start(%%rip), %%rdi\n"
        "leaq   __bss_end(%%rip), %%rsi\n"
        "1:     cmpq   %%rsi, %%rdi\n"
        "jge    2f\n"
        "movq   $0, (%%rdi)\n"
        "addq   $8, %%rdi\n"
        "jmp    1b\n"
        "2:\n"

        "leaq   __init_array_start(%%rip), %%rdi\n"
        "leaq   __init_array_end(%%rip), %%rsi\n"
        "3:     cmpq   %%rsi, %%rdi\n"
        "jge    4f\n"
        "movq   (%%rdi), %%rax\n"
        "addq   $8, %%rdi\n"
        "callq  *%%rax\n"
        "jmp    3b\n"
        "4:\n"

        "callq  init\n"
        "hlt\n"
        "jmp    .\n"
        :
        : [stack_top] "i"(&stack_top)
        : "rax", "rdi", "rsi", "memory");
}
