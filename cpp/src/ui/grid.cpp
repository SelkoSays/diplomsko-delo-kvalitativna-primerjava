#include "grid.hpp"

namespace ui {

Grid::Grid(u32 cols, u32 rows)
    : m_cols(cols), m_rows(rows), m_cells(cols * rows) {}

GridCell* Grid::at(int x, int y) {
    if (x < 0 || y < 0) return nullptr;
    if (static_cast<u32>(x) >= m_cols || static_cast<u32>(y) >= m_rows) return nullptr;
    return &m_cells[x + y * m_cols];
}

void Grid::clearCells() {
    for (auto& cell : m_cells) {
        cell.clear();
    }
}

void Grid::draw(tui::Screen& screen, BBox bbox) {
    u32 xPad = (bbox.w / m_cols) / 2;
    u32 yPad = (bbox.h / m_rows) / 2;

    for (u32 row = 0; row < m_rows; row++) {
        for (u32 col = 0; col < m_cols; col++) {
            int xx = col * bbox.w / m_cols + bbox.x + xPad;
            int yy = row * bbox.h / m_rows + bbox.y + yPad;

            GridCell& cell = m_cells[col + row * m_cols];
            if (cell.hasCallback()) {
                cell.draw(screen, xx, yy);
            }
        }
    }
}

} // namespace ui
