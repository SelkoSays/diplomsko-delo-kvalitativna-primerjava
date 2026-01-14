#include "menu.h"
#include <stdlib.h>
#include <string.h>

void menu_add_entry(Menu *m, Entry e) {
    if (m == NULL) return;

    if (e.name != NULL) {
        size_t len = strlen(e.name);
        char *str = (char *)malloc(len + 1);
        memcpy(str, e.name, len);
        str[len] = '\0';
        e.name = str;
    } else {
        e.selectable = false;
    }
    list_append(&m->entries, e);
}

void menu_free(Menu *m) {
    for (int i = 0; i < m->entries.count; i++) {
        free(m->entries.items[i].name);
    }
    list_free(m->entries);
}

void menu_draw(Menu *m, Screen *s) {
    if (m == NULL || s == NULL) return;

    int max_len = 0;
    for (int i = 0; i < m->entries.count; i++) {
        if (m->entries.items[i].name) {
            int len = strlen(m->entries.items[i].name) + 2;
            if (len > max_len) {
                max_len = len;
            }
        }
    }

    int start_x = m->x;
    int start_y = m->y;
    if (start_x == 0) {
        start_x = (s->width - max_len) / 2;
    }
    if (start_y == 0) {
        start_y = (s->height - m->entries.count) / 2;
    }

    for (int i = 0; i < m->entries.count; i++) {
        int y = start_y + i;

        if (m->entries.items[i].name == NULL) {
            continue;
        }

        screen_puts(s, start_x, y, m->entries.items[i].name);

        Color fg = m->entries.items[i].selectable ? COLOR_WHITE : COLOR_GRAY;
        int len = strlen(m->entries.items[i].name);
        for (int j = 0; j < len; j++) {
            screen_set_fg_color(s, start_x + j, y, fg);
        }

        if (i == m->at) {
            screen_puts(s, start_x + len + 1, y, "<");
            screen_set_fg_color(s, start_x + len + 1, y, COLOR_YELLOW);
        }
    }
}

void menu_up(Menu *m) {
    if (m == NULL) return;

    if (m->at < 0) {
        for (int i = 0; i < m->entries.count; i++) {
            if (m->entries.items[i].selectable) {
                m->at = i;
                break;
            }
        }
    } else if (m->at > 0) {
        for (int i = m->at - 1; i >= 0; i--) {
            if (m->entries.items[i].selectable) {
                m->at = i;
                break;
            }
        }
    }
}

void menu_down(Menu *m) {
    if (m == NULL) return;

    if (m->at < (m->entries.count - 1)) {
        for (int i = m->at + 1; i < m->entries.count; i++) {
            if (m->entries.items[i].selectable) {
                m->at = i;
                break;
            }
        }
    } else {
        for (int i = m->entries.count - 1; i >= 0; i--) {
            if (m->entries.items[i].selectable) {
                m->at = i;
                break;
            }
        }
    }
}

void menu_select(Menu *m) {
    if (m == NULL) return;

    if (m->at < 0) {
        m->at = 0;
    } else if (m->at >= m->entries.count) {
        m->at = m->entries.count - 1;
    }

    if (m->entries.items[m->at].on_selected) {
        m->entries.items[m->at].on_selected(m->entries.items[m->at].ctx);
    }
}
