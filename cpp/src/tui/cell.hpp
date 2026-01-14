#pragma once

#include "color.hpp"
#include <cstring>

namespace tui {

struct Cell {
    std::array<char, 5> ch = {' ', '\0', '\0', '\0', '\0'};  // UTF-8 char + null
    Color fg = Color::None();
    Color bg = Color::None();
    u8 attrs = ATTR_NONE;

    Cell() = default;

    void setChar(std::string_view s) {
        ch.fill('\0');
        auto len = std::min(s.size(), size_t(4));
        std::memcpy(ch.data(), s.data(), len);
    }

    void clear() {
        ch = {' ', '\0', '\0', '\0', '\0'};
        fg = Color::None();
        bg = Color::None();
        attrs = ATTR_NONE;
    }

    bool operator==(const Cell& other) const {
        return fg == other.fg && bg == other.bg &&
               attrs == other.attrs && ch == other.ch;
    }

    bool operator!=(const Cell& other) const {
        return !(*this == other);
    }
};

} // namespace tui
