#pragma once

#include <stddef.h>

inline void* operator new(size_t, void* ptr) noexcept {
    return ptr;
}

inline void* operator new[](size_t, void* ptr) noexcept {
    return ptr;
}

void* operator new(size_t);
void* operator new[](size_t);

void operator delete(void*) noexcept;
void operator delete[](void*) noexcept;

void operator delete(void*, unsigned long) noexcept;
void operator delete[](void*, unsigned long) noexcept;

extern "C" inline int __cxa_atexit(void (*)(void*), void*, void*) {
    return 0;
}

extern "C" void __cxa_pure_virtual();

extern "C" void* memset(void* dst, int val, size_t n);
extern "C" void* memcpy(void* dst, const void* src, size_t n);
extern "C" void* memmove(void* dst, const void* src, size_t n);
