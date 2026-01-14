#include "level.h"
#include "game.h"

const LevelDef LEVELS[] = {
    // Level 1: 6 enemies in 2 rows, slow fire rate
    {
        .enemy_count = 6,
        .enemies = {
            {0, 0, 3, 5, 5, 1},
            {2, 1, 3, 5, 6, 1},
            {4, 0, 3, 5, 5, 1},
            {6, 1, 3, 5, 6, 1},
            {8, 0, 3, 5, 5, 1},
            {10, 1, 3, 5, 6, 1},
        },
    },
    // Level 2: 8 enemies, faster and tougher
    {
        .enemy_count = 8,
        .enemies = {
            {0, 0, 4, 8, 4, 1},
            {2, 1, 3, 6, 5, 1},
            {4, 0, 5, 10, 4, 1},
            {6, 1, 3, 6, 5, 1},
            {8, 0, 4, 8, 4, 1},
            {10, 1, 3, 6, 5, 1},
            {1, 2, 4, 8, 4, 1},
            {9, 2, 4, 8, 4, 1},
        },
    },
    // Level 3: 10 enemies, fast and dangerous
    {
        .enemy_count = 10,
        .enemies = {
            {0, 0, 5, 12, 3, 1},
            {2, 1, 4, 10, 4, 2},
            {4, 0, 6, 15, 3, 1},
            {6, 1, 4, 10, 4, 2},
            {8, 0, 5, 12, 3, 1},
            {10, 1, 4, 10, 4, 2},
            {1, 2, 5, 12, 3, 1},
            {5, 2, 6, 15, 3, 2},
            {9, 2, 5, 12, 3, 1},
            {3, 3, 4, 10, 4, 1},
        },
    },
};

const int LEVEL_COUNT = sizeof(LEVELS) / sizeof(LEVELS[0]);

void level_spawn(Game *g, const LevelDef *level) {
    for (int i = 0; i < level->enemy_count; i++) {
        const EnemyDef *e = &level->enemies[i];
        game_spawn_enemy(g, e->x, e->y, e->health, e->score, e->fire_freq, e->damage);
    }
}
