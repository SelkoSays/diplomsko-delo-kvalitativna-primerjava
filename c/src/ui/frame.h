#ifndef UI_FRAME_H
#define UI_FRAME_H

#include "widget.h"

typedef enum FrameSplit {
    FRAME_SPLIT_VERTICAL,
    FRAME_SPLIT_HORIZONTAL
} FrameSplit;

typedef struct Frame {
    u32 w, h;
    u32 x, y;              // real coords
    struct Frame *split[2];
    Vec(Widget) widgets;
} Frame;

// Create new frame
Frame *frame_new(u32 w, u32 h, u32 x, u32 y);

// Free frame and all children
void frame_free(Frame *f);

// Split frame at coordinate, returns pointer to split[2] array
Frame *(*frame_split(Frame *f, u32 coord, FrameSplit split))[2];

// Add widget to frame
void frame_add_widget(Frame *f, Widget widget);

// Draw frame (stage 1=borders, 2=joints, 3=items)
void frame_draw(Frame *f, Screen *s, int stage);

// Draw all stages
void frame_draw_all(Frame *f, Screen *s);

#endif // UI_FRAME_H
