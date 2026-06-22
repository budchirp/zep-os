module zep.arch;

extern "C" unsigned long long kernel_stack_temp = 0;
extern "C" unsigned long long user_rsp_temp = 0;

extern "C" void handle_syscall(unsigned long long num, unsigned long long arg1,
                               unsigned long long arg2, unsigned long long arg3,
                               unsigned long long arg4);

extern "C" __attribute__((naked)) void syscall_entry() {
    __asm__ volatile("movq %%rsp, user_rsp_temp(%%rip)\n"
                     "movq kernel_stack_temp(%%rip), %%rsp\n"

                     "pushq user_rsp_temp(%%rip)\n"
                     "pushq %%r11\n"
                     "pushq %%rcx\n"
                     "pushq %%rdx\n"
                     "pushq %%rsi\n"
                     "pushq %%rdi\n"
                     "pushq %%r8\n"
                     "pushq %%r9\n"
                     "pushq %%r10\n"

                     "movq %%r10, %%r8\n"
                     "movq %%rdx, %%rcx\n"
                     "movq %%rsi, %%rdx\n"
                     "movq %%rdi, %%rsi\n"
                     "movq %%rax, %%rdi\n"

                     "call handle_syscall\n"

                     "popq %%r10\n"
                     "popq %%r9\n"
                     "popq %%r8\n"
                     "popq %%rdi\n"
                     "popq %%rsi\n"
                     "popq %%rdx\n"
                     "popq %%rcx\n"
                     "popq %%r11\n"
                     "popq %%rsp\n"

                     "sysretq\n"
                     :
                     :
                     : "memory");
}

extern "C" __attribute__((naked)) void jump_to_user(unsigned long long entry,
                                                    unsigned long long stack,
                                                    unsigned long long argc,
                                                    unsigned long long argv) {
    __asm__ volatile("movq %%rsp, kernel_stack_temp(%%rip)\n"
                     "pushq $0x1B\n"
                     "pushq %%rsi\n"
                     "pushq $0x202\n"
                     "pushq $0x23\n"
                     "pushq %%rdi\n"
                     "movq %%rdx, %%rdi\n"
                     "movq %%rcx, %%rsi\n"
                     "xorq %%rax, %%rax\n"
                     "xorq %%rbx, %%rbx\n"
                     "xorq %%rcx, %%rcx\n"
                     "xorq %%rdx, %%rdx\n"
                     "xorq %%rbp, %%rbp\n"
                     "xorq %%r8, %%r8\n"
                     "xorq %%r9, %%r9\n"
                     "xorq %%r10, %%r10\n"
                     "xorq %%r11, %%r11\n"
                     "xorq %%r12, %%r12\n"
                     "xorq %%r13, %%r13\n"
                     "xorq %%r14, %%r14\n"
                     "xorq %%r15, %%r15\n"
                     "iretq\n"
                     :
                     :
                     : "memory");
}

extern "C" unsigned long long read_msr(unsigned int msr) {
    unsigned int low, high;
    __asm__ volatile("rdmsr" : "=a"(low), "=d"(high) : "c"(msr));
    return (static_cast<unsigned long long>(high) << 32) | low;
}

extern "C" void write_msr(unsigned int msr, unsigned long long value) {
    unsigned int low = static_cast<unsigned int>(value & 0xFFFFFFFF);
    unsigned int high = static_cast<unsigned int>((value >> 32) & 0xFFFFFFFF);
    __asm__ volatile("wrmsr" : : "c"(msr), "a"(low), "d"(high));
}

extern "C" void init_syscalls_arch() {
    unsigned long long efer = read_msr(0xC0000080);
    write_msr(0xC0000080, efer | 1);

    unsigned long long star = ((0x10ULL) << 48) | ((0x08ULL) << 32);
    write_msr(0xC0000081, star);
    write_msr(0xC0000082, reinterpret_cast<unsigned long long>(syscall_entry));
    write_msr(0xC0000084, 0x200);
}

extern "C" void load_page_table(unsigned long long phys_addr) {
    __asm__ volatile("mov %0, %%cr3" : : "r"(phys_addr) : "memory");
}

extern "C" unsigned char inb(unsigned short port) {
    unsigned char value;
    __asm__ volatile("inb %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

extern "C" void outb(unsigned short port, unsigned char value) {
    __asm__ volatile("outb %0, %1" : : "a"(value), "Nd"(port));
}

extern "C" void load_gdt(void* gdtr_ptr) {
    __asm__ volatile("lgdt (%0)" : : "r"(gdtr_ptr) : "memory");

    __asm__ volatile("mov $0x10, %%ax\n"
                     "mov %%ax, %%ds\n"
                     "mov %%ax, %%es\n"
                     "mov %%ax, %%fs\n"
                     "mov %%ax, %%gs\n"
                     "mov %%ax, %%ss\n"
                     "pushq $0x08\n"
                     "leaq 1f(%%rip), %%rax\n"
                     "pushq %%rax\n"
                     "lretq\n"
                     "1:\n"
                     :
                     :
                     : "rax", "memory");
}

extern "C" void load_tss(unsigned short selector) {
    __asm__ volatile("ltr %0" : : "r"(selector));
}

extern "C" void load_idt(void* idtr_ptr) {
    __asm__ volatile("lidt (%0)" : : "r"(idtr_ptr) : "memory");
}

extern "C" void enable_interrupts() {
    __asm__ volatile("sti");
}

extern "C" unsigned long long read_cr2() {
    unsigned long long val;
    __asm__ volatile("mov %%cr2, %0" : "=r"(val));
    return val;
}

extern "C" void arch_serial_write(char byte) {
    __asm__ volatile("outb %0, %1"
                     :
                     : "a"(static_cast<unsigned char>(byte)),
                       "Nd"(static_cast<unsigned short>(0x3f8)));
}
