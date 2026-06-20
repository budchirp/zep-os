export module zep.std.map;

import zep.std.types;
import zep.std.string_view;

export template <typename V>
class Map {
  private:
    static constexpr usize max_entries = 32;

    StringView keys[max_entries];
    V values[max_entries] = {};
    usize count = 0;

  public:
    Map() = default;

    bool insert(StringView key, V value) {
        for (usize i = 0; i < count; ++i) {
            if (keys[i].equals(key)) {
                values[i] = value;
                return true;
            }
        }

        if (count >= max_entries) {
            return false;
        }

        keys[count] = key;
        values[count] = value;
        count++;
        return true;
    }

    V find(StringView key) {
        for (usize i = 0; i < count; ++i) {
            if (keys[i].equals(key)) {
                return values[i];
            }
        }

        return V{};
    }
};
