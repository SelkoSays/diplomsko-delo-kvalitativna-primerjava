package si.game;

public final class Bullet extends Entity {
    private final EntityType owner;
    private final int bulletDamage;
    private final int moveFreq = 1;
    private int lastMoved = 0;
    private Game game;

    public Bullet(int x, int y, int damage, EntityType owner) {
        super(x, y, EntityType.BULLET, "0");
        this.owner = owner;
        this.bulletDamage = damage;
        this.color = EntityColor.NONE;
    }

    public void setGame(Game game) {
        this.game = game;
    }

    public EntityType owner() {
        return owner;
    }

    public int bulletDamage() {
        return bulletDamage;
    }

    @Override
    public void update() {
        lastMoved++;
        if (lastMoved < moveFreq) {
            return;
        }
        lastMoved = 0;

        if (game == null) {
            return;
        }

        if (owner == EntityType.ENEMY) {
            Player player = game.player();
            if (player != null && collision(this, player)) {
                player.damage(bulletDamage);
                kill();
                return;
            }
            if (y + 1 >= game.bounds().h()) {
                kill();
                return;
            }
            y++;
        } else {
            for (Enemy enemy : game.enemies()) {
                if (enemy != null && collision(this, enemy)) {
                    int scoreGained = enemy.damage(bulletDamage);
                    game.addScore(scoreGained);
                    game.incrementShotsHit();
                    if (scoreGained > 0) {
                        game.incrementKills();
                    }
                    kill();
                    return;
                }
            }
            if (y - 1 < 0) {
                kill();
                return;
            }
            y--;
        }
    }
}
