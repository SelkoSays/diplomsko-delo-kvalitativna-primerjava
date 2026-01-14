#include "frame.h"
#include <stdlib.h>

// Box-drawing characters
static const char *frame_chars[] = {
    "╭", "╮", "╰", "╯", "─", "│", "┬", "┤", "┴", "├"
};
#define UP_LEFT 0
#define UP_RIGHT 1
#define DOWN_LEFT 2
#define DOWN_RIGHT 3
#define HDASH 4
#define VDASH 5
#define FORK_DOWN 6
#define FORK_LEFT 7
#define FORK_UP 8
#define FORK_RIGHT 9

#define FRAME_BORDER_COLOR COLOR_GRAY

Frame *frame_new(u32 w, u32 h, u32 x, u32 y) {
    Frame *f = box(Frame);
    f->w = w;
    f->h = h;
    f->x = x;
    f->y = y;
    f->split[0] = NULL;
    f->split[1] = NULL;
    return f;
}

void frame_free(Frame *f) {
    if (f == NULL) return;

    frame_free(f->split[0]);
    frame_free(f->split[1]);

    for (int i = 0; i < f->widgets.count; i++) {
        if (f->widgets.items[i].free_item) {
            f->widgets.items[i].free_item(f->widgets.items[i].ctx);
        }
    }
    list_free(f->widgets);
    free(f);
}

Frame *(*frame_split(Frame *f, u32 coord, FrameSplit split))[2] {
    if (f == NULL) return NULL;

    if (f->split[0] != NULL || f->split[1] != NULL) {
        // Frame is already split - would panic in original
        return NULL;
    }

    u32 pad = 2;

    switch (split) {
    case FRAME_SPLIT_VERTICAL:
        if (coord >= (f->w - pad) || coord < pad) {
            return NULL;  // Invalid split
        }
        f->split[0] = frame_new(coord, f->h, f->x, f->y);
        f->split[1] = frame_new(f->w - coord + 1, f->h, f->x + coord - 1, f->y);
        break;

    case FRAME_SPLIT_HORIZONTAL:
        if (coord >= (f->h - pad) || coord < pad) {
            return NULL;  // Invalid split
        }
        f->split[0] = frame_new(f->w, coord, f->x, f->y);
        f->split[1] = frame_new(f->w, f->h - coord + 1, f->x, f->y + coord - 1);
        break;

    default:
        return NULL;
    }

    return &f->split;
}

void frame_add_widget(Frame *f, Widget widget) {
    if (f == NULL) return;
    list_append(&f->widgets, widget);
}

void frame_draw(Frame *f, Screen *s, int stage) {
    if (f == NULL || s == NULL) return;

    // Recursively draw child frames
    frame_draw(f->split[0], s, stage);
    frame_draw(f->split[1], s, stage);

    // Stage 1: Draw borders
    if (stage == 1) {
        // Top-left corner
        screen_putc(s, f->x, f->y, frame_chars[UP_LEFT]);
        screen_set_fg_color(s, f->x, f->y, FRAME_BORDER_COLOR);

        // Top edge
        for (u32 i = 1; i < (f->w - 1); i++) {
            screen_putc(s, f->x + i, f->y, frame_chars[HDASH]);
            screen_set_fg_color(s, f->x + i, f->y, FRAME_BORDER_COLOR);
        }

        // Top-right corner
        screen_putc(s, f->x + f->w - 1, f->y, frame_chars[UP_RIGHT]);
        screen_set_fg_color(s, f->x + f->w - 1, f->y, FRAME_BORDER_COLOR);

        // Side edges
        for (u32 i = 1; i < (f->h - 1); i++) {
            screen_putc(s, f->x, f->y + i, frame_chars[VDASH]);
            screen_set_fg_color(s, f->x, f->y + i, FRAME_BORDER_COLOR);
            screen_putc(s, f->x + f->w - 1, f->y + i, frame_chars[VDASH]);
            screen_set_fg_color(s, f->x + f->w - 1, f->y + i, FRAME_BORDER_COLOR);
        }

        // Bottom-left corner
        screen_putc(s, f->x, f->y + f->h - 1, frame_chars[DOWN_LEFT]);
        screen_set_fg_color(s, f->x, f->y + f->h - 1, FRAME_BORDER_COLOR);

        // Bottom edge
        for (u32 i = 1; i < (f->w - 1); i++) {
            screen_putc(s, f->x + i, f->y + f->h - 1, frame_chars[HDASH]);
            screen_set_fg_color(s, f->x + i, f->y + f->h - 1, FRAME_BORDER_COLOR);
        }

        // Bottom-right corner
        screen_putc(s, f->x + f->w - 1, f->y + f->h - 1, frame_chars[DOWN_RIGHT]);
        screen_set_fg_color(s, f->x + f->w - 1, f->y + f->h - 1, FRAME_BORDER_COLOR);
    }
    // Stage 2: Draw split joints
    else if (stage == 2) {
        if (f->split[1]) {
            if (f->y == f->split[1]->y) {
                // Vertical split
                screen_putc(s, f->split[1]->x, f->y, frame_chars[FORK_DOWN]);
                screen_set_fg_color(s, f->split[1]->x, f->y, FRAME_BORDER_COLOR);
                screen_putc(s, f->split[1]->x, f->y + f->h - 1, frame_chars[FORK_UP]);
                screen_set_fg_color(s, f->split[1]->x, f->y + f->h - 1, FRAME_BORDER_COLOR);
            } else {
                // Horizontal split
                screen_putc(s, f->x, f->split[1]->y, frame_chars[FORK_RIGHT]);
                screen_set_fg_color(s, f->x, f->split[1]->y, FRAME_BORDER_COLOR);
                screen_putc(s, f->x + f->w - 1, f->split[1]->y, frame_chars[FORK_LEFT]);
                screen_set_fg_color(s, f->x + f->w - 1, f->split[1]->y, FRAME_BORDER_COLOR);
            }
        }
    }
    // Stage 3: Draw items
    else if (stage == 3) {
        for (int i = 0; i < f->widgets.count; i++) {
            if (f->widgets.items[i].draw) {
                BBox bbox = {
                    .x = f->x + 1,
                    .y = f->y + 1,
                    .w = f->w - 2,
                    .h = f->h - 2,
                };
                f->widgets.items[i].draw(f->widgets.items[i].ctx, s, bbox);
            }
        }
    }
}

void frame_draw_all(Frame *f, Screen *s) {
    frame_draw(f, s, 1);  // borders
    frame_draw(f, s, 2);  // joints
    frame_draw(f, s, 3);  // items
}
