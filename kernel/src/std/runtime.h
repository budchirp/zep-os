#pragma once

#include <stddef.h>

inline void* operator new(size_t, void* ptr) noexcept {
    return ptr;
}

inline void* operator new[](size_t, void* ptr) noexcept {
    return ptr;
}

extern "C" inline int __cxa_atexit(void (*)(void*), void*, void*) {
    return 0;
}
