package si.ui;

import si.tui.Screen;

public final class Grid implements Widget {
    private final int cols;
    private final int rows;
    private final GridCell[] cells;

    public Grid(int cols, int rows) {
        this.cols = cols;
        this.rows = rows;
        this.cells = new GridCell[cols * rows];
        for (int i = 0; i < cells.length; i++) {
            cells[i] = new GridCell();
        }
    }

    public GridCell at(int x, int y) {
        if (x < 0 || y < 0 || x >= cols || y >= rows) {
            return null;
        }
        return cells[x + y * cols];
    }

    public void clearCells() {
        for (GridCell cell : cells) {
            cell.clear();
        }
    }

    @Override
    public void draw(Screen screen, BBox bbox) {
        int xPad = (bbox.w() / cols) / 2;
        int yPad = (bbox.h() / rows) / 2;

        for (int row = 0; row < rows; row++) {
            for (int col = 0; col < cols; col++) {
                int xx = col * bbox.w() / cols + bbox.x() + xPad;
                int yy = row * bbox.h() / rows + bbox.y() + yPad;

                GridCell cell = cells[col + row * cols];
                if (cell.hasCallback()) {
                    cell.draw(screen, xx, yy);
                }
            }
        }
    }

    public int cols() {
        return cols;
    }

    public int rows() {
        return rows;
    }
}
