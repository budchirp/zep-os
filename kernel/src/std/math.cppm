export module zep.std.math;

import zep.std.types;

export template <typename T>
class Vector2 {
  public:
    T x;
    T y;

    constexpr Vector2() : x(), y() {}

    constexpr explicit Vector2(T x, T y) : x(x), y(y) {}
};

export template <typename T>
class Vector3 {
  public:
    T x;
    T y;
    T z;

    constexpr Vector3() : x(), y(), z() {}

    constexpr explicit Vector3(T x, T y, T z) : x(x), y(y), z(z) {}
};

export using Vec2i8 = Vector2<i8>;
export using Vec2i16 = Vector2<i16>;
export using Vec2i32 = Vector2<i32>;
export using Vec2i64 = Vector2<i64>;

export using Vec2u8 = Vector2<u8>;
export using Vec2u16 = Vector2<u16>;
export using Vec2u32 = Vector2<u32>;
export using Vec2u64 = Vector2<u64>;

export using Vec3i8 = Vector3<i8>;
export using Vec3i16 = Vector3<i16>;
export using Vec3i32 = Vector3<i32>;
export using Vec3i64 = Vector3<i64>;

export using Vec3u8 = Vector3<u8>;
export using Vec3u16 = Vector3<u16>;
export using Vec3u32 = Vector3<u32>;
export using Vec3u64 = Vector3<u64>;
