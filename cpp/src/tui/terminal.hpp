#pragma once

#include "../common.hpp"
#include <termios.h>
#include <utility>

namespace tui {

// RAII terminal management - handles raw mode lifecycle
class Terminal {
public:
    Terminal();
    ~Terminal();

    // Non-copyable, non-movable
    Terminal(const Terminal&) = delete;
    Terminal& operator=(const Terminal&) = delete;
    Terminal(Terminal&&) = delete;
    Terminal& operator=(Terminal&&) = delete;

    std::pair<int, int> size() const;

private:
    struct termios m_origTermios;
    bool m_initialized = false;
};

// ANSI escape sequences
namespace esc {
    constexpr const char* ALT_SCREEN_ON  = "\x1b[?1049h";
    constexpr const char* ALT_SCREEN_OFF = "\x1b[?1049l";
    constexpr const char* CURSOR_HIDE    = "\x1b[?25l";
    constexpr const char* CURSOR_SHOW    = "\x1b[?25h";
    constexpr const char* MOUSE_ON       = "\x1b[?1003h\x1b[?1006h";
    constexpr const char* MOUSE_OFF      = "\x1b[?1003l\x1b[?1006l";
    constexpr const char* CLEAR_SCREEN   = "\x1b[2J";
    constexpr const char* RESET_ATTRS    = "\x1b[0m";
    constexpr const char* HOME           = "\x1b[H";
}

} // namespace tui
