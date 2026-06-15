module zep.std;

namespace zep {

    [[noreturn]] void halt() {
        while (true) {
            __asm__ volatile("wfi");
        }
    }

}
