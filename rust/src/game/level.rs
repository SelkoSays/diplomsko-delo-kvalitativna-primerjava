//! Level definitions

use crate::game::game::Game;

/// Enemy definition for level spawning
#[derive(Clone, Copy, Debug)]
pub struct EnemyDef {
    pub x: i32,
    pub y: i32,
    pub health: i32,
    pub score: i32,
    pub fire_freq: i32,
    pub damage: i32,
}

/// Level definition
pub struct LevelDef {
    pub enemies: &'static [EnemyDef],
}

pub const LEVEL_COUNT: i32 = 3;

pub static LEVELS: [LevelDef; 3] = [
    // Level 1: 6 enemies in 2 rows, slow fire rate
    LevelDef {
        enemies: &[
            EnemyDef { x: 0, y: 0, health: 3, score: 5, fire_freq: 5, damage: 1 },
            EnemyDef { x: 2, y: 1, health: 3, score: 5, fire_freq: 6, damage: 1 },
            EnemyDef { x: 4, y: 0, health: 3, score: 5, fire_freq: 5, damage: 1 },
            EnemyDef { x: 6, y: 1, health: 3, score: 5, fire_freq: 6, damage: 1 },
            EnemyDef { x: 8, y: 0, health: 3, score: 5, fire_freq: 5, damage: 1 },
            EnemyDef { x: 10, y: 1, health: 3, score: 5, fire_freq: 6, damage: 1 },
        ],
    },
    // Level 2: 8 enemies, faster and tougher
    LevelDef {
        enemies: &[
            EnemyDef { x: 0, y: 0, health: 4, score: 8, fire_freq: 4, damage: 1 },
            EnemyDef { x: 2, y: 1, health: 3, score: 6, fire_freq: 5, damage: 1 },
            EnemyDef { x: 4, y: 0, health: 5, score: 10, fire_freq: 4, damage: 1 },
            EnemyDef { x: 6, y: 1, health: 3, score: 6, fire_freq: 5, damage: 1 },
            EnemyDef { x: 8, y: 0, health: 4, score: 8, fire_freq: 4, damage: 1 },
            EnemyDef { x: 10, y: 1, health: 3, score: 6, fire_freq: 5, damage: 1 },
            EnemyDef { x: 1, y: 2, health: 4, score: 8, fire_freq: 4, damage: 1 },
            EnemyDef { x: 9, y: 2, health: 4, score: 8, fire_freq: 4, damage: 1 },
        ],
    },
    // Level 3: 10 enemies, fast and dangerous
    LevelDef {
        enemies: &[
            EnemyDef { x: 0, y: 0, health: 5, score: 12, fire_freq: 3, damage: 1 },
            EnemyDef { x: 2, y: 1, health: 4, score: 10, fire_freq: 4, damage: 2 },
            EnemyDef { x: 4, y: 0, health: 6, score: 15, fire_freq: 3, damage: 1 },
            EnemyDef { x: 6, y: 1, health: 4, score: 10, fire_freq: 4, damage: 2 },
            EnemyDef { x: 8, y: 0, health: 5, score: 12, fire_freq: 3, damage: 1 },
            EnemyDef { x: 10, y: 1, health: 4, score: 10, fire_freq: 4, damage: 2 },
            EnemyDef { x: 1, y: 2, health: 5, score: 12, fire_freq: 3, damage: 1 },
            EnemyDef { x: 5, y: 2, health: 6, score: 15, fire_freq: 3, damage: 2 },
            EnemyDef { x: 9, y: 2, health: 5, score: 12, fire_freq: 3, damage: 1 },
            EnemyDef { x: 3, y: 3, health: 4, score: 10, fire_freq: 4, damage: 1 },
        ],
    },
];

pub fn spawn_level(game: &mut Game, level: &LevelDef) {
    for enemy in level.enemies {
        game.spawn_enemy(
            enemy.x,
            enemy.y,
            enemy.health,
            enemy.score,
            enemy.fire_freq,
            enemy.damage,
        );
    }
}
