#include "frame.hpp"
#include <stdexcept>

namespace ui {

namespace chars {
    constexpr const char* UP_LEFT    = "╭";
    constexpr const char* UP_RIGHT   = "╮";
    constexpr const char* DOWN_LEFT  = "╰";
    constexpr const char* DOWN_RIGHT = "╯";
    constexpr const char* HDASH      = "─";
    constexpr const char* VDASH      = "│";
    constexpr const char* FORK_DOWN  = "┬";
    constexpr const char* FORK_LEFT  = "┤";
    constexpr const char* FORK_UP    = "┴";
    constexpr const char* FORK_RIGHT = "├";
}

constexpr auto BORDER_COLOR = tui::Color::Gray();

Frame::Frame(u32 w, u32 h, u32 x, u32 y)
    : m_w(w), m_h(h), m_x(x), m_y(y) {}

std::pair<Frame&, Frame&> Frame::split(u32 coord, FrameSplit splitType) {
    if (m_split[0] || m_split[1]) {
        throw std::runtime_error("Frame is already split");
    }

    constexpr u32 pad = 2;

    if (splitType == FrameSplit::Vertical) {
        if (coord >= (m_w - pad) || coord < pad) {
            throw std::runtime_error("Invalid vertical split");
        }
        m_split[0] = std::make_unique<Frame>(coord, m_h, m_x, m_y);
        m_split[1] = std::make_unique<Frame>(m_w - coord + 1, m_h, m_x + coord - 1, m_y);
    } else {
        if (coord >= (m_h - pad) || coord < pad) {
            throw std::runtime_error("Invalid horizontal split");
        }
        m_split[0] = std::make_unique<Frame>(m_w, coord, m_x, m_y);
        m_split[1] = std::make_unique<Frame>(m_w, m_h - coord + 1, m_x, m_y + coord - 1);
    }

    return {*m_split[0], *m_split[1]};
}

void Frame::addWidget(std::unique_ptr<Widget> widget) {
    m_widgets.push_back(std::move(widget));
}

void Frame::draw(tui::Screen& screen) {
    drawStage(screen, 1);  // borders
    drawStage(screen, 2);  // joints
    drawStage(screen, 3);  // widgets
}

void Frame::drawStage(tui::Screen& screen, int stage) {
    if (m_split[0]) m_split[0]->drawStage(screen, stage);
    if (m_split[1]) m_split[1]->drawStage(screen, stage);

    if (stage == 1) {
        drawBorders(screen);
    } else if (stage == 2) {
        drawJoints(screen);
    } else if (stage == 3) {
        drawWidgets(screen);
    }
}

void Frame::drawBorders(tui::Screen& screen) {
    screen.putChar(m_x, m_y, chars::UP_LEFT);
    screen.setFgColor(m_x, m_y, BORDER_COLOR);

    for (u32 i = 1; i < (m_w - 1); i++) {
        screen.putChar(m_x + i, m_y, chars::HDASH);
        screen.setFgColor(m_x + i, m_y, BORDER_COLOR);
    }

    screen.putChar(m_x + m_w - 1, m_y, chars::UP_RIGHT);
    screen.setFgColor(m_x + m_w - 1, m_y, BORDER_COLOR);

    for (u32 i = 1; i < (m_h - 1); i++) {
        screen.putChar(m_x, m_y + i, chars::VDASH);
        screen.setFgColor(m_x, m_y + i, BORDER_COLOR);
        screen.putChar(m_x + m_w - 1, m_y + i, chars::VDASH);
        screen.setFgColor(m_x + m_w - 1, m_y + i, BORDER_COLOR);
    }

    screen.putChar(m_x, m_y + m_h - 1, chars::DOWN_LEFT);
    screen.setFgColor(m_x, m_y + m_h - 1, BORDER_COLOR);

    for (u32 i = 1; i < (m_w - 1); i++) {
        screen.putChar(m_x + i, m_y + m_h - 1, chars::HDASH);
        screen.setFgColor(m_x + i, m_y + m_h - 1, BORDER_COLOR);
    }

    screen.putChar(m_x + m_w - 1, m_y + m_h - 1, chars::DOWN_RIGHT);
    screen.setFgColor(m_x + m_w - 1, m_y + m_h - 1, BORDER_COLOR);
}

void Frame::drawJoints(tui::Screen& screen) {
    if (m_split[1]) {
        if (m_y == m_split[1]->y()) {
            // Vertical split
            screen.putChar(m_split[1]->x(), m_y, chars::FORK_DOWN);
            screen.setFgColor(m_split[1]->x(), m_y, BORDER_COLOR);
            screen.putChar(m_split[1]->x(), m_y + m_h - 1, chars::FORK_UP);
            screen.setFgColor(m_split[1]->x(), m_y + m_h - 1, BORDER_COLOR);
        } else {
            // Horizontal split
            screen.putChar(m_x, m_split[1]->y(), chars::FORK_RIGHT);
            screen.setFgColor(m_x, m_split[1]->y(), BORDER_COLOR);
            screen.putChar(m_x + m_w - 1, m_split[1]->y(), chars::FORK_LEFT);
            screen.setFgColor(m_x + m_w - 1, m_split[1]->y(), BORDER_COLOR);
        }
    }
}

void Frame::drawWidgets(tui::Screen& screen) {
    BBox bbox{m_x + 1, m_y + 1, m_w - 2, m_h - 2};
    for (auto& widget : m_widgets) {
        if (widget) {
            widget->draw(screen, bbox);
        }
    }
}

} // namespace ui
