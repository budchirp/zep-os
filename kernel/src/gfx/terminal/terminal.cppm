module;

#include "std/runtime.h"

export module zep.gfx.terminal;

import zep.std.types;
import zep.device.display.framebuffer;
import zep.gfx.font;
import zep.gfx.terminal.color;

export class Terminal {
  private:
    Framebuffer& framebuffer;

    u64 cursor_x;
    u64 cursor_y;

    u64 columns;
    u64 rows;

    void draw_glyph(u64 x, u64 y, char c) {
        if (c < 32 || c > 126) {
            return;
        }
        auto index = static_cast<u64>(static_cast<u8>(c) - 32);
        auto& glyph = font[index];
        for (u64 row = 0; row < glyph_height; ++row) {
            u8 line = glyph[row];
            for (u64 col = 0; col < glyph_width; ++col) {
                bool pixel_on = (line >> (7 - col)) & 1;
                framebuffer.write(x + col, y + row, pixel_on ? fg_color.value : bg_color.value);
            }
        }
    }

    void newline() {
        cursor_x = 0;
        cursor_y += 1;
        if (cursor_y >= rows) {
            scroll();
            cursor_y = rows - 1;
        }
    }

    void scroll() {
        auto row_bytes = framebuffer.pitch * glyph_height;
        auto framebuffer_size_bytes = framebuffer.pitch * framebuffer.height;
        for (u64 offset = 0; offset + row_bytes < framebuffer_size_bytes; ++offset) {
            framebuffer.base[offset] = framebuffer.base[offset + row_bytes];
        }
        for (u64 y = rows - 1; y < rows; ++y) {
            for (u64 x = 0; x < columns; ++x) {
                for (u64 py = 0; py < glyph_height; ++py) {
                    for (u64 px = 0; px < glyph_width; ++px) {
                        framebuffer.write(x * glyph_width + px, y * glyph_height + py,
                                          bg_color.value);
                    }
                }
            }
        }
    }

    void print_char(char character) {
        if (character == '\n') {
            newline();
            return;
        }
        if (character == '\r') {
            cursor_x = 0;
            return;
        }
        draw_glyph(cursor_x * glyph_width, cursor_y * glyph_height, character);
        cursor_x += 1;
        if (cursor_x >= columns) {
            newline();
        }
    }

  public:
    Color fg_color;
    Color bg_color;

    explicit Terminal(Framebuffer& framebuffer)
        : framebuffer(framebuffer), cursor_x(0), cursor_y(0),
          columns(framebuffer.width / glyph_width), rows(framebuffer.height / glyph_height),
          fg_color(Color::white()), bg_color(Color::black()) {}

    void clear() {
        framebuffer.clear(bg_color.value);

        cursor_x = 0;
        cursor_y = 0;
    }

    void print(string str) {
        for (auto* character = str; *character != '\0'; ++character) {
            print_char(*character);
        }
    }
};

alignas(Terminal) static unsigned char terminal_storage[sizeof(Terminal)];

export Terminal* init_terminal(Framebuffer* framebuffer) {
    auto terminal = new (terminal_storage) Terminal(*framebuffer);

    terminal->clear();

    return terminal;
}

export extern "C" void zep_terminal_clear(Terminal* terminal) {
    terminal->clear();
}

export extern "C" void zep_terminal_print(Terminal* terminal, string str) {
    terminal->print(str);
}
