#ifndef UI_WIDGET_H
#define UI_WIDGET_H

#include "../common.h"
#include "../tui/screen.h"

typedef struct BBox {
    u32 x, y, w, h;
} BBox;

// Draw callback: (ctx, screen, bbox)
typedef void (*WidgetDrawFn)(void *ctx, Screen *s, BBox b);

// Free callback: (ctx)
typedef void (*WidgetFreeFn)(void *ctx);

typedef struct Widget {
    void *ctx;
    WidgetDrawFn draw;
    WidgetFreeFn free_item;
} Widget;

#endif // UI_WIDGET_H
