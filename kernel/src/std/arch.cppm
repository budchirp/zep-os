export module zep.arch;

import zep.std.types;

export extern "C" [[noreturn]] void halt();
export extern "C" unsigned long long read_msr(unsigned int msr);
export extern "C" void write_msr(unsigned int msr, unsigned long long value);
export extern "C" void load_page_table(unsigned long long phys_addr);
export extern "C" unsigned char inb(unsigned short port);
export extern "C" void outb(unsigned short port, unsigned char value);
export extern "C" void load_gdt(void* gdtr_ptr);
export extern "C" void load_tss(unsigned short selector);
export extern "C" void load_idt(void* idtr_ptr);
export extern "C" void enable_interrupts();
export extern "C" unsigned long long read_cr2();
export extern "C" void arch_serial_write(char byte);
export extern "C" void init_syscalls_arch();
export extern "C" void jump_to_user(unsigned long long entry, unsigned long long stack, unsigned long long argc, unsigned long long argv);
