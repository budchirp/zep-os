module;

#include <common/syscalls.h>

export module zep.system.syscalls;

import zep.std.types;
import zep.std.string_view;
import zep.std;
import zep.arch;
import zep.context;
import zep.fs;
import zep.system.scheduler;
import zep.device.keyboard;

static char virtual_users_file[1024] = "root:0\nbudchirp:1000\n";
static usize virtual_users_size = 21;

static char virtual_cmdline[1024] = "";
static usize virtual_cmdline_size = 0;

static u32 get_uid_by_username(StringView username) {
    usize idx = 0;
    while (idx < virtual_users_size) {
        usize line_end = idx;
        while (line_end < virtual_users_size && virtual_users_file[line_end] != '\n') {
            line_end++;
        }

        StringView line(&virtual_users_file[idx], line_end - idx);
        int colon_idx = -1;
        for (usize i = 0; i < line.size; ++i) {
            if (line.data[i] == ':') {
                colon_idx = static_cast<int>(i);
                break;
            }
        }

        if (colon_idx != -1) {
            StringView name(line.data, colon_idx);
            if (name.equals(username)) {
                u32 uid = 0;
                for (usize i = colon_idx + 1; i < line.size; ++i) {
                    if (line.data[i] >= '0' && line.data[i] <= '9') {
                        uid = uid * 10 + (line.data[i] - '0');
                    }
                }
                return uid;
            }
        }
        idx = line_end + 1;
    }
    return static_cast<u32>(-1);
}

static bool check_read_permission(u32 uid, StringView path) {
    if (uid == 0) {
        return true;
    }
    if (path.equals(StringView("/Config/users")) || path.equals(StringView("/Config/cmdline"))) {
        return true;
    }
    if (path.starts_with(StringView("/Config"))) {
        return false;
    }
    if (path.starts_with(StringView("/Users/"))) {
        usize start = 7;
        usize end = start;
        while (end < path.size && path.data[end] != '/') {
            end++;
        }
        StringView username(path.data + start, end - start);
        u32 path_uid = get_uid_by_username(username);
        if (path_uid != static_cast<u32>(-1)) {
            if (uid != path_uid) {
                return false;
            }
        } else {
            return false;
        }
    }
    return true;
}

static bool check_write_permission(u32 uid, StringView path) {
    if (path.equals(StringView("/Config/cmdline"))) {
        return true;
    }
    if (uid == 0) {
        return true;
    }
    if (path.starts_with(StringView("/System")) || path.starts_with(StringView("/Config"))) {
        return false;
    }
    if (path.starts_with(StringView("/Users/"))) {
        usize start = 7;
        usize end = start;
        while (end < path.size && path.data[end] != '/') {
            end++;
        }
        StringView username(path.data + start, end - start);
        u32 path_uid = get_uid_by_username(username);
        if (path_uid != static_cast<u32>(-1)) {
            if (uid != path_uid) {
                return false;
            }
        } else {
            return false;
        }
    }
    return true;
}

export extern "C" u64 handle_syscall(u64 num, u64 arg1, u64 arg2, u64 arg3, u64 arg4) {
    auto* current_thread = Scheduler::get_current_thread();
    u32 current_uid = (current_thread != nullptr && current_thread->process != nullptr)
                          ? current_thread->process->uid
                          : 0;

    switch (static_cast<Syscall::Num>(num)) {
    case Syscall::Num::Allocate:
        return reinterpret_cast<u64>(allocate(arg1));
    case Syscall::Num::Free:
        free(reinterpret_cast<void*>(arg1));
        return 0;
    case Syscall::Num::Print:
        print(StringView(reinterpret_cast<const char*>(arg1), arg2));
        return 0;
    case Syscall::Num::Log:
        log(StringView(reinterpret_cast<const char*>(arg1), arg2));
        return 0;
    case Syscall::Num::Error:
        error(StringView(reinterpret_cast<const char*>(arg1), arg2));
        return 0;
    case Syscall::Num::Panic:
        panic(StringView(reinterpret_cast<const char*>(arg1), arg2));
        return 0;
    case Syscall::Num::Halt:
        halt();
        return 0;
    case Syscall::Num::Spawn: {
        const char* path = reinterpret_cast<const char*>(arg1);
        const char** argv = reinterpret_cast<const char**>(arg2);
        return Scheduler::spawn(path, argv, current_uid);
    }
    case Syscall::Num::Kill: {
        u32 pid = static_cast<u32>(arg1);
        return static_cast<u64>(Scheduler::kill(pid));
    }
    case Syscall::Num::Exit: {
        if (current_thread != nullptr) {
            current_thread->active = false;
        }
        Scheduler::schedule();
        return 0;
    }
    case Syscall::Num::GetPid: {
        if (current_thread != nullptr && current_thread->process != nullptr) {
            return current_thread->process->pid;
        }
        return 0;
    }
    case Syscall::Num::ReadChar: {
        return static_cast<u64>(Keyboard::get_char());
    }
    case Syscall::Num::SetUid: {
        u32 new_uid = static_cast<u32>(arg1);
        if (current_uid == 0) {
            if (current_thread != nullptr && current_thread->process != nullptr) {
                current_thread->process->uid = new_uid;
                return 0;
            }
        }
        return static_cast<u64>(-1);
    }
    case Syscall::Num::GetUid: {
        return current_uid;
    }
    case Syscall::Num::ReadFile: {
        const char* path_str = reinterpret_cast<const char*>(arg1);
        char* buf = reinterpret_cast<char*>(arg2);
        u64 size = arg3;
        u64 offset = arg4;
        StringView path(path_str);

        if (!check_read_permission(current_uid, path)) {
            log(StringView("[Security] Access Denied: Read permission verification failed."));
            return static_cast<u64>(-1);
        }

        // Virtualized /Config/users
        if (path.equals(StringView("/Config/users"))) {
            if (offset >= virtual_users_size) {
                return 0;
            }
            u64 to_read = size;
            if (offset + to_read > virtual_users_size) {
                to_read = virtual_users_size - offset;
            }
            for (u64 i = 0; i < to_read; ++i) {
                buf[i] = virtual_users_file[offset + i];
            }
            return to_read;
        }

        // Virtualized /Config/cmdline
        if (path.equals(StringView("/Config/cmdline"))) {
            if (offset >= virtual_cmdline_size) {
                return 0;
            }
            u64 to_read = size;
            if (offset + to_read > virtual_cmdline_size) {
                to_read = virtual_cmdline_size - offset;
            }
            for (u64 i = 0; i < to_read; ++i) {
                buf[i] = virtual_cmdline[offset + i];
            }
            return to_read;
        }

        auto* context = get_context();
        auto* user_fs = context->device_manager->get<FileSystem>(StringView("fs"));
        if (user_fs == nullptr) {
            return static_cast<u64>(-1);
        }
        return user_fs->read(path, offset, reinterpret_cast<u8*>(buf), size);
    }
    case Syscall::Num::WriteFile: {
        const char* path_str = reinterpret_cast<const char*>(arg1);
        const char* buf = reinterpret_cast<const char*>(arg2);
        u64 size = arg3;
        u64 offset = arg4;
        StringView path(path_str);

        if (!check_write_permission(current_uid, path)) {
            log(StringView("[Security] Access Denied: Write permission verification failed."));
            return static_cast<u64>(-1);
        }

        // Virtualized /Config/users
        if (path.equals(StringView("/Config/users"))) {
            if (offset + size > 1023) {
                return static_cast<u64>(-1);
            }
            for (u64 i = 0; i < size; ++i) {
                virtual_users_file[offset + i] = buf[i];
            }
            if (offset + size > virtual_users_size) {
                virtual_users_size = offset + size;
            }
            virtual_users_file[virtual_users_size] = '\0';
            return size;
        }

        // Virtualized /Config/cmdline
        if (path.equals(StringView("/Config/cmdline"))) {
            if (offset + size > 1023) {
                return static_cast<u64>(-1);
            }
            for (u64 i = 0; i < size; ++i) {
                virtual_cmdline[offset + i] = buf[i];
            }
            if (offset + size > virtual_cmdline_size) {
                virtual_cmdline_size = offset + size;
            }
            virtual_cmdline[virtual_cmdline_size] = '\0';
            return size;
        }

        return static_cast<u64>(-1);
    }
    default:
        return static_cast<u64>(-1);
    }
}

export void init_syscalls() {
    init_syscalls_arch();
}
