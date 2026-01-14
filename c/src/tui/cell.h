#ifndef TUI_CELL_H
#define TUI_CELL_H

#include "color.h"
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#define ATTR_NONE       0
#define ATTR_BOLD       (1 << 0)
#define ATTR_DIM        (1 << 1)
#define ATTR_ITALIC     (1 << 2)
#define ATTR_UNDERLINE  (1 << 3)
#define ATTR_BLINK      (1 << 4)
#define ATTR_REVERSE    (1 << 5)
#define ATTR_CROSSED    (1 << 6)

typedef struct Cell {
    char ch[5];      // UTF-8 character (4 bytes + null terminator)
    Color fg;        // foreground: 0xAARRGGBB (A=0 means default)
    Color bg;        // background: 0xAARRGGBB (A=0 means default)
    uint8_t attrs;   // text attributes (bold, dim, etc.)
} Cell;

static inline void cell_init(Cell *c) {
    c->ch[0] = ' ';
    c->ch[1] = '\0';
    c->fg = COLOR_NONE;
    c->bg = COLOR_NONE;
    c->attrs = ATTR_NONE;
}

static inline bool cell_eq(const Cell *a, const Cell *b) {
    return a->fg == b->fg && a->bg == b->bg && a->attrs == b->attrs &&
           memcmp(a->ch, b->ch, 5) == 0;
}

static inline void cell_copy(Cell *dst, const Cell *src) {
    *dst = *src;
}

#endif // TUI_CELL_H
