export module zep.std.types;

export namespace zep {
using u8 = unsigned char;
using u16 = unsigned short;
using u32 = unsigned int;
using u64 = unsigned long long;

using i8 = signed char;
using i16 = signed short;
using i32 = signed int;
using i64 = signed long long;

using string = const char*;

using usize = decltype(sizeof(0));
using isize = decltype(static_cast<usize>(0) - static_cast<usize>(0));
using uintptr = unsigned long long;
} // namespace zep
