package si.game;

public final class Levels {
    public static final int LEVEL_COUNT = 3;

    public static final LevelDef[] LEVELS = {
        // Level 1: 6 enemies in 2 rows, slow fire rate
        new LevelDef(new EnemyDef[]{
            new EnemyDef(0, 0, 3, 5, 5, 1),
            new EnemyDef(2, 1, 3, 5, 6, 1),
            new EnemyDef(4, 0, 3, 5, 5, 1),
            new EnemyDef(6, 1, 3, 5, 6, 1),
            new EnemyDef(8, 0, 3, 5, 5, 1),
            new EnemyDef(10, 1, 3, 5, 6, 1),
        }),
        // Level 2: 8 enemies, faster and tougher
        new LevelDef(new EnemyDef[]{
            new EnemyDef(0, 0, 4, 8, 4, 1),
            new EnemyDef(2, 1, 3, 6, 5, 1),
            new EnemyDef(4, 0, 5, 10, 4, 1),
            new EnemyDef(6, 1, 3, 6, 5, 1),
            new EnemyDef(8, 0, 4, 8, 4, 1),
            new EnemyDef(10, 1, 3, 6, 5, 1),
            new EnemyDef(1, 2, 4, 8, 4, 1),
            new EnemyDef(9, 2, 4, 8, 4, 1),
        }),
        // Level 3: 10 enemies, fast and dangerous
        new LevelDef(new EnemyDef[]{
            new EnemyDef(0, 0, 5, 12, 3, 1),
            new EnemyDef(2, 1, 4, 10, 4, 2),
            new EnemyDef(4, 0, 6, 15, 3, 1),
            new EnemyDef(6, 1, 4, 10, 4, 2),
            new EnemyDef(8, 0, 5, 12, 3, 1),
            new EnemyDef(10, 1, 4, 10, 4, 2),
            new EnemyDef(1, 2, 5, 12, 3, 1),
            new EnemyDef(5, 2, 6, 15, 3, 2),
            new EnemyDef(9, 2, 5, 12, 3, 1),
            new EnemyDef(3, 3, 4, 10, 4, 1),
        }),
    };

    public static void spawnLevel(Game game, LevelDef level) {
        for (EnemyDef e : level.enemies()) {
            game.spawnEnemy(e.x(), e.y(), e.health(), e.score(), e.fireFreq(), e.damage());
        }
    }

    private Levels() {
    }
}
