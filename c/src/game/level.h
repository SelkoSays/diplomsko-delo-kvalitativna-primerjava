#ifndef GAME_LEVEL_H
#define GAME_LEVEL_H

// Forward declaration
struct Game;

typedef struct EnemyDef {
    int x, y;
    int health;
    int score;
    int fire_freq;
    int damage;
} EnemyDef;

typedef struct LevelDef {
    int enemy_count;
    EnemyDef enemies[32];
} LevelDef;

extern const LevelDef LEVELS[];
extern const int LEVEL_COUNT;

// Spawn all enemies for a level
void level_spawn(struct Game *g, const LevelDef *level);

#endif // GAME_LEVEL_H
