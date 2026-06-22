#include <common/string_view.h>
#include <common/syscalls.h>

extern "C" {

unsigned long long make_syscall(unsigned long long num, unsigned long long arg1 = 0, unsigned long long arg2 = 0, unsigned long long arg3 = 0, unsigned long long arg4 = 0);

unsigned long long strlen(const char* str) {
    if (str == nullptr) {
        return 0;
    }
    unsigned long long len = 0;
    while (str[len] != '\0') {
        len++;
    }
    return len;
}

void print(const char* str) {
    make_syscall(static_cast<unsigned long long>(Syscall::Num::Print), reinterpret_cast<unsigned long long>(str), strlen(str));
}

[[noreturn]] void panic(const char* str) {
    make_syscall(static_cast<unsigned long long>(Syscall::Num::Panic), reinterpret_cast<unsigned long long>(str), strlen(str));
    while (true) {}
}

[[noreturn]] void halt() {
    make_syscall(static_cast<unsigned long long>(Syscall::Num::Halt));
    while (true) {}
}

void* kernel_allocate(unsigned long long size) {
    return reinterpret_cast<void*>(make_syscall(static_cast<unsigned long long>(Syscall::Num::Allocate), size));
}

void kernel_deallocate(void* ptr) {
    make_syscall(static_cast<unsigned long long>(Syscall::Num::Free), reinterpret_cast<unsigned long long>(ptr));
}

unsigned int spawn(const char* path, const char** argv) {
    return static_cast<unsigned int>(make_syscall(static_cast<unsigned long long>(Syscall::Num::Spawn), reinterpret_cast<unsigned long long>(path), reinterpret_cast<unsigned long long>(argv)));
}

int kill(unsigned int pid) {
    return static_cast<int>(make_syscall(static_cast<unsigned long long>(Syscall::Num::Kill), pid));
}

[[noreturn]] void exit(int code) {
    make_syscall(static_cast<unsigned long long>(Syscall::Num::Exit), static_cast<unsigned long long>(code));
    while (true) {}
}

unsigned int getpid() {
    return static_cast<unsigned int>(make_syscall(static_cast<unsigned long long>(Syscall::Num::GetPid)));
}

char read_char() {
    return static_cast<char>(make_syscall(static_cast<unsigned long long>(Syscall::Num::ReadChar)));
}

int setuid(unsigned int uid) {
    return static_cast<int>(make_syscall(static_cast<unsigned long long>(Syscall::Num::SetUid), uid));
}

unsigned int getuid() {
    return static_cast<unsigned int>(make_syscall(static_cast<unsigned long long>(Syscall::Num::GetUid)));
}

long long read_file(const char* path, char* buffer, unsigned long long size, unsigned long long offset) {
    return static_cast<long long>(make_syscall(
        static_cast<unsigned long long>(Syscall::Num::ReadFile),
        reinterpret_cast<unsigned long long>(path),
        reinterpret_cast<unsigned long long>(buffer),
        size,
        offset
    ));
}

long long write_file(const char* path, const char* buffer, unsigned long long size, unsigned long long offset) {
    return static_cast<long long>(make_syscall(
        static_cast<unsigned long long>(Syscall::Num::WriteFile),
        reinterpret_cast<unsigned long long>(path),
        reinterpret_cast<unsigned long long>(buffer),
        size,
        offset
    ));
}

void print_buffer(unsigned long long buf) {
    print(reinterpret_cast<const char*>(buf));
}

void* allocate(unsigned long long size) {
    return reinterpret_cast<void*>(make_syscall(static_cast<unsigned long long>(Syscall::Num::Allocate), size));
}

void free(void* ptr) {
    make_syscall(static_cast<unsigned long long>(Syscall::Num::Free), reinterpret_cast<unsigned long long>(ptr));
}

int strcmp(const char* s1, const char* s2) {
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return *(const unsigned char*)s1 - *(const unsigned char*)s2;
}

int strncmp(const char* s1, const char* s2, unsigned long long n) {
    while (n && *s1 && (*s1 == *s2)) {
        s1++;
        s2++;
        n--;
    }
    if (n == 0) {
        return 0;
    }
    return *(const unsigned char*)s1 - *(const unsigned char*)s2;
}

unsigned int parse_int(const char* str) {
    unsigned int val = 0;
    while (*str >= '0' && *str <= '9') {
        val = val * 10 + (*str - '0');
        str++;
    }
    return val;
}

void print_char(char c) {
    char buf[2] = {c, '\0'};
    print(buf);
}

void readline(char* buf, int max_len) {
    int index = 0;
    while (index < max_len - 1) {
        char c = read_char();
        if (c == '\n' || c == '\r') {
            print("\n");
            break;
        } else if (c == '\b' || c == 127) {
            if (index > 0) {
                index--;
                print("\b \b");
            }
        } else {
            buf[index++] = c;
        }
    }
    buf[index] = '\0';
}

} // extern "C"

// C++ APIs
void print(StringView str) {
    make_syscall(static_cast<unsigned long long>(Syscall::Num::Print), reinterpret_cast<unsigned long long>(str.data), str.size);
}

void log(StringView str) {
    make_syscall(static_cast<unsigned long long>(Syscall::Num::Log), reinterpret_cast<unsigned long long>(str.data), str.size);
}

void error(StringView str) {
    make_syscall(static_cast<unsigned long long>(Syscall::Num::Error), reinterpret_cast<unsigned long long>(str.data), str.size);
}

void panic(StringView str) {
    make_syscall(static_cast<unsigned long long>(Syscall::Num::Panic), reinterpret_cast<unsigned long long>(str.data), str.size);
}
