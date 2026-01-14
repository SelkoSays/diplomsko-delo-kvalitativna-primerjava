#pragma once

#include <array>
#include <span>

namespace game {

// Forward declaration
class Game;

struct EnemyDef {
    int x, y;
    int health;
    int score;
    int fireFreq;
    int damage;
};

struct LevelDef {
    int enemyCount;
    std::array<EnemyDef, 32> enemies;
};

// Level data
extern const std::array<LevelDef, 3> LEVELS;
inline constexpr int LEVEL_COUNT = 3;

// Spawn all enemies for a level
void spawnLevel(Game& game, const LevelDef& level);

} // namespace game
