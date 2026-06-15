extern "C" void init();

extern "C" unsigned char stack_top[];

__attribute__((used, section(".vectors"))) volatile unsigned vector_table[2048 / 4] = {
    [0 * 32] = 0x14000000,  [1 * 32] = 0x14000000,  [2 * 32] = 0x14000000,  [3 * 32] = 0x14000000,
    [4 * 32] = 0x14000000,  [5 * 32] = 0x14000000,  [6 * 32] = 0x14000000,  [7 * 32] = 0x14000000,
    [8 * 32] = 0x14000000,  [9 * 32] = 0x14000000,  [10 * 32] = 0x14000000, [11 * 32] = 0x14000000,
    [12 * 32] = 0x14000000, [13 * 32] = 0x14000000, [14 * 32] = 0x14000000, [15 * 32] = 0x14000000,
};

__attribute__((naked, section(".text._start"))) extern "C" void _start() {
    __asm__ volatile("adrp x0, stack_top\n"
                     "add  sp, x0, :lo12:stack_top\n"

                     "adrp x0, vector_table\n"
                     "add  x0, x0, :lo12:vector_table\n"
                     "msr  vbar_el1, x0\n"

                     "mrs  x0, sctlr_el1\n"
                     "orr  x0, x0, #(1 << 2)\n"
                     "orr  x0, x0, #(1 << 12)\n"
                     "msr  sctlr_el1, x0\n"

                     "mrs  x0, cpacr_el1\n"
                     "orr  x0, x0, #(3 << 20)\n"
                     "msr  cpacr_el1, x0\n"

                     "adrp x0, __bss_start\n"
                     "add  x0, x0, :lo12:__bss_start\n"
                     "adrp x1, __bss_end\n"
                     "add  x1, x1, :lo12:__bss_end\n"
                     "1: cmp  x0, x1\n"
                     "b.ge 2f\n"
                     "str  xzr, [x0], #8\n"
                     "b    1b\n"
                     "2:\n"

                     "adrp x0, __init_array_start\n"
                     "add  x0, x0, :lo12:__init_array_start\n"
                     "adrp x1, __init_array_end\n"
                     "add  x1, x1, :lo12:__init_array_end\n"
                     "3: cmp  x0, x1\n"
                     "b.ge 4f\n"
                     "ldr  x2, [x0], #8\n"
                     "blr  x2\n"
                     "b    3b\n"
                     "4:\n"

                     "bl   init\n"
                     "wfi\n"
                     "b    .\n");
}
