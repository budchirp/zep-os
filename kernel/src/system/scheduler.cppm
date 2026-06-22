module;

#include "runtime/runtime.h"

export module zep.system.scheduler;

import zep.std.types;

export class Thread {
  public:
    u64 rsp = 0;
    u8* stack = nullptr;

    explicit Thread(void (*entry_point)()) {
        constexpr usize STACK_SIZE = 16384;
        stack = new u8[STACK_SIZE];

        u64* stack_top = reinterpret_cast<u64*>(stack + STACK_SIZE);
        *(--stack_top) = reinterpret_cast<u64>(entry_point);
        *(--stack_top) = 0;
        *(--stack_top) = 0;
        *(--stack_top) = 0;
        *(--stack_top) = 0;
        *(--stack_top) = 0;
        *(--stack_top) = 0;

        rsp = reinterpret_cast<u64>(stack_top);
    }

    explicit Thread() {}

    ~Thread() {
        if (stack != nullptr) {
            delete[] stack;
        }
    }
};

extern "C" void switch_context(u64* old_rsp, u64 new_rsp);

export class Scheduler {
  private:
    static inline Thread* threads[4] = {nullptr};
    static inline usize thread_count = 0;
    static inline usize current_thread_index = 0;

  public:
    static void add_thread(Thread* thread) {
        if (thread_count < 4) {
            threads[thread_count++] = thread;
        }
    }

    static void schedule() {
        if (thread_count <= 1) {
            return;
        }

        usize old_index = current_thread_index;
        current_thread_index = (current_thread_index + 1) % thread_count;

        switch_context(&threads[old_index]->rsp, threads[current_thread_index]->rsp);
    }
};

export void thread_yield() {
    Scheduler::schedule();
}
