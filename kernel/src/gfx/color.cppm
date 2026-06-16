export module zep.gfx.color;

import zep.std.types;

export class Color {
  public:
    u32 value;

    explicit Color(u32 value) : value(value) {}

    static const Color black() { return Color(0x000000); }
    static const Color white() { return Color(0xFFFFFF); }
    static const Color red() { return Color(0xFF0000); }
    static const Color green() { return Color(0x00FF00); }
    static const Color blue() { return Color(0x0000FF); }
};
