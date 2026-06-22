module zep.arch;

extern "C" unsigned long long kernel_stack_temp = 0;
extern "C" unsigned long long user_rsp_temp = 0;

extern "C" void syscall_entry() {}

extern "C" void jump_to_user(unsigned long long entry, unsigned long long stack, unsigned long long argc, unsigned long long argv) {
    (void)entry;
    (void)stack;
    (void)argc;
    (void)argv;
    while (true) {}
}

extern "C" unsigned long long read_msr(unsigned int msr) {
    (void)msr;
    return 0;
}

extern "C" void write_msr(unsigned int msr, unsigned long long value) {
    (void)msr;
    (void)value;
}

extern "C" void init_syscalls_arch() {}

extern "C" void load_page_table(unsigned long long phys_addr) {
    (void)phys_addr;
}

extern "C" unsigned char inb(unsigned short port) {
    (void)port;
    return 0;
}

extern "C" void outb(unsigned short port, unsigned char value) {
    (void)port;
    (void)value;
}

extern "C" void load_gdt(void* gdtr_ptr) {
    (void)gdtr_ptr;
}

extern "C" void load_tss(unsigned short selector) {
    (void)selector;
}

extern "C" void load_idt(void* idtr_ptr) {
    (void)idtr_ptr;
}

extern "C" void enable_interrupts() {}

extern "C" unsigned long long read_cr2() {
    return 0;
}

extern "C" void arch_serial_write(char byte) {
    *reinterpret_cast<volatile unsigned int*>(0x09000000) = static_cast<unsigned int>(byte);
}
