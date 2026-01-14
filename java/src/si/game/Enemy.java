package si.game;

public final class Enemy extends Entity {
    private int health;
    private final int score;
    private final int fireFreq;
    private int lastFired = 0;
    private final int bulletDamage;
    private Game game;

    public Enemy(int x, int y, int health, int score, int fireFreq, int damage) {
        super(x, y, EntityType.ENEMY, "V");
        this.health = health;
        this.score = score;
        this.fireFreq = fireFreq;
        this.bulletDamage = damage;
        this.color = EntityColor.NONE;
    }

    public void setGame(Game game) {
        this.game = game;
    }

    @Override
    public int damage(int amount) {
        if (health <= amount) {
            health = 0;
            kill();
            return score;
        }
        health -= amount;
        return 0;
    }

    @Override
    public void update() {
        color = EntityColor.NONE;
        lastFired++;

        if (lastFired >= fireFreq - 1) {
            color = EntityColor.RED;
        }

        if (lastFired >= fireFreq) {
            if (game != null) {
                Bullet bullet = game.spawnBullet(x, y + 1, bulletDamage, EntityType.ENEMY);
                bullet.setShape("|");
            }
            lastFired = 0;
        }
    }

    public int scoreValue() {
        return score;
    }
}
