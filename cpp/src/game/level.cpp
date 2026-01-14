#include "level.hpp"
#include "game.hpp"

namespace game {

const std::array<LevelDef, 3> LEVELS = {{
    // Level 1: 6 enemies in 2 rows, slow fire rate
    {
        6,  // enemyCount
        {{
            {0, 0, 3, 5, 5, 1},
            {2, 1, 3, 5, 6, 1},
            {4, 0, 3, 5, 5, 1},
            {6, 1, 3, 5, 6, 1},
            {8, 0, 3, 5, 5, 1},
            {10, 1, 3, 5, 6, 1},
        }},
    },
    // Level 2: 8 enemies, faster and tougher
    {
        8,  // enemyCount
        {{
            {0, 0, 4, 8, 4, 1},
            {2, 1, 3, 6, 5, 1},
            {4, 0, 5, 10, 4, 1},
            {6, 1, 3, 6, 5, 1},
            {8, 0, 4, 8, 4, 1},
            {10, 1, 3, 6, 5, 1},
            {1, 2, 4, 8, 4, 1},
            {9, 2, 4, 8, 4, 1},
        }},
    },
    // Level 3: 10 enemies, fast and dangerous
    {
        10,  // enemyCount
        {{
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
        }},
    },
}};

void spawnLevel(Game& game, const LevelDef& level) {
    for (int i = 0; i < level.enemyCount; ++i) {
        const auto& e = level.enemies[i];
        game.spawnEnemy(e.x, e.y, e.health, e.score, e.fireFreq, e.damage);
    }
}

} // namespace game
