#ifndef UI_MENU_H
#define UI_MENU_H

#include "widget.h"

typedef struct Entry {
    bool selectable;
    char *name;
    void *ctx;
    void (*on_selected)(void *ctx);
} Entry;

typedef struct Menu {
    int selected;
    int at;
    int x, y;  // screen position (0 = centered)
    Vec(Entry) entries;
} Menu;

// Add entry to menu
void menu_add_entry(Menu *m, Entry e);

// Free menu resources
void menu_free(Menu *m);

// Draw menu on screen
void menu_draw(Menu *m, Screen *s);

// Navigate up
void menu_up(Menu *m);

// Navigate down
void menu_down(Menu *m);

// Select current entry (triggers callback)
void menu_select(Menu *m);

#endif // UI_MENU_H
