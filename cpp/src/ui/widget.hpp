#pragma once

#include "../common.hpp"
#include "../tui/screen.hpp"

namespace ui {

struct BBox {
    u32 x = 0, y = 0, w = 0, h = 0;
};

// Abstract drawable widget
class Widget {
public:
    virtual ~Widget() = default;
    virtual void draw(tui::Screen& screen, BBox bbox) = 0;
};

} // namespace ui
