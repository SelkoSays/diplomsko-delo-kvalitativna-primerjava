package si.game;

public final class Player extends Entity {
    private int health;
    private final int attackDamage;
    private final int cooldown;
    private int ticks;
    private boolean damaged = false;

    public Player(int x, int y, int health, int damage, int cooldown) {
        super(x, y, EntityType.PLAYER, "A");
        this.health = health;
        this.attackDamage = damage;
        this.cooldown = cooldown;
        this.ticks = cooldown;
        this.color = EntityColor.CYAN;
    }

    @Override
    public int damage(int amount) {
        if (health <= amount) {
            health = 0;
            kill();
        } else {
            health -= amount;
        }
        damaged = true;
        color = EntityColor.NONE;
        return 0;
    }

    @Override
    public void update() {
        ticks++;
        if (damaged) {
            damaged = false;
        } else {
            color = EntityColor.CYAN;
        }
    }

    public boolean canFire() {
        return ticks >= cooldown;
    }

    public void resetFireCooldown() {
        ticks = 0;
    }

    public int health() {
        return health;
    }

    public void setHealth(int h) {
        health = h;
        ticks = cooldown;
    }

    public int attackDamage() {
        return attackDamage;
    }
}
