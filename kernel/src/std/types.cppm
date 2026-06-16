export module zep.std.types;

export using u8 = unsigned char;
export using u16 = unsigned short;
export using u32 = unsigned int;
export using u64 = unsigned long long;

export using i8 = signed char;
export using i16 = signed short;
export using i32 = signed int;
export using i64 = signed long long;

export using string = const char*;

export using usize = decltype(sizeof(0));
export using isize = decltype(static_cast<usize>(0) - static_cast<usize>(0));
export using uintptr = unsigned long long;
