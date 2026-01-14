#include "game.h"
#include <stdlib.h>

static int damage_player(Entity *e, int amount) {
    Player *p = (Player *)e->ctx;
    if (p->health <= amount) {
        p->health = 0;
        e->alive = false;
    } else {
        p->health -= amount;
    }
    p->damaged = true;
    e->color = ECOLOR_NONE;
    return 0;
}

static void update_player(Entity *e) {
    Player *p = (Player *)e->ctx;
    p->ticks++;
    if (p->damaged) {
        p->damaged = false;
    } else {
        e->color = ECOLOR_CYAN;
    }
}

static int damage_enemy(Entity *e, int amount) {
    Enemy *p = (Enemy *)e->ctx;
    if (p->health <= amount) {
        p->health = 0;
        e->alive = false;
        return p->score;
    }
    p->health -= amount;
    return 0;
}

static void update_enemy(Entity *e) {
    Enemy *p = (Enemy *)e->ctx;

    e->color = ECOLOR_NONE;
    p->last_fired += 1;

    if (p->last_fired >= (p->fire_freq - 1)) {
        e->color = ECOLOR_RED;
    }

    if (p->last_fired >= p->fire_freq) {
        Entity *b = game_spawn_bullet(p->game, e->x, e->y + 1, p->damage, ENTITY_ENEMY);
        b->shape = "|";
        p->last_fired = 0;
    }
}

static void update_bullet(Entity *e) {
    Bullet *b = (Bullet *)e->ctx;
    b->last_moved++;

    if (b->last_moved < b->move_freq) {
        return;
    }
    b->last_moved = 0;

    if (b->owner == ENTITY_ENEMY) {
        if (entity_collision(e, &b->game->player)) {
            entity_damage(&b->game->player, b->damage);
            e->alive = false;
        }
        // Move down
        if (e->y + 1 >= b->game->bounds.h) {
            e->alive = false;
            return;
        }
        e->y += 1;
    } else {
        for (int i = 0; i < b->game->enemies.count; i++) {
            if (entity_collision(e, &b->game->enemies.items[i])) {
                int score = entity_damage(&b->game->enemies.items[i], b->damage);
                b->game->score += score;
                b->game->shots_hit++;
                if (score > 0) {
                    b->game->kills++;
                }
                e->alive = false;
                break;
            }
        }
        // Move up
        if (e->y - 1 < 0) {
            e->alive = false;
            return;
        }
        e->y -= 1;
    }
}

Entity *game_spawn_player(Game *g, int x, int y, int health, int damage, int cooldown) {
    Player *p = (Player *)malloc(sizeof(Player));
    p->health = health;
    p->damage = damage;
    p->cooldown = cooldown;
    p->ticks = cooldown;
    p->damaged = false;

    Entity e = {
        .x = x,
        .y = y,
        .alive = true,
        .shape = "A",
        .type = ENTITY_PLAYER,
        .damage = damage_player,
        .update = update_player,
        .color = ECOLOR_CYAN,
        .ctx = p,
    };

    g->player = e;
    return &g->player;
}

Entity *game_spawn_enemy(Game *g, int x, int y, int health, int score, int fire_freq, int damage) {
    Enemy *p = (Enemy *)malloc(sizeof(Enemy));
    p->health = health;
    p->score = score;
    p->game = g;
    p->fire_freq = fire_freq;
    p->last_fired = 0;
    p->damage = damage;

    Entity e = {
        .x = x,
        .y = y,
        .alive = true,
        .shape = "V",
        .type = ENTITY_ENEMY,
        .damage = damage_enemy,
        .update = update_enemy,
        .color = ECOLOR_NONE,
        .ctx = p,
    };

    list_append(&g->enemies, e);
    return &g->enemies.items[g->enemies.count - 1];
}

Entity *game_spawn_bullet(Game *g, int x, int y, int damage, EntityType owner) {
    Bullet *b = (Bullet *)malloc(sizeof(Bullet));
    b->damage = damage;
    b->owner = owner;
    b->game = g;
    b->move_freq = 1;
    b->last_moved = 0;

    Entity e = {
        .x = x,
        .y = y,
        .alive = true,
        .shape = "0",
        .type = ENTITY_BULLET,
        .damage = NULL,
        .update = update_bullet,
        .color = ECOLOR_NONE,
        .ctx = b,
    };

    list_append(&g->bullets, e);
    return &g->bullets.items[g->bullets.count - 1];
}

void game_init(Game *g, int w, int h, i64 tps) {
    Game gi = {
        .bounds = {.w = w, .h = h},
        .level = 1,
        .score = 0,
        .status = GAME_STATUS_RUNNING,
        .enemies = {0},
        .bullets = {0},
        .player = {0},
        .last_update = 0,
        .tps = tps,
        .grid = NULL,
        .shots_fired = 0,
        .shots_hit = 0,
        .kills = 0,
        .elapsed_time = 0,
        .time_seconds = 0,
        .accuracy_percent = 0,
    };
    *g = gi;
    game_spawn_player(g, (w - 1) / 2, h - 1, 5, 1, 2);
}

void game_free(Game *g) {
    if (g == NULL) return;

    for (int i = 0; i < g->bullets.count; i++) {
        free(g->bullets.items[i].ctx);
    }
    list_free(g->bullets);

    for (int i = 0; i < g->enemies.count; i++) {
        free(g->enemies.items[i].ctx);
    }
    list_free(g->enemies);

    free(g->player.ctx);

    if (g->grid) {
        grid_free(g->grid);
    }
}

void game_reset(Game *g) {
    for (int i = 0; i < g->bullets.count; i++) {
        free(g->bullets.items[i].ctx);
    }
    g->bullets.count = 0;

    for (int i = 0; i < g->enemies.count; i++) {
        free(g->enemies.items[i].ctx);
    }
    g->enemies.count = 0;

    // Reset player
    Player *p = (Player *)g->player.ctx;
    p->health = 5;
    p->ticks = p->cooldown;
    p->damaged = false;
    g->player.alive = true;
    g->player.x = (g->bounds.w - 1) / 2;
    g->player.y = g->bounds.h - 1;
    g->player.color = ECOLOR_CYAN;

    // Reset game state
    g->level = 1;
    g->score = 0;
    g->status = GAME_STATUS_RUNNING;

    // Reset statistics
    g->shots_fired = 0;
    g->shots_hit = 0;
    g->kills = 0;
    g->elapsed_time = 0;
    g->time_seconds = 0;
    g->accuracy_percent = 0;
}

void game_update(Game *g, i64 udt) {
    g->elapsed_time += udt;

    g->time_seconds = (int)(g->elapsed_time / US_PER_SEC);
    if (g->shots_fired > 0) {
        g->accuracy_percent = (g->shots_hit * 100) / g->shots_fired;
    } else {
        g->accuracy_percent = 0;
    }

    for (int i = 0; i < g->bullets.count; i++) {
        entity_update(&g->bullets.items[i]);
    }

    for (int i = 0; i < g->enemies.count; i++) {
        entity_update(&g->enemies.items[i]);
    }

    entity_update(&g->player);
}

void game_remove_dead_entities(Game *g) {
    int i = 0;
    while (i < g->bullets.count) {
        if (!g->bullets.items[i].alive) {
            free(g->bullets.items[i].ctx);
            list_remove_unordered(&g->bullets, i);
            continue;
        }
        i++;
    }

    i = 0;
    while (i < g->enemies.count) {
        if (!g->enemies.items[i].alive) {
            free(g->enemies.items[i].ctx);
            list_remove_unordered(&g->enemies, i);
            continue;
        }
        i++;
    }

    if (!g->player.alive) {
        g->status = GAME_STATUS_GAME_OVER;
    }
}

void game_process_input(Game *g, Event *ev) {
    if (ev->type != EVENT_KEY) return;

    Player *p = (Player *)g->player.ctx;

    // Movement
    if (key_is_char(ev, 'a') || key_is_char(ev, 'A') || key_is(ev, KEY_LEFT)) {
        if (g->player.x > 0) {
            g->player.x--;
        }
    } else if (key_is_char(ev, 'd') || key_is_char(ev, 'D') || key_is(ev, KEY_RIGHT)) {
        if (g->player.x < g->bounds.w - 1) {
            g->player.x++;
        }
    }
    // Fire
    else if (key_is_char(ev, ' ')) {
        if (p->ticks >= p->cooldown) {
            game_spawn_bullet(g, g->player.x, g->player.y - 1, p->damage, ENTITY_PLAYER);
            p->ticks = 0;
            g->shots_fired++;
        }
    }
}

// Cell draw callback for entities
static void draw_entity_cell(void *ctx, Screen *s, int x, int y) {
    Entity *e = (Entity *)ctx;
    entity_draw(e, s, x, y);
}

void game_draw(void *ctx, Screen *s, BBox b) {
    Game *g = (Game *)ctx;

    if (g->grid == NULL) {
        g->grid = grid_new(g->bounds.w, g->bounds.h);
    }

    for (u32 y = 0; y < g->grid->rows; y++) {
        for (u32 x = 0; x < g->grid->cols; x++) {
            grid_at(g->grid, x, y)->draw = NULL;
        }
    }

    for (int i = 0; i < g->bullets.count; i++) {
        Entity *e = &g->bullets.items[i];
        GridCell *c = grid_at(g->grid, e->x, e->y);
        if (c) {
            c->ctx = e;
            c->draw = draw_entity_cell;
        }
    }

    for (int i = 0; i < g->enemies.count; i++) {
        Entity *e = &g->enemies.items[i];
        GridCell *c = grid_at(g->grid, e->x, e->y);
        if (c) {
            c->ctx = e;
            c->draw = draw_entity_cell;
        }
    }

    Entity *e = &g->player;
    if (e->alive) {
        GridCell *c = grid_at(g->grid, e->x, e->y);
        if (c) {
            c->ctx = e;
            c->draw = draw_entity_cell;
        }
    }

    grid_draw(g->grid, s, b);
}
