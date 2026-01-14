#include "entity.h"

Color ecolor_to_color(EntityColor ecolor) {
    switch (ecolor) {
    case ECOLOR_RED:    return COLOR_RED;
    case ECOLOR_GREEN:  return COLOR_GREEN;
    case ECOLOR_BLUE:   return COLOR_BLUE;
    case ECOLOR_CYAN:   return COLOR_CYAN;
    case ECOLOR_YELLOW: return COLOR_YELLOW;
    default:            return COLOR_WHITE;
    }
}

int entity_damage(Entity *e, int amount) {
    if (e && e->damage) {
        return e->damage(e, amount);
    }
    return 0;
}

void entity_update(Entity *e) {
    if (e && e->update) {
        e->update(e);
    }
}

bool entity_collision(const Entity *e1, const Entity *e2) {
    return (e1->x == e2->x && e1->y == e2->y);
}

void entity_draw(Entity *e, Screen *s, int x, int y) {
    if (e == NULL || !e->alive || s == NULL) {
        return;
    }

    Color fg = ecolor_to_color(e->color);
    screen_putc(s, x, y, e->shape);
    screen_set_fg_color(s, x, y, fg);
}
