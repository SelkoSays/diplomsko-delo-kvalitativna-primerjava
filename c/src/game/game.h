#ifndef GAME_GAME_H
#define GAME_GAME_H

#include "entity.h"
#include "../ui/widget.h"
#include "../ui/grid.h"
#include "../input/input.h"

typedef enum GameStatus {
    GAME_STATUS_RUNNING,
    GAME_STATUS_PAUSED,
    GAME_STATUS_LEVEL_COMPLETED,
    GAME_STATUS_FINISHED,
    GAME_STATUS_GAME_OVER,
} GameStatus;

typedef struct Game {
    struct { int w, h; } bounds;
    int level;
    int score;
    GameStatus status;

    Vec(Entity) enemies;
    Vec(Entity) bullets;
    Entity player;

    i64 last_update;
    i64 tps;  // Ticks Per Second

    Grid *grid;  // game grid for rendering

    // Statistics
    int shots_fired;
    int shots_hit;
    int kills;
    i64 elapsed_time;  // in microseconds

    // Computed stats for display
    int time_seconds;
    int accuracy_percent;
} Game;

// Initialize game state
void game_init(Game *g, int w, int h, i64 tps);

// Free game resources
void game_free(Game *g);

// Reset game for new game (keeps player allocated, resets state)
void game_reset(Game *g);

// Spawn player at position
Entity *game_spawn_player(Game *g, int x, int y, int health, int damage, int cooldown);

// Spawn enemy at position
Entity *game_spawn_enemy(Game *g, int x, int y, int health, int score, int fire_freq, int damage);

// Spawn bullet at position
Entity *game_spawn_bullet(Game *g, int x, int y, int damage, EntityType owner);

// Update all entities
void game_update(Game *g, i64 udt);

// Remove dead entities
void game_remove_dead_entities(Game *g);

// Process input event
void game_process_input(Game *g, Event *ev);

// Draw game (as frame item callback)
void game_draw(void *ctx, Screen *s, BBox b);

#endif // GAME_GAME_H
