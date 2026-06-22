module;

#include "runtime/runtime.h"

export module zep.device.keyboard;

import zep.std.types;

extern "C" void print(string str);

export class Keyboard {
  private:
    static inline bool shift_pressed = false;

    static inline const char scancode_map[128] = {
        0,   27,  '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
        '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
        0,   'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0,
        '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0,   '*', 0,   ' '
    };

    static inline const char scancode_map_shift[128] = {
        0,   27,  '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b',
        '\t', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n',
        0,   'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~', 0,
        '|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', 0,   '*', 0,   ' '
    };

  public:
    static void handle_scancode(u8 scancode) {
        if (scancode == 0x2A || scancode == 0x36) {
            shift_pressed = true;
            return;
        }

        if (scancode == 0xAA || scancode == 0xB6) {
            shift_pressed = false;
            return;
        }

        if ((scancode & 0x80) != 0) {
            return;
        }

        if (scancode < 128) {
            char c = shift_pressed ? scancode_map_shift[scancode] : scancode_map[scancode];
            if (c != 0) {
                char str[2] = {c, '\0'};
                print(str);
            }
        }
    }
};
