#ifndef ZEP_COMMON_SYSCALLS_H
#define ZEP_COMMON_SYSCALLS_H

#include <common/types.h>

class Syscall {
public:
    enum class Num : u64 {
        Allocate = 1,
        Free = 2,
        Print = 3,
        Log = 4,
        Error = 5,
        Panic = 6,
        Halt = 7,
        Spawn = 8,
        Kill = 9,
        Exit = 10,
        GetPid = 11,
        ReadChar = 12,
        SetUid = 13,
        GetUid = 14,
        ReadFile = 15,
        WriteFile = 16
    };
};

#endif // ZEP_COMMON_SYSCALLS_H
