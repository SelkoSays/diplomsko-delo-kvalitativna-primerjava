#ifndef UI_GRID_H
#define UI_GRID_H

#include "widget.h"

typedef void (*GridCellFn)(void *ctx, Screen *s, int x, int y);

typedef struct GridCell {
    void *ctx;
    GridCellFn draw;
} GridCell;

typedef struct Grid {
    u32 cols, rows;
    GridCell *cells;
} Grid;

// Create new grid
Grid *grid_new(u32 cols, u32 rows);

// Free grid
void grid_free(Grid *g);

// Get cell at position (NULL if out of bounds)
GridCell *grid_at(Grid *g, int x, int y);

// Draw grid within bounding box
void grid_draw(Grid *g, Screen *s, BBox b);

// Get grid as widget
Widget grid_as_widget(Grid *g);

#endif // UI_GRID_H
