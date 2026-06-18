#include "runtime.h"

void* operator new(size_t) {
    return nullptr;
}

void operator delete(void*, unsigned long) noexcept {}

void operator delete(void*) noexcept {}

extern "C" void* memset(void* dst, int val, size_t n) {
    auto* p = static_cast<unsigned char*>(dst);
    for (size_t i = 0; i < n; ++i) {
        p[i] = static_cast<unsigned char>(val);
    }
    return dst;
}

extern "C" void __cxa_pure_virtual() {
    while (true) {
        __builtin_trap();
    }
}
