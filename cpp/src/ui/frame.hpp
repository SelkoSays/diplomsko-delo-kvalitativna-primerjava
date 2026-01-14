#pragma once

#include "widget.hpp"

namespace ui {

enum class FrameSplit {
    Vertical,
    Horizontal
};

class Frame {
public:
    Frame(u32 w, u32 h, u32 x, u32 y);

    // Split operations return references to child frames
    std::pair<Frame&, Frame&> split(u32 coord, FrameSplit splitType);

    void addWidget(std::unique_ptr<Widget> widget);

    void draw(tui::Screen& screen);

    u32 width() const { return m_w; }
    u32 height() const { return m_h; }
    u32 x() const { return m_x; }
    u32 y() const { return m_y; }

private:
    void drawBorders(tui::Screen& screen);
    void drawJoints(tui::Screen& screen);
    void drawWidgets(tui::Screen& screen);
    void drawStage(tui::Screen& screen, int stage);

    u32 m_w, m_h, m_x, m_y;
    std::unique_ptr<Frame> m_split[2];
    std::vector<std::unique_ptr<Widget>> m_widgets;
};

} // namespace ui
