module;

#include "runtime/runtime.h"

export module zep.device.graphics.framebuffer;

import zep.device;
import zep.std.types;
import zep.std.string_view;
import zep.std.math;

export class Framebuffer : public Device {
  private:
  public:
    using Device::write;

    u8* front = nullptr;
    u8* back = nullptr;

    Vec2u64 resolution;

    u64 pitch;
    u16 bpp;

    explicit Framebuffer(u8* front, u8* back, Vec2u64 resolution, u64 pitch, u16 bpp)
        : front(front), back(back), resolution(resolution), pitch(pitch), bpp(bpp) {}

    ~Framebuffer() override = default;

    StringView name() override { return StringView("framebuffer"); }

    void write(Vec2u64 position, u32 color) {
        if (position.x >= resolution.x || position.y >= resolution.y) {
            return;
        }

        u64 offset = position.y * pitch + position.x * 4;

        *reinterpret_cast<u32*>(back + offset) = color;
    }

    void clear(u32 color) {
        for (u64 y = 0; y < resolution.y; ++y) {
            for (u64 x = 0; x < resolution.x; ++x) {
                write(Vec2u64(x, y), color);
            }
        }
    }

    Vec2u64 size() const { return resolution; }
};
