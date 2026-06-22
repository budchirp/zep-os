module;

#include "runtime/runtime.h"

export module zep.system.scheduler;

import zep.std.types;
import zep.memory.vmm;
import zep.arch;
import zep.std.string_view;
import zep.context;
import zep.fs;
import zep.system.elf;
import zep.std;

extern "C" void switch_context(u64* old_rsp, u64 new_rsp);
extern "C" unsigned long long kernel_stack_temp;

export class Process {
  public:
    u32 pid = 0;
    u32 uid = 0;
    u32 gid = 0;
    u64 cr3 = 0;
    u64 entry = 0;
    u64 user_stack = 0;
    u64 argc = 0;
    u64 argv = 0;
    bool active = false;
};

export class Thread {
  public:
    u64 rsp = 0;
    u8* kernel_stack = nullptr;
    u64 kernel_stack_top = 0;
    Process* process = nullptr;
    bool active = false;

    explicit Thread(void (*entry_point)()) {
        constexpr usize STACK_SIZE = 16384;
        kernel_stack = new u8[STACK_SIZE];
        kernel_stack_top = reinterpret_cast<u64>(kernel_stack + STACK_SIZE);

        u64* stack_top = reinterpret_cast<u64*>(kernel_stack_top);
        *(--stack_top) = reinterpret_cast<u64>(entry_point);
        *(--stack_top) = 0; // rbx
        *(--stack_top) = 0; // rbp
        *(--stack_top) = 0; // r12
        *(--stack_top) = 0; // r13
        *(--stack_top) = 0; // r14
        *(--stack_top) = 0; // r15

        rsp = reinterpret_cast<u64>(stack_top);
        active = true;
    }

    explicit Thread() { active = true; }

    ~Thread() {
        if (kernel_stack != nullptr) {
            delete[] kernel_stack;
        }
    }
};

static void userspace_thread_stub();

export class Scheduler {
  private:
    static inline Process processes[16] = {};
    static inline Thread* threads[16] = {nullptr};
    static inline usize thread_count = 0;
    static inline usize current_thread_index = 0;
    static inline u32 next_pid = 1;

  public:
    static void add_thread(Thread* thread) {
        if (thread_count < 16) {
            threads[thread_count++] = thread;
        }
    }

    static Thread* get_current_thread() {
        if (current_thread_index < thread_count) {
            return threads[current_thread_index];
        }
        return nullptr;
    }

    static void schedule() {
        if (thread_count <= 1) {
            return;
        }

        usize old_index = current_thread_index;

        // Find next active thread
        usize next_index = old_index;
        for (usize i = 0; i < thread_count; ++i) {
            next_index = (next_index + 1) % thread_count;
            if (threads[next_index] != nullptr && threads[next_index]->active) {
                break;
            }
        }

        if (next_index == old_index) {
            return; // no other active thread
        }

        current_thread_index = next_index;
        auto* next_thread = threads[current_thread_index];

        // Update syscall stack top
        if (next_thread->kernel_stack_top != 0) {
            kernel_stack_temp = next_thread->kernel_stack_top;
        }

        // Switch page table if process changes
        if (next_thread->process != nullptr && next_thread->process->cr3 != 0) {
            load_page_table(next_thread->process->cr3);
            PageTableManager::set_active_pml4(
                reinterpret_cast<PageTable*>(next_thread->process->cr3));
        }

        switch_context(&threads[old_index]->rsp, next_thread->rsp);
    }

    static u32 spawn(const char* path, const char** argv, u32 parent_uid) {
        print(StringView("Scheduler::spawn: Starting spawn...\n"));
        // Find process slot
        Process* proc = nullptr;
        for (usize i = 0; i < 16; ++i) {
            if (!processes[i].active) {
                proc = &processes[i];
                break;
            }
        }
        if (proc == nullptr) {
            print(StringView("Scheduler::spawn: No active process slot!\n"));
            return 0;
        }

        // Read ELF file
        auto* context = get_context();
        auto* user_fs = context->device_manager->get<FileSystem>(StringView("fs"));
        if (user_fs == nullptr) {
            print(StringView("Scheduler::spawn: fs driver not found!\n"));
            return 0;
        }

        void* elf_buffer = allocate(65536);
        if (elf_buffer == nullptr) {
            print(StringView("Scheduler::spawn: failed to allocate elf_buffer!\n"));
            return 0;
        }

        print(StringView("Scheduler::spawn: Reading ELF file...\n"));
        usize elf_size =
            user_fs->read(StringView(path), 0, reinterpret_cast<u8*>(elf_buffer), 65536);
        if (elf_size == 0) {
            print(StringView("Scheduler::spawn: Failed to read ELF file (size 0)!\n"));
            free(elf_buffer);
            return 0;
        }

        print(StringView("Scheduler::spawn: Cloning PML4...\n"));
        u64 old_cr3 = reinterpret_cast<u64>(get_active_pml4());
        PageTable* new_pml4 = clone_pml4(get_active_pml4());
        u64 new_cr3 = reinterpret_cast<u64>(new_pml4);

        print(StringView("Scheduler::spawn: Loading ELF into new CR3...\n"));
        // Load ELF into new PML4
        load_page_table(new_cr3);
        PageTableManager::set_active_pml4(new_pml4);
        u64 entry_point = ElfLoader::load(reinterpret_cast<const u8*>(elf_buffer), elf_size);

        print(StringView("Scheduler::spawn: Allocating stack...\n"));
        // Allocate user stack
        void* user_stack = allocate(16384);
        u64 user_stack_top = reinterpret_cast<u64>(user_stack) + 16384;
        u64 sp = user_stack_top;

        u64 argc = 0;
        u64 final_argv = 0;

        if (argv != nullptr) {
            while (argv[argc] != nullptr) {
                argc++;
            }
        }

        if (argc > 0) {
            // Allocate temporary array to hold the child-virtual pointers
            u64* child_argv_ptrs = new u64[argc + 1];

            // Push strings onto stack in reverse order
            for (int i = static_cast<int>(argc) - 1; i >= 0; --i) {
                const char* arg = argv[i];
                usize len = 0;
                while (arg[len] != '\0') {
                    len++;
                }
                len++; // including null terminator

                sp -= len;
                char* dest = reinterpret_cast<char*>(sp);
                for (usize j = 0; j < len; ++j) {
                    dest[j] = arg[j];
                }
                child_argv_ptrs[i] = sp;
            }
            child_argv_ptrs[argc] = 0;

            // Align stack to 8 bytes for pointers
            sp &= ~7ULL;

            // Push pointers to strings onto stack
            sp -= (argc + 1) * sizeof(u64);
            u64* argv_array = reinterpret_cast<u64*>(sp);
            for (usize i = 0; i <= argc; ++i) {
                argv_array[i] = child_argv_ptrs[i];
            }

            final_argv = sp;

            delete[] child_argv_ptrs;
        }

        // Align stack to 16 bytes for user jump
        sp &= ~15ULL;

        print(StringView("Scheduler::spawn: Restoring old CR3...\n"));
        // Restore old PML4
        load_page_table(old_cr3);
        PageTableManager::set_active_pml4(reinterpret_cast<PageTable*>(old_cr3));
        free(elf_buffer);

        // Initialize PCB
        proc->pid = next_pid++;
        proc->uid = parent_uid; // inherits parent UID
        proc->cr3 = new_cr3;
        proc->entry = entry_point;
        proc->user_stack = sp;
        proc->argc = argc;
        proc->argv = final_argv;
        proc->active = true;

        print(StringView("Scheduler::spawn: Initializing TCB...\n"));
        // Initialize TCB
        auto* thread = new Thread(userspace_thread_stub);
        thread->process = proc;

        add_thread(thread);
        print(StringView("Scheduler::spawn: Spawn complete, returning pid...\n"));
        return proc->pid;
    }

    static int kill(u32 pid) {
        // Can't kill init (pid 1) unless root
        auto* current = get_current_thread();
        if (pid == 1 && current != nullptr && current->process != nullptr &&
            current->process->uid != 0) {
            return -1;
        }

        bool found = false;
        for (usize i = 0; i < thread_count; ++i) {
            if (threads[i] != nullptr && threads[i]->process != nullptr &&
                threads[i]->process->pid == pid) {
                threads[i]->active = false;
                threads[i]->process->active = false;
                found = true;
            }
        }
        return found ? 0 : -1;
    }
};

static void userspace_thread_stub() {
    auto* current_thread = Scheduler::get_current_thread();
    auto* process = current_thread->process;
    enable_interrupts();
    jump_to_user(process->entry, process->user_stack, process->argc, process->argv);
}

export extern "C" void thread_yield() {
    Scheduler::schedule();
}
