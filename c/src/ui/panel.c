#include "panel.h"
#include <stdio.h>

void panel_add_item(Panel *p, PanelItem item) {
    if (p == NULL) return;
    list_append(&p->items, item);
}

void panel_empty_line(Panel *p) {
    if (p == NULL) return;
    PanelItem item = {.ctx = NULL, .draw = NULL};
    list_append(&p->items, item);
}

void panel_draw(Panel *p, Screen *s, BBox b) {
    if (p == NULL || s == NULL) return;

    for (int i = 0; i < p->items.count && (u32)i < b.h; i++) {
        if (p->items.items[i].draw) {
            p->items.items[i].draw(p->items.items[i].ctx, s, b.x, b.y + i);
        }
    }
}

void panel_free(Panel *p) {
    list_free(p->items);
}

static void panel_draw_frame(void *ctx, Screen *s, BBox bbox) {
    Panel *p = (Panel *)ctx;
    panel_draw(p, s, bbox);
}

Widget panel_as_widget(Panel *p) {
    Widget widget = {
        .ctx = (void *)p,
        .draw = panel_draw_frame,
        .free_item = (WidgetFreeFn)panel_free,
    };
    return widget;
}

void panel_item_text_draw(void *ctx, Screen *s, int x, int y) {
    char *text = (char *)ctx;
    screen_puts(s, x, y, text);
}

void value_draw(void *ctx, Screen *s, int x, int y) {
    Value *v = (Value *)ctx;
    static char label_buf[256];
    static char value_buf[256];

    // Get stringified value
    if (v->stringify) {
        v->stringify(v->ctx, value_buf, sizeof(value_buf));
    } else {
        value_buf[0] = '\0';
    }

    snprintf(label_buf, sizeof(label_buf), "%s: %s", v->label, value_buf);
    screen_puts(s, x, y, label_buf);
}

PanelItem value_as_panel_item(Value *v) {
    PanelItem item = {
        .ctx = (void *)v,
        .draw = value_draw,
    };
    return item;
}

void value_stringify_int(void *ctx, char *buf, usize buf_size) {
    int *val = (int *)ctx;
    snprintf(buf, buf_size, "%d", *val);
}

void value_stringify_percent(void *ctx, char *buf, usize buf_size) {
    int *val = (int *)ctx;
    snprintf(buf, buf_size, "%d%%", *val);
}

void value_stringify_time(void *ctx, char *buf, usize buf_size) {
    int *val = (int *)ctx;
    int secs = *val;
    int mins = secs / 60;
    secs = secs % 60;
    if (mins > 0) {
        snprintf(buf, buf_size, "%d:%02d", mins, secs);
    } else {
        snprintf(buf, buf_size, "%ds", secs);
    }
}
