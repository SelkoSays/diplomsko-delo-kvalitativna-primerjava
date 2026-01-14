#pragma once

#include "../common.hpp"

namespace tui {

// Color format: 0xAARRGGBB
// A = 0: don't apply this color (transparent/default)
// A != 0: apply this color
struct Color {
    u32 value;

    constexpr Color() : value(0) {}
    constexpr explicit Color(u32 v) : value(v) {}

    static constexpr Color None() { return Color(0x00000000); }
    static constexpr Color Black() { return Color(0xFF000000); }
    static constexpr Color White() { return Color(0xFFFFFFFF); }
    static constexpr Color Red() { return Color(0xFFFF0000); }
    static constexpr Color Green() { return Color(0xFF00FF00); }
    static constexpr Color Blue() { return Color(0xFF0000FF); }
    static constexpr Color Yellow() { return Color(0xFFFFFF00); }
    static constexpr Color Cyan() { return Color(0xFF00FFFF); }
    static constexpr Color Magenta() { return Color(0xFFFF00FF); }
    static constexpr Color Gray() { return Color(0xFF808080); }

    static constexpr Color RGB(u8 r, u8 g, u8 b) {
        return Color(0xFF000000 | (static_cast<u32>(r) << 16) |
                     (static_cast<u32>(g) << 8) | static_cast<u32>(b));
    }

    static constexpr Color RGBA(u8 r, u8 g, u8 b, u8 a) {
        return Color((static_cast<u32>(a) << 24) | (static_cast<u32>(r) << 16) |
                     (static_cast<u32>(g) << 8) | static_cast<u32>(b));
    }

    constexpr u8 a() const { return (value >> 24) & 0xFF; }
    constexpr u8 r() const { return (value >> 16) & 0xFF; }
    constexpr u8 g() const { return (value >> 8) & 0xFF; }
    constexpr u8 b() const { return value & 0xFF; }

    constexpr bool isSet() const { return (value >> 24) != 0; }

    constexpr bool operator==(const Color& other) const { return value == other.value; }
    constexpr bool operator!=(const Color& other) const { return value != other.value; }
};

// Text attributes
enum Attr : u8 {
    ATTR_NONE      = 0,
    ATTR_BOLD      = 1 << 0,
    ATTR_DIM       = 1 << 1,
    ATTR_ITALIC    = 1 << 2,
    ATTR_UNDERLINE = 1 << 3,
    ATTR_BLINK     = 1 << 4,
    ATTR_REVERSE   = 1 << 5,
    ATTR_CROSSED   = 1 << 6,
};

} // namespace tui
