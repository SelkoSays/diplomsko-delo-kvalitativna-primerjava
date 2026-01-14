#ifndef UI_PANEL_H
#define UI_PANEL_H

#include "widget.h"

typedef void (*PanelItemFn)(void *ctx, Screen *s, int x, int y);

typedef struct PanelItem {
    void *ctx;
    PanelItemFn draw;
} PanelItem;

typedef struct Panel {
    Vec(PanelItem) items;
} Panel;

// Add item to panel
void panel_add_item(Panel *p, PanelItem item);

// Add empty line to panel
void panel_empty_line(Panel *p);

// Draw panel within bounding box
void panel_draw(Panel *p, Screen *s, BBox b);

// Free panel resources
void panel_free(Panel *p);

// Get panel as widget
Widget panel_as_widget(Panel *p);

void panel_item_text_draw(void *ctx, Screen *s, int x, int y);

// Function to stringify a value into buffer
typedef void (*ValueStringifyFn)(void *ctx, char *buf, usize buf_size);

typedef struct Value {
    char *label;
    void *ctx;
    ValueStringifyFn stringify;
} Value;

// Draw value item
void value_draw(void *ctx, Screen *s, int x, int y);

// Get value as panel item
PanelItem value_as_panel_item(Value *v);

// Stringify an int pointer
void value_stringify_int(void *ctx, char *buf, usize buf_size);

// Stringify an int pointer with % suffix
void value_stringify_percent(void *ctx, char *buf, usize buf_size);

// Stringify an int pointer as time (seconds)
void value_stringify_time(void *ctx, char *buf, usize buf_size);

#endif // UI_PANEL_H
