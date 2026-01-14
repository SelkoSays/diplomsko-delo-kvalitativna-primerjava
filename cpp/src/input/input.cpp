#include "input.hpp"
#include <unistd.h>
#include <cstring>
#include <cstdio>

namespace input {

InputHandler::InputHandler() = default;

bool InputHandler::read() {
    // Shift remaining data to start
    if (m_pos > 0 && m_pos < m_len) {
        std::memmove(m_buf.data(), m_buf.data() + m_pos, m_len - m_pos);
        m_len -= m_pos;
        m_pos = 0;
    } else if (m_pos >= m_len) {
        m_len = 0;
        m_pos = 0;
    }

    // Read more data
    int space = static_cast<int>(m_buf.size()) - m_len;
    if (space > 0) {
        int n = ::read(STDIN_FILENO, m_buf.data() + m_len, space);
        if (n > 0) m_len += n;
    }

    return m_len - m_pos > 0;
}

bool InputHandler::match(const char* seq, int len) const {
    if (m_pos + len > m_len) return false;
    return std::memcmp(m_buf.data() + m_pos, seq, len) == 0;
}

void InputHandler::consume(int n) {
    m_pos += n;
}

int InputHandler::remaining() const {
    return m_len - m_pos;
}

char InputHandler::peek(int offset) const {
    int idx = m_pos + offset;
    if (idx < 0 || idx >= m_len) return 0;
    return m_buf[idx];
}

bool InputHandler::parseMouseSgr(Event& ev) {
    if (!match("\x1b[<", 3)) return false;

    int end = -1;
    for (int i = 3; i < remaining(); i++) {
        char c = peek(i);
        if (c == 'M' || c == 'm') {
            end = i;
            break;
        }
        if (c < '0' || (c > '9' && c != ';')) return false;
    }
    if (end < 0) return false;

    // Parse: btn;x;y
    char tmp[32];
    int len = end - 3;
    if (len >= static_cast<int>(sizeof(tmp))) return false;
    std::memcpy(tmp, m_buf.data() + m_pos + 3, len);
    tmp[len] = '\0';

    int btn = 0, x = 0, y = 0;
    if (std::sscanf(tmp, "%d;%d;%d", &btn, &x, &y) != 3) return false;

    bool released = (peek(end) == 'm');

    MouseEvent mouse;
    mouse.x = x - 1;
    mouse.y = y - 1;
    mouse.mods = MOD_NONE;

    // Decode modifiers from button code
    if (btn & 4)  mouse.mods |= MOD_SHIFT;
    if (btn & 8)  mouse.mods |= MOD_ALT;
    if (btn & 16) mouse.mods |= MOD_CTRL;

    // Decode button and action
    int baseBtn = btn & 3;
    bool motion = (btn & 32) != 0;
    bool scroll = (btn & 64) != 0;

    if (scroll) {
        mouse.button = (baseBtn == 0) ? MouseButton::ScrollUp : MouseButton::ScrollDown;
        mouse.action = MouseAction::Press;
    } else if (motion) {
        mouse.button = (baseBtn == 3) ? MouseButton::None : static_cast<MouseButton>(baseBtn + 1);
        mouse.action = (baseBtn == 3) ? MouseAction::Move : MouseAction::Drag;
    } else {
        mouse.button = (baseBtn == 3) ? MouseButton::None : static_cast<MouseButton>(baseBtn + 1);
        mouse.action = released ? MouseAction::Release : MouseAction::Press;
    }

    ev = mouse;
    consume(end + 1);
    return true;
}

bool InputHandler::parseCsi(Event& ev) {
    if (!match("\x1b[", 2)) return false;

    if (peek(2) == '<') {
        return parseMouseSgr(ev);
    }

    int params[4] = {0};
    int paramCount = 0;
    int i = 2;
    int currentParam = 0;

    while (i < remaining()) {
        char c = peek(i);

        if (c >= '0' && c <= '9') {
            currentParam = currentParam * 10 + (c - '0');
            i++;
        } else if (c == ';') {
            if (paramCount < 4) params[paramCount++] = currentParam;
            currentParam = 0;
            i++;
        } else if (c >= 0x40 && c <= 0x7E) {
            // Final byte found
            if (paramCount < 4) params[paramCount++] = currentParam;

            KeyEvent key;
            key.mods = MOD_NONE;
            key.ch = {};

            // Decode modifiers from param
            if (paramCount >= 2 && params[1] > 1) {
                int mod = params[1] - 1;
                if (mod & MOD_SHIFT) key.mods |= MOD_SHIFT;
                if (mod & MOD_ALT) key.mods |= MOD_ALT;
                if (mod & MOD_CTRL) key.mods |= MOD_CTRL;
            }

            // Decode key
            switch (c) {
                case 'A': key.key = KeyCode::Up; break;
                case 'B': key.key = KeyCode::Down; break;
                case 'C': key.key = KeyCode::Right; break;
                case 'D': key.key = KeyCode::Left; break;
                case 'H': key.key = KeyCode::Home; break;
                case 'F': key.key = KeyCode::End; break;
                case '~':
                    switch (params[0]) {
                        case 1:  key.key = KeyCode::Home; break;
                        case 2:  key.key = KeyCode::Insert; break;
                        case 3:  key.key = KeyCode::Delete; break;
                        case 4:  key.key = KeyCode::End; break;
                        case 5:  key.key = KeyCode::PageUp; break;
                        case 6:  key.key = KeyCode::PageDown; break;
                        case 11: key.key = KeyCode::F1; break;
                        case 12: key.key = KeyCode::F2; break;
                        case 13: key.key = KeyCode::F3; break;
                        case 14: key.key = KeyCode::F4; break;
                        case 15: key.key = KeyCode::F5; break;
                        case 17: key.key = KeyCode::F6; break;
                        case 18: key.key = KeyCode::F7; break;
                        case 19: key.key = KeyCode::F8; break;
                        case 20: key.key = KeyCode::F9; break;
                        case 21: key.key = KeyCode::F10; break;
                        case 23: key.key = KeyCode::F11; break;
                        case 24: key.key = KeyCode::F12; break;
                        default: key.key = KeyCode::None; break;
                    }
                    break;
                default:
                    key.key = KeyCode::None;
                    break;
            }

            consume(i + 1);
            if (key.key != KeyCode::None) {
                ev = key;
                return true;
            }
            return false;
        } else {
            break;
        }
    }

    return false;
}

bool InputHandler::parseSs3(Event& ev) {
    if (!match("\x1bO", 2)) return false;
    if (remaining() < 3) return false;

    char c = peek(2);

    KeyEvent key;
    key.mods = MOD_NONE;
    key.ch = {};

    switch (c) {
        case 'P': key.key = KeyCode::F1; break;
        case 'Q': key.key = KeyCode::F2; break;
        case 'R': key.key = KeyCode::F3; break;
        case 'S': key.key = KeyCode::F4; break;
        case 'A': key.key = KeyCode::Up; break;
        case 'B': key.key = KeyCode::Down; break;
        case 'C': key.key = KeyCode::Right; break;
        case 'D': key.key = KeyCode::Left; break;
        case 'H': key.key = KeyCode::Home; break;
        case 'F': key.key = KeyCode::End; break;
        default:  return false;
    }

    ev = key;
    consume(3);
    return true;
}

bool InputHandler::parseAlt(Event& ev) {
    if (peek(0) != '\x1b') return false;
    if (remaining() < 2) return false;

    char c = peek(1);

    // Don't consume if it might be a CSI or SS3 sequence
    if (c == '[' || c == 'O') return false;

    KeyEvent key;
    key.mods = MOD_ALT;

    if (c >= 1 && c <= 26) {
        key.key = static_cast<KeyCode>('a' + c - 1);
        key.mods |= MOD_CTRL;
        key.ch = {};
    } else if (c >= 32 && c <= 126) {
        key.key = static_cast<KeyCode>(c);
        key.ch[0] = c;
        key.ch[1] = '\0';
    } else {
        return false;
    }

    ev = key;
    consume(2);
    return true;
}

bool InputHandler::parseEscape(Event& ev) {
    if (peek(0) != '\x1b') return false;

    if (remaining() == 1) {
        KeyEvent key;
        key.key = KeyCode::Escape;
        key.mods = MOD_NONE;
        key.ch = {};
        ev = key;
        consume(1);
        return true;
    }

    return false;
}

bool InputHandler::parseChar(Event& ev) {
    char c = peek(0);
    if (c == 0) return false;

    KeyEvent key;
    key.mods = MOD_NONE;

    // Handle special control chars
    switch (c) {
        case 0:    return false;
        case 9:    key.key = KeyCode::Tab; key.ch = {}; consume(1); ev = key; return true;
        case 10:   // fallthrough (LF)
        case 13:   key.key = KeyCode::Enter; key.ch = {}; consume(1); ev = key; return true;
        case 27:   return false;  // escape handled elsewhere
        case 127:  key.key = KeyCode::Backspace; key.ch = {}; consume(1); ev = key; return true;
        default:   break;
    }

    // Control characters (Ctrl+A through Ctrl+Z)
    if (c >= 1 && c <= 26) {
        key.key = static_cast<KeyCode>('a' + c - 1);
        key.mods = MOD_CTRL;
        key.ch = {};
        consume(1);
        ev = key;
        return true;
    }

    // Printable ASCII
    if (c >= 32 && c <= 126) {
        key.key = static_cast<KeyCode>(c);
        key.ch[0] = c;
        key.ch[1] = '\0';
        consume(1);
        ev = key;
        return true;
    }

    // UTF-8
    if (static_cast<unsigned char>(c) >= 0x80) {
        int len = 1;
        unsigned char first = static_cast<unsigned char>(c);
        if ((first & 0xE0) == 0xC0) len = 2;
        else if ((first & 0xF0) == 0xE0) len = 3;
        else if ((first & 0xF8) == 0xF0) len = 4;

        if (remaining() < len) return false;

        key.key = KeyCode::None;
        std::memcpy(key.ch.data(), m_buf.data() + m_pos, len);
        key.ch[len] = '\0';
        consume(len);
        ev = key;
        return true;
    }

    return false;
}

std::optional<Event> InputHandler::poll() {
    read();

    if (remaining() == 0) return std::nullopt;

    Event ev;

    if (parseCsi(ev)) return ev;
    if (parseSs3(ev)) return ev;
    if (parseAlt(ev)) return ev;
    if (parseEscape(ev)) return ev;
    if (parseChar(ev)) return ev;

    consume(1);
    return std::nullopt;
}

bool InputHandler::isKey(const Event& ev, KeyCode key) {
    if (auto* k = std::get_if<KeyEvent>(&ev)) {
        return k->key == key;
    }
    return false;
}

bool InputHandler::isChar(const Event& ev, char c) {
    if (auto* k = std::get_if<KeyEvent>(&ev)) {
        return k->isChar(c);
    }
    return false;
}

bool InputHandler::isCtrl(const Event& ev, char c) {
    if (auto* k = std::get_if<KeyEvent>(&ev)) {
        return k->isCtrl(c);
    }
    return false;
}

const char* InputHandler::keyName(KeyCode key) {
    switch (key) {
        case KeyCode::None: return "NONE";
        case KeyCode::Escape: return "ESC";
        case KeyCode::Enter: return "ENTER";
        case KeyCode::Tab: return "TAB";
        case KeyCode::Backspace: return "BACKSPACE";
        case KeyCode::Delete: return "DELETE";
        case KeyCode::Insert: return "INSERT";
        case KeyCode::Up: return "UP";
        case KeyCode::Down: return "DOWN";
        case KeyCode::Left: return "LEFT";
        case KeyCode::Right: return "RIGHT";
        case KeyCode::Home: return "HOME";
        case KeyCode::End: return "END";
        case KeyCode::PageUp: return "PGUP";
        case KeyCode::PageDown: return "PGDN";
        case KeyCode::F1: return "F1";
        case KeyCode::F2: return "F2";
        case KeyCode::F3: return "F3";
        case KeyCode::F4: return "F4";
        case KeyCode::F5: return "F5";
        case KeyCode::F6: return "F6";
        case KeyCode::F7: return "F7";
        case KeyCode::F8: return "F8";
        case KeyCode::F9: return "F9";
        case KeyCode::F10: return "F10";
        case KeyCode::F11: return "F11";
        case KeyCode::F12: return "F12";
        default:
            if (static_cast<u16>(key) >= 32 && static_cast<u16>(key) <= 126) {
                static char buf[2];
                buf[0] = static_cast<char>(static_cast<u16>(key));
                buf[1] = '\0';
                return buf;
            }
            return "?";
    }
}

} // namespace input
