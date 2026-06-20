#include "runtime.h"

extern "C" void* kernel_allocate(size_t size);
extern "C" void kernel_deallocate(void* ptr);

void* operator new(size_t size) {
    return kernel_allocate(size);
}

void* operator new[](size_t size) {
    return kernel_allocate(size);
}

void operator delete(void* ptr) noexcept {
    kernel_deallocate(ptr);
}

void operator delete[](void* ptr) noexcept {
    kernel_deallocate(ptr);
}

void operator delete(void* ptr, unsigned long) noexcept {
    kernel_deallocate(ptr);
}

void operator delete[](void* ptr, unsigned long) noexcept {
    kernel_deallocate(ptr);
}

extern "C" void* memset(void* dst, int val, size_t n) {
    auto* p = static_cast<unsigned char*>(dst);

    for (size_t i = 0; i < n; ++i) {
        p[i] = static_cast<unsigned char>(val);
    }

    return dst;
}

extern "C" void* memcpy(void* dst, const void* src, size_t n) {
    auto* d = static_cast<unsigned char*>(dst);
    const auto* s = static_cast<const unsigned char*>(src);

    for (size_t i = 0; i < n; ++i) {
        d[i] = s[i];
    }

    return dst;
}

extern "C" void* memmove(void* dst, const void* src, size_t n) {
    auto* d = static_cast<unsigned char*>(dst);
    const auto* s = static_cast<const unsigned char*>(src);

    if (d < s) {
        for (size_t i = 0; i < n; ++i) {
            d[i] = s[i];
        }
    } else if (d > s) {
        for (size_t i = n; i > 0; --i) {
            d[i - 1] = s[i - 1];
        }
    }

    return dst;
}

extern "C" void __cxa_pure_virtual() {
    while (true) {
        __builtin_trap();
    }
}
