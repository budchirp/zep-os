module;

#include <common/types.h>

export module zep.std.types;

export using ::u8;
export using ::u16;
export using ::u32;
export using ::u64;

export using ::i8;
export using ::i16;
export using ::i32;
export using ::i64;

export using string = const char*;

export using ::usize;
export using isize = decltype(static_cast<usize>(0) - static_cast<usize>(0));
export using ::uintptr;
