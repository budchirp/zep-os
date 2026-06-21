module;

#include "runtime/runtime.h"

export module zep.gfx.renderer;

import zep.std.types;
import zep.std.math;

import zep.device.graphics.framebuffer;

import zep.gfx.font;
import zep.gfx.color;

export class Renderer {
  private:
    Framebuffer& framebuffer;

  public:
    explicit Renderer(Framebuffer& framebuffer) : framebuffer(framebuffer) {}

    void clear(Color color) { framebuffer.clear(color.value); }

    void pixel(Vec2u64 position, Color color) { framebuffer.write(position, color.value); }

    void sync() {
        if (framebuffer.front == nullptr || framebuffer.back == nullptr) {
            return;
        }

        usize total = framebuffer.resolution.y * framebuffer.pitch;

        for (usize i = 0; i < total; ++i) {
            framebuffer.front[i] = framebuffer.back[i];
        }
    }

    void rect(Vec2u64 position, Vec2u64 size, Color color) {
        for (u64 y = 0; y < size.y; ++y) {
            for (u64 x = 0; x < size.x; ++x) {
                framebuffer.write(Vec2u64(position.x + x, position.y + y), color.value);
            }
        }
    }

    void glyph(Vec2u64 position, char character, Color fg, Color bg) {
        if (character < 32 || character > 126) {
            return;
        }

        const u64 glyph_index = static_cast<u64>(character - 32);
        const auto& glyph_data = font[glyph_index];

        for (u64 row = 0; row < glyph_height; ++row) {
            const auto line = glyph_data[row];

            for (u64 col = 0; col < glyph_width; ++col) {
                const bool enabled = (line >> (7 - col)) & 1;

                framebuffer.write(Vec2u64(position.x + col, position.y + row),
                                  enabled ? fg.value : bg.value);
            }
        }
    }
};

alignas(Renderer) static unsigned char renderer_storage[sizeof(Renderer)];

export Renderer* init_renderer(Framebuffer* framebuffer) {
    return new (renderer_storage) Renderer(*framebuffer);
}
