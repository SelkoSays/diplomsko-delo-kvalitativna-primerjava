#include "screen.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

Screen *screen_new(void) {
    Screen *s = malloc(sizeof(Screen));
    if (!s) return NULL;

    terminal_get_size(&s->width, &s->height);

    int size = s->width * s->height;
    s->back = malloc(size * sizeof(Cell));
    s->front = malloc(size * sizeof(Cell));

    if (!s->back || !s->front) {
        free(s->back);
        free(s->front);
        free(s);
        return NULL;
    }

    for (int i = 0; i < size; i++) {
        cell_init(&s->back[i]);
        cell_init(&s->front[i]);
    }

    terminal_setup(&s->terminal);

    return s;
}

void screen_free(Screen *s) {
    if (!s) return;

    terminal_restore(&s->terminal);

    free(s->back);
    free(s->front);
    free(s);
}

void screen_resize(Screen *s) {
    int new_w, new_h;
    terminal_get_size(&new_w, &new_h);

    if (new_w == s->width && new_h == s->height) return;

    int new_size = new_w * new_h;
    Cell *new_back = malloc(new_size * sizeof(Cell));
    Cell *new_front = malloc(new_size * sizeof(Cell));

    if (!new_back || !new_front) {
        free(new_back);
        free(new_front);
        return;  // keep old buffers on failure
    }

    for (int i = 0; i < new_size; i++) {
        cell_init(&new_back[i]);
        cell_init(&new_front[i]);
    }

    free(s->back);
    free(s->front);
    s->back = new_back;
    s->front = new_front;
    s->width = new_w;
    s->height = new_h;
}

void screen_clear(Screen *s) {
    int size = s->width * s->height;
    for (int i = 0; i < size; i++) {
        cell_init(&s->back[i]);
    }
}

Cell *screen_cell(Screen *s, int x, int y) {
    if (x < 0 || x >= s->width || y < 0 || y >= s->height) return NULL;
    return &s->back[y * s->width + x];
}

void screen_putc(Screen *s, int x, int y, const char *ch) {
    Cell *c = screen_cell(s, x, y);
    if (!c) return;

    int len = 0;
    if (ch && ch[0]) {
        unsigned char first = (unsigned char)ch[0];
        if ((first & 0x80) == 0) len = 1;           // ASCII
        else if ((first & 0xE0) == 0xC0) len = 2;   // 2-byte UTF-8
        else if ((first & 0xF0) == 0xE0) len = 3;   // 3-byte UTF-8
        else if ((first & 0xF8) == 0xF0) len = 4;   // 4-byte UTF-8
        else len = 1;  // fallback
    }

    if (len > 0) {
        memcpy(c->ch, ch, len);
        c->ch[len] = '\0';
    } else {
        c->ch[0] = ' ';
        c->ch[1] = '\0';
    }
}

void screen_puts(Screen *s, int x, int y, const char *str) {
    if (!str) return;

    int cx = x;
    int cy = y;
    const char *p = str;

    while (*p && cy < s->height) {
        unsigned char first = (unsigned char)*p;

        if (first < 0x80) {
            if (first == '\n') {
                cy++;
                cx = x;
                p++;
                continue;
            }
            if (first < 0x20 || first == 0x7F) {
                p++;
                continue;
            }
        }

        if (cx >= s->width) {
            p++;
            continue;
        }

        int len = 1;
        if ((first & 0x80) == 0) len = 1;
        else if ((first & 0xE0) == 0xC0) len = 2;
        else if ((first & 0xF0) == 0xE0) len = 3;
        else if ((first & 0xF8) == 0xF0) len = 4;

        screen_putc(s, cx, cy, p);
        p += len;
        cx++;
    }
}

void screen_set_fg_color(Screen *s, int x, int y, Color color) {
    Cell *c = screen_cell(s, x, y);
    if (c) c->fg = color;
}

void screen_set_bg_color(Screen *s, int x, int y, Color color) {
    Cell *c = screen_cell(s, x, y);
    if (c) c->bg = color;
}

void screen_set_fg(Screen *s, int x, int y, uint8_t r, uint8_t g, uint8_t b) {
    screen_set_fg_color(s, x, y, COLOR_RGB(r, g, b));
}

void screen_set_bg(Screen *s, int x, int y, uint8_t r, uint8_t g, uint8_t b) {
    screen_set_bg_color(s, x, y, COLOR_RGB(r, g, b));
}

void screen_set_attr(Screen *s, int x, int y, uint8_t attr) {
    Cell *c = screen_cell(s, x, y);
    if (c) c->attrs = attr;
}

void screen_put_color(Screen *s, int x, int y, const char *ch,
                      Color fg, Color bg, uint8_t attrs) {
    Cell *c = screen_cell(s, x, y);
    if (!c) return;

    screen_putc(s, x, y, ch);
    c->fg = fg;
    c->bg = bg;
    c->attrs = attrs;
}

void screen_fill(Screen *s, int x, int y, int w, int h, const char *ch) {
    for (int cy = y; cy < y + h && cy < s->height; cy++) {
        for (int cx = x; cx < x + w && cx < s->width; cx++) {
            screen_putc(s, cx, cy, ch);
        }
    }
}

void screen_fill_color(Screen *s, int x, int y, int w, int h,
                       Color fg, Color bg) {
    for (int cy = y; cy < y + h && cy < s->height; cy++) {
        for (int cx = x; cx < x + w && cx < s->width; cx++) {
            Cell *c = screen_cell(s, cx, cy);
            if (c) {
                if (COLOR_IS_SET(fg)) c->fg = fg;
                if (COLOR_IS_SET(bg)) c->bg = bg;
            }
        }
    }
}

void screen_flush(Screen *s) {
    static char buf[65536];
    int buf_pos = 0;

    #define BUF_APPEND(...) buf_pos += snprintf(buf + buf_pos, sizeof(buf) - buf_pos, __VA_ARGS__)
    #define BUF_APPEND_STR(str) do { \
        int len = strlen(str); \
        if (buf_pos + len < (int)sizeof(buf)) { \
            memcpy(buf + buf_pos, str, len); \
            buf_pos += len; \
        } \
    } while(0)

    int last_x = -2, last_y = -2;
    Color last_fg = 0xFFFFFFFF;  // invalid initial value
    Color last_bg = 0xFFFFFFFF;
    uint8_t last_attrs = 0xFF;
    bool first_cell = true;

    for (int y = 0; y < s->height; y++) {
        for (int x = 0; x < s->width; x++) {
            Cell *back = &s->back[y * s->width + x];
            Cell *front = &s->front[y * s->width + x];

            if (cell_eq(back, front)) {
                last_x = -2;
                continue;
            }

            if (buf_pos > (int)sizeof(buf) - 256) {
                write(STDOUT_FILENO, buf, buf_pos);
                buf_pos = 0;
            }

            if (x != last_x + 1 || y != last_y) {
                BUF_APPEND("\x1b[%d;%dH", y + 1, x + 1);
            }

            if (first_cell || back->attrs != last_attrs) {
                BUF_APPEND("\x1b[0");
                if (back->attrs & ATTR_BOLD)      BUF_APPEND_STR(";1");
                if (back->attrs & ATTR_DIM)       BUF_APPEND_STR(";2");
                if (back->attrs & ATTR_ITALIC)    BUF_APPEND_STR(";3");
                if (back->attrs & ATTR_UNDERLINE) BUF_APPEND_STR(";4");
                if (back->attrs & ATTR_BLINK)     BUF_APPEND_STR(";5");
                if (back->attrs & ATTR_REVERSE)   BUF_APPEND_STR(";7");
                if (back->attrs & ATTR_CROSSED)   BUF_APPEND_STR(";9");
                BUF_APPEND_STR("m");
                last_attrs = back->attrs;
                last_fg = 0xFFFFFFFF;
                last_bg = 0xFFFFFFFF;
            }

            if (COLOR_IS_SET(back->fg)) {
                if (first_cell || back->fg != last_fg) {
                    BUF_APPEND("\x1b[38;2;%d;%d;%dm",
                               COLOR_R(back->fg), COLOR_G(back->fg), COLOR_B(back->fg));
                    last_fg = back->fg;
                }
            } else if (first_cell || COLOR_IS_SET(last_fg)) {
                BUF_APPEND_STR("\x1b[39m");
                last_fg = COLOR_NONE;
            }

            if (COLOR_IS_SET(back->bg)) {
                if (first_cell || back->bg != last_bg) {
                    BUF_APPEND("\x1b[48;2;%d;%d;%dm",
                               COLOR_R(back->bg), COLOR_G(back->bg), COLOR_B(back->bg));
                    last_bg = back->bg;
                }
            } else if (first_cell || COLOR_IS_SET(last_bg)) {
                BUF_APPEND_STR("\x1b[49m");
                last_bg = COLOR_NONE;
            }

            int ch_len = strlen(back->ch);
            if (buf_pos + ch_len < (int)sizeof(buf)) {
                memcpy(buf + buf_pos, back->ch, ch_len);
                buf_pos += ch_len;
            }

            cell_copy(front, back);

            last_x = x;
            last_y = y;
            first_cell = false;
        }
    }

    if (buf_pos > 0) {
        write(STDOUT_FILENO, buf, buf_pos);
    }

    #undef BUF_APPEND
    #undef BUF_APPEND_STR
}

void screen_flush_full(Screen *s) {
    int size = s->width * s->height;
    for (int i = 0; i < size; i++) {
        s->front[i].ch[0] = '\0';
    }

    printf(ESC_CLEAR_SCREEN);
    screen_flush(s);
}
