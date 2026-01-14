#pragma once

#include "terminal.hpp"
#include "cell.hpp"

namespace tui {

class Screen {
public:
    Screen();

    int width() const { return m_width; }
    int height() const { return m_height; }

    void clear();
    void flush();
    void flushFull();
    void resize();

    // Drawing primitives
    void putChar(int x, int y, std::string_view ch);
    void putString(int x, int y, std::string_view str);
    void setFgColor(int x, int y, Color color);
    void setBgColor(int x, int y, Color color);
    void setAttr(int x, int y, u8 attrs);
    void fill(int x, int y, int w, int h, std::string_view ch);
    void fillColor(int x, int y, int w, int h, Color fg, Color bg);

private:
    Cell* cell(int x, int y);
    const Cell* cell(int x, int y) const;

    int m_width;
    int m_height;
    std::vector<Cell> m_back;   // Write buffer
    std::vector<Cell> m_front;  // Current screen state
    Terminal m_terminal;         // RAII terminal management
};

} // namespace tui
