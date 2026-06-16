export module zep.gfx.framebuffer;

import zep.std.types;

export class Framebuffer {
  public:
    u8* base;

    u64 width;
    u64 height;

    u64 pitch;
    u16 bpp;

    explicit Framebuffer(void* address, u64 width, u64 height, u64 pitch, u16 bpp)
        : base(static_cast<u8*>(address)), width(width), height(height), pitch(pitch), bpp(bpp) {}

    void write(u64 x, u64 y, u32 color) {
        if (x >= width || y >= height) {
            return;
        }
        auto offset = y * pitch + x * 4;
        auto* pixel = reinterpret_cast<u32*>(base + offset);
        *pixel = color;
    }

    void clear(u32 color) {
        for (u64 y = 0; y < height; ++y) {
            for (u64 x = 0; x < width; ++x) {
                write(x, y, color);
            }
        }
    }
};

export extern "C" void zep_framebuffer_write(Framebuffer* framebuffer, u64 x, u64 y, u32 color) {
    framebuffer->write(x, y, color);
}

export extern "C" void zep_framebuffer_clear(Framebuffer* framebuffer, u32 color) {
    framebuffer->clear(color);
}
