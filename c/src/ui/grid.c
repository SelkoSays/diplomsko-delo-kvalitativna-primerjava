#include "grid.h"
#include <stdlib.h>

Grid *grid_new(u32 cols, u32 rows) {
    Grid *g = box(Grid);
    g->cols = cols;
    g->rows = rows;
    g->cells = (GridCell *)calloc(cols * rows, sizeof(GridCell));
    return g;
}

void grid_free(Grid *g) {
    if (g == NULL) return;
    free(g->cells);
    free(g);
}

GridCell *grid_at(Grid *g, int x, int y) {
    if (x < 0 || y < 0) return NULL;
    if ((u32)x >= g->cols || (u32)y >= g->rows) return NULL;
    return &g->cells[x + y * (int)g->cols];
}

void grid_draw(Grid *g, Screen *s, BBox b) {
    if (g == NULL || s == NULL) return;

    u32 x_pad = (b.w / g->cols) / 2;
    u32 y_pad = (b.h / g->rows) / 2;

    for (u32 row = 0; row < g->rows; row++) {
        for (u32 col = 0; col < g->cols; col++) {
            int xx = col * b.w / g->cols + b.x + x_pad;
            int yy = row * b.h / g->rows + b.y + y_pad;

            GridCell *cell = &g->cells[col + row * g->cols];
            if (cell->draw) {
                cell->draw(cell->ctx, s, xx, yy);
            }
        }
    }
}

static void grid_draw_frame(void *ctx, Screen *s, BBox bbox) {
    Grid *g = (Grid *)ctx;
    grid_draw(g, s, bbox);
}

Widget grid_as_widget(Grid *g) {
    Widget widget = {
        .ctx = (void *)g,
        .draw = grid_draw_frame,
        .free_item = NULL,
    };
    return widget;
}
