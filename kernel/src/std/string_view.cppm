export module zep.std.string_view;

import zep.std.types;

export class StringView {
  public:
    string data = nullptr;
    usize lenght = 0;

    StringView() = default;

    explicit StringView(string data) : data(data) {
        if (data != nullptr) {
            usize count = 0;

            while (data[count] != '\0') {
                count++;
            }

            lenght = count;
        }
    }

    explicit StringView(string data, usize len) : data(data), lenght(len) {}

    usize length() const { return lenght; }

    bool equals(StringView other) const {
        if (lenght != other.lenght) {
            return false;
        }

        for (usize i = 0; i < lenght; ++i) {
            if (data[i] != other.data[i]) {
                return false;
            }
        }

        return true;
    }

    bool starts_with(StringView prefix) const {
        if (lenght < prefix.lenght) {
            return false;
        }

        for (usize i = 0; i < prefix.lenght; ++i) {
            if (data[i] != prefix.data[i]) {
                return false;
            }
        }

        return true;
    }
};
