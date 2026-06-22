#ifndef ZEP_COMMON_STRING_VIEW_H
#define ZEP_COMMON_STRING_VIEW_H

#include <common/types.h>

class StringView {
public:
    const char* data = nullptr;
    usize size = 0;

    constexpr StringView() = default;

    constexpr explicit StringView(const char* data) : data(data) {
        if (data != nullptr) {
            usize count = 0;
            while (data[count] != '\0') {
                count++;
            }
            size = count;
        }
    }

    constexpr explicit StringView(const char* data, usize len) : data(data), size(len) {}

    constexpr usize length() const {
        return size;
    }

    constexpr bool equals(StringView other) const {
        if (size != other.size) {
            return false;
        }
        for (usize i = 0; i < size; ++i) {
            if (data[i] != other.data[i]) {
                return false;
            }
        }
        return true;
    }

    constexpr bool starts_with(StringView prefix) const {
        if (size < prefix.size) {
            return false;
        }
        for (usize i = 0; i < prefix.size; ++i) {
            if (data[i] != prefix.data[i]) {
                return false;
            }
        }
        return true;
    }
};

#endif // ZEP_COMMON_STRING_VIEW_H
