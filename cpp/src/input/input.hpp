#pragma once

#include "../common.hpp"

namespace input {

// Special key codes (values chosen to not conflict with ASCII)
enum class KeyCode : u16 {
    None = 0,

    // ASCII printable range: 32-126 stored directly
    // Control characters: 1-31 (Ctrl+A = 1, etc.)

    // Special keys start at 256
    Escape = 256,
    Enter,
    Tab,
    Backspace,
    Delete,
    Insert,

    Up,
    Down,
    Left,
    Right,

    Home,
    End,
    PageUp,
    PageDown,

    F1, F2, F3, F4,
    F5, F6, F7, F8,
    F9, F10, F11, F12,
};

// Modifier flags
enum Modifier : u8 {
    MOD_NONE  = 0,
    MOD_SHIFT = 1 << 0,
    MOD_ALT   = 1 << 1,
    MOD_CTRL  = 1 << 2,
};

// Mouse button codes
enum class MouseButton : u8 {
    None = 0,
    Left,
    Middle,
    Right,
    ScrollUp,
    ScrollDown,
};

// Mouse event types
enum class MouseAction : u8 {
    Press,
    Release,
    Move,
    Drag,
};

struct KeyEvent {
    KeyCode key = KeyCode::None;
    u8 mods = MOD_NONE;
    std::array<char, 5> ch = {};  // UTF-8 character if printable

    bool isChar(char c) const {
        if (mods != MOD_NONE) return false;
        char lower_c = (c >= 'A' && c <= 'Z') ? c + 32 : c;
        u16 k = static_cast<u16>(key);
        char lower_k = (k >= 'A' && k <= 'Z') ? k + 32 : k;
        return lower_k == static_cast<u16>(lower_c);
    }

    bool isKey(KeyCode k) const {
        return key == k;
    }

    bool isCtrl(char c) const {
        char lower = (c >= 'A' && c <= 'Z') ? c + 32 : c;
        return static_cast<u16>(key) == static_cast<u16>(lower) && (mods & MOD_CTRL);
    }
};

struct MouseEvent {
    MouseButton button = MouseButton::None;
    MouseAction action = MouseAction::Press;
    int x = 0, y = 0;
    u8 mods = MOD_NONE;
};

// Event as variant
using Event = std::variant<std::monostate, KeyEvent, MouseEvent>;

// Input handler with buffered reading
class InputHandler {
public:
    InputHandler();

    std::optional<Event> poll();

    // Convenience static methods
    static bool isKey(const Event& ev, KeyCode key);
    static bool isChar(const Event& ev, char c);
    static bool isCtrl(const Event& ev, char c);
    static const char* keyName(KeyCode key);

private:
    bool read();
    bool match(const char* seq, int len) const;
    void consume(int n);
    int remaining() const;
    char peek(int offset) const;

    bool parseCsi(Event& ev);
    bool parseMouseSgr(Event& ev);
    bool parseSs3(Event& ev);
    bool parseAlt(Event& ev);
    bool parseEscape(Event& ev);
    bool parseChar(Event& ev);

    std::array<char, 64> m_buf = {};
    int m_len = 0;
    int m_pos = 0;
};

} // namespace input
