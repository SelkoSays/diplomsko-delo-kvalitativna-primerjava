#include "terminal.hpp"
#include <unistd.h>
#include <sys/ioctl.h>
#include <cstdio>

namespace tui {

Terminal::Terminal() {
    // Get original terminal settings
    if (tcgetattr(STDIN_FILENO, &m_origTermios) < 0) {
        return;
    }

    // Configure raw mode
    struct termios raw = m_origTermios;
    raw.c_lflag &= ~(ECHO | ICANON | ISIG);  // no echo, canonical mode, or signals
    raw.c_iflag &= ~(IXON | ICRNL);          // no flow control, no CR->NL
    raw.c_cc[VMIN] = 0;                       // non-blocking read
    raw.c_cc[VTIME] = 0;

    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) < 0) {
        return;
    }

    // Enter alternate screen, hide cursor, enable mouse
    std::printf("%s%s%s%s",
                esc::ALT_SCREEN_ON,
                esc::CURSOR_HIDE,
                esc::MOUSE_ON,
                esc::CLEAR_SCREEN);
    std::fflush(stdout);

    m_initialized = true;
}

Terminal::~Terminal() {
    if (!m_initialized) return;

    // Restore terminal
    std::printf("%s%s%s%s",
                esc::RESET_ATTRS,
                esc::MOUSE_OFF,
                esc::CURSOR_SHOW,
                esc::ALT_SCREEN_OFF);
    std::fflush(stdout);

    tcsetattr(STDIN_FILENO, TCSAFLUSH, &m_origTermios);
}

std::pair<int, int> Terminal::size() const {
    struct winsize ws;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) < 0) {
        return {80, 24};  // fallback
    }
    return {ws.ws_col, ws.ws_row};
}

} // namespace tui
