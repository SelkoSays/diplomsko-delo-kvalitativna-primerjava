#include "screen.hpp"
#include <unistd.h>
#include <cstdio>
#include <cstring>

namespace tui {

Screen::Screen() {
    auto [w, h] = m_terminal.size();
    m_width = w;
    m_height = h;

    int size = m_width * m_height;
    m_back.resize(size);
    m_front.resize(size);
}

Cell* Screen::cell(int x, int y) {
    if (x < 0 || x >= m_width || y < 0 || y >= m_height) return nullptr;
    return &m_back[y * m_width + x];
}

const Cell* Screen::cell(int x, int y) const {
    if (x < 0 || x >= m_width || y < 0 || y >= m_height) return nullptr;
    return &m_back[y * m_width + x];
}

void Screen::clear() {
    for (auto& c : m_back) {
        c.clear();
    }
}

void Screen::putChar(int x, int y, std::string_view ch) {
    Cell* c = cell(x, y);
    if (!c) return;

    if (ch.empty()) {
        c->ch = {' ', '\0', '\0', '\0', '\0'};
        return;
    }

    unsigned char first = static_cast<unsigned char>(ch[0]);
    int len = 1;
    if ((first & 0x80) == 0) len = 1;
    else if ((first & 0xE0) == 0xC0) len = 2;
    else if ((first & 0xF0) == 0xE0) len = 3;
    else if ((first & 0xF8) == 0xF0) len = 4;

    len = std::min(len, static_cast<int>(ch.size()));
    c->ch.fill('\0');
    std::memcpy(c->ch.data(), ch.data(), len);
}

void Screen::putString(int x, int y, std::string_view str) {
    int cx = x;
    int cy = y;
    size_t i = 0;

    while (i < str.size() && cy < m_height) {
        unsigned char first = static_cast<unsigned char>(str[i]);

        if (first < 0x80) {
            if (first == '\n') {
                cy++;
                cx = x;
                i++;
                continue;
            }
            if (first < 0x20 || first == 0x7F) {
                i++;
                continue;
            }
        }

        if (cx >= m_width) {
            i++;
            continue;
        }

        int len = 1;
        if ((first & 0x80) == 0) len = 1;
        else if ((first & 0xE0) == 0xC0) len = 2;
        else if ((first & 0xF0) == 0xE0) len = 3;
        else if ((first & 0xF8) == 0xF0) len = 4;

        putChar(cx, cy, str.substr(i, len));
        i += len;
        cx++;
    }
}

void Screen::setFgColor(int x, int y, Color color) {
    Cell* c = cell(x, y);
    if (c) c->fg = color;
}

void Screen::setBgColor(int x, int y, Color color) {
    Cell* c = cell(x, y);
    if (c) c->bg = color;
}

void Screen::setAttr(int x, int y, u8 attrs) {
    Cell* c = cell(x, y);
    if (c) c->attrs = attrs;
}

void Screen::fill(int x, int y, int w, int h, std::string_view ch) {
    for (int cy = y; cy < y + h && cy < m_height; cy++) {
        for (int cx = x; cx < x + w && cx < m_width; cx++) {
            putChar(cx, cy, ch);
        }
    }
}

void Screen::fillColor(int x, int y, int w, int h, Color fg, Color bg) {
    for (int cy = y; cy < y + h && cy < m_height; cy++) {
        for (int cx = x; cx < x + w && cx < m_width; cx++) {
            Cell* c = cell(cx, cy);
            if (c) {
                if (fg.isSet()) c->fg = fg;
                if (bg.isSet()) c->bg = bg;
            }
        }
    }
}

void Screen::resize() {
    auto [newW, newH] = m_terminal.size();
    if (newW == m_width && newH == m_height) return;

    int newSize = newW * newH;
    m_back.clear();
    m_back.resize(newSize);
    m_front.clear();
    m_front.resize(newSize);
    m_width = newW;
    m_height = newH;
}

void Screen::flush() {
    static char buf[65536];
    int bufPos = 0;

    auto appendStr = [&](const char* str) {
        int len = std::strlen(str);
        if (bufPos + len < static_cast<int>(sizeof(buf))) {
            std::memcpy(buf + bufPos, str, len);
            bufPos += len;
        }
    };

    auto append = [&](const char* fmt, auto... args) {
        bufPos += std::snprintf(buf + bufPos, sizeof(buf) - bufPos, fmt, args...);
    };

    int lastX = -2, lastY = -2;
    Color lastFg(0xFFFFFFFF);  // invalid initial value
    Color lastBg(0xFFFFFFFF);
    u8 lastAttrs = 0xFF;
    bool firstCell = true;

    for (int y = 0; y < m_height; y++) {
        for (int x = 0; x < m_width; x++) {
            Cell& back = m_back[y * m_width + x];
            Cell& front = m_front[y * m_width + x];

            if (back == front) {
                lastX = -2;
                continue;
            }

            if (bufPos > static_cast<int>(sizeof(buf)) - 256) {
                write(STDOUT_FILENO, buf, bufPos);
                bufPos = 0;
            }

            if (x != lastX + 1 || y != lastY) {
                append("\x1b[%d;%dH", y + 1, x + 1);
            }

            if (firstCell || back.attrs != lastAttrs) {
                bufPos += std::snprintf(buf + bufPos, sizeof(buf) - bufPos, "\x1b[0");
                if (back.attrs & ATTR_BOLD)      appendStr(";1");
                if (back.attrs & ATTR_DIM)       appendStr(";2");
                if (back.attrs & ATTR_ITALIC)    appendStr(";3");
                if (back.attrs & ATTR_UNDERLINE) appendStr(";4");
                if (back.attrs & ATTR_BLINK)     appendStr(";5");
                if (back.attrs & ATTR_REVERSE)   appendStr(";7");
                if (back.attrs & ATTR_CROSSED)   appendStr(";9");
                appendStr("m");
                lastAttrs = back.attrs;
                lastFg = Color(0xFFFFFFFF);
                lastBg = Color(0xFFFFFFFF);
            }

            if (back.fg.isSet()) {
                if (firstCell || back.fg != lastFg) {
                    append("\x1b[38;2;%d;%d;%dm", back.fg.r(), back.fg.g(), back.fg.b());
                    lastFg = back.fg;
                }
            } else if (firstCell || lastFg.isSet()) {
                appendStr("\x1b[39m");
                lastFg = Color::None();
            }

            if (back.bg.isSet()) {
                if (firstCell || back.bg != lastBg) {
                    append("\x1b[48;2;%d;%d;%dm", back.bg.r(), back.bg.g(), back.bg.b());
                    lastBg = back.bg;
                }
            } else if (firstCell || lastBg.isSet()) {
                appendStr("\x1b[49m");
                lastBg = Color::None();
            }

            int chLen = std::strlen(back.ch.data());
            if (bufPos + chLen < static_cast<int>(sizeof(buf))) {
                std::memcpy(buf + bufPos, back.ch.data(), chLen);
                bufPos += chLen;
            }

            front = back;

            lastX = x;
            lastY = y;
            firstCell = false;
        }
    }

    if (bufPos > 0) {
        write(STDOUT_FILENO, buf, bufPos);
    }
}

void Screen::flushFull() {
    for (auto& c : m_front) {
        c.ch[0] = '\0';
    }
    std::printf("%s", esc::CLEAR_SCREEN);
    flush();
}

} // namespace tui
