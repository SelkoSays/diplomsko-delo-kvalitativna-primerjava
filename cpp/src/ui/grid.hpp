#pragma once

#include "widget.hpp"

namespace ui {

// Grid cell with optional draw callback
class GridCell {
public:
    GridCell() = default;

    void setDrawCallback(std::function<void(tui::Screen&, int, int)> fn) {
        m_drawFn = std::move(fn);
    }

    void clear() { m_drawFn = nullptr; }

    void draw(tui::Screen& screen, int x, int y) {
        if (m_drawFn) {
            m_drawFn(screen, x, y);
        }
    }

    bool hasCallback() const { return m_drawFn != nullptr; }

private:
    std::function<void(tui::Screen&, int, int)> m_drawFn;
};

// 2D Grid for game area
class Grid : public Widget {
public:
    Grid(u32 cols, u32 rows);

    void draw(tui::Screen& screen, BBox bbox) override;

    GridCell* at(int x, int y);
    void clearCells();

    u32 cols() const { return m_cols; }
    u32 rows() const { return m_rows; }

private:
    u32 m_cols, m_rows;
    std::vector<GridCell> m_cells;
};

} // namespace ui
