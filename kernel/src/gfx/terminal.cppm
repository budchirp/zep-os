module;

#include "runtime/runtime.h"

export module zep.gfx.terminal;

import zep.std.types;
import zep.std.math;

import zep.gfx.renderer;
import zep.gfx.font;
import zep.gfx.color;

export class Terminal {
  private:
    Renderer& renderer;

    Vec2u64 cursor;

    u64 columns;
    u64 rows;

    void newline() {
        cursor.x = 0;
        cursor.y += 1;

        if (cursor.y >= rows) {
            cursor.y = rows - 1;
        }
    }

    void print_char(char character, Color color) {
        if (character == '\n') {
            newline();
            return;
        }

        if (character == '\r') {
            cursor.x = 0;
            return;
        }

        renderer.glyph(Vec2u64(cursor.x * glyph_width, cursor.y * glyph_height), character, color,
                       bg_color);

        cursor.x += 1;

        if (cursor.x >= columns) {
            newline();
        }
    }

  public:
    Color fg_color;
    Color bg_color;

    explicit Terminal(Renderer& renderer, Vec2u64 resolution)
        : renderer(renderer), cursor(0, 0), columns(resolution.x / glyph_width),
          rows(resolution.y / glyph_height), fg_color(Color::white()), bg_color(Color::black()) {}

    void clear() {
        renderer.clear(bg_color);

        cursor = Vec2u64(0, 0);
    }

    void print(string str, Color color) {
        for (const auto* character = str; *character != '\0'; ++character) {
            print_char(*character, color);
        }
    }

    void print(string str) { print(str, fg_color); }
};

alignas(Terminal) static unsigned char terminal_storage[sizeof(Terminal)];

export Terminal* init_terminal(Renderer* renderer, Vec2u64 resolution) {
    auto terminal = new (terminal_storage) Terminal(*renderer, resolution);

    terminal->clear();

    return terminal;
}

export extern "C" void zep_terminal_clear(Terminal* terminal) {
    terminal->clear();
}

export extern "C" void zep_terminal_print(Terminal* terminal, string str) {
    terminal->print(str);
}
