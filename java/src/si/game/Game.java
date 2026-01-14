package si.game;

import si.input.Event;
import si.input.KeyCode;
import si.input.KeyEvent;
import si.tui.Screen;
import si.ui.BBox;
import si.ui.Grid;
import si.ui.GridCell;
import si.ui.Widget;

import java.util.ArrayList;
import java.util.List;

public final class Game implements Widget {
    public record Bounds(int w, int h) {
    }

    private final Bounds bounds;
    private int level = 1;
    private int score = 0;
    private GameStatus status = GameStatus.RUNNING;

    private Player player;
    private final List<Enemy> enemies = new ArrayList<>();
    private final List<Bullet> bullets = new ArrayList<>();

    private final Grid grid;
    private final int tps;

    private int shotsFired = 0;
    private int shotsHit = 0;
    private int kills = 0;
    private long elapsedTime = 0;

    public Game(int width, int height, int tps) {
        this.bounds = new Bounds(width, height);
        this.grid = new Grid(width, height);
        this.tps = tps;
    }

    public Player spawnPlayer(int x, int y, int health, int damage, int cooldown) {
        player = new Player(x, y, health, damage, cooldown);
        return player;
    }

    public Enemy spawnEnemy(int x, int y, int health, int score, int fireFreq, int damage) {
        Enemy enemy = new Enemy(x, y, health, score, fireFreq, damage);
        enemy.setGame(this);
        enemies.add(enemy);
        return enemy;
    }

    public Bullet spawnBullet(int x, int y, int damage, EntityType owner) {
        Bullet bullet = new Bullet(x, y, damage, owner);
        bullet.setGame(this);
        bullets.add(bullet);
        return bullet;
    }

    public void update(long deltaTime) {
        elapsedTime += deltaTime;

        for (Bullet bullet : bullets) {
            if (bullet != null) {
                bullet.update();
            }
        }
        for (Enemy enemy : enemies) {
            if (enemy != null) {
                enemy.update();
            }
        }
        if (player != null) {
            player.update();
        }
    }

    public void removeDeadEntities() {
        bullets.removeIf(b -> b == null || !b.isAlive());
        enemies.removeIf(e -> e == null || !e.isAlive());

        if (player != null && !player.isAlive()) {
            status = GameStatus.GAME_OVER;
        }
    }

    public void reset() {
        bullets.clear();
        enemies.clear();

        if (player != null) {
            player.setHealth(5);
            player.setAlive(true);
            player.setPos((bounds.w - 1) / 2, bounds.h - 1);
            player.setColor(EntityColor.CYAN);
        }

        level = 1;
        score = 0;
        status = GameStatus.RUNNING;
        shotsFired = 0;
        shotsHit = 0;
        kills = 0;
        elapsedTime = 0;
    }

    public void processInput(Event ev) {
        if (!(ev instanceof KeyEvent keyEv)) {
            return;
        }

        if (keyEv.isChar('a') || keyEv.isChar('A') || keyEv.isKey(KeyCode.LEFT)) {
            if (player != null && player.x() > 0) {
                player.move(-1, 0);
            }
        } else if (keyEv.isChar('d') || keyEv.isChar('D') || keyEv.isKey(KeyCode.RIGHT)) {
            if (player != null && player.x() < bounds.w - 1) {
                player.move(1, 0);
            }
        } else if (keyEv.isChar(' ')) {
            if (player != null && player.canFire()) {
                spawnBullet(player.x(), player.y() - 1, player.attackDamage(), EntityType.PLAYER);
                player.resetFireCooldown();
                shotsFired++;
            }
        }
    }

    private void placeEntitiesOnGrid() {
        grid.clearCells();

        for (Bullet bullet : bullets) {
            if (bullet != null && bullet.isAlive()) {
                GridCell cell = grid.at(bullet.x(), bullet.y());
                if (cell != null) {
                    cell.setDrawCallback((screen, pos) -> bullet.draw(screen, pos[0], pos[1]));
                }
            }
        }

        for (Enemy enemy : enemies) {
            if (enemy != null && enemy.isAlive()) {
                GridCell cell = grid.at(enemy.x(), enemy.y());
                if (cell != null) {
                    cell.setDrawCallback((screen, pos) -> enemy.draw(screen, pos[0], pos[1]));
                }
            }
        }

        if (player != null && player.isAlive()) {
            GridCell cell = grid.at(player.x(), player.y());
            if (cell != null) {
                cell.setDrawCallback((screen, pos) -> player.draw(screen, pos[0], pos[1]));
            }
        }
    }

    @Override
    public void draw(Screen screen, BBox bbox) {
        placeEntitiesOnGrid();
        grid.draw(screen, bbox);
    }

    public GameStatus status() {
        return status;
    }

    public void setStatus(GameStatus s) {
        status = s;
    }

    public int score() {
        return score;
    }

    public void addScore(int s) {
        score += s;
    }

    public int level() {
        return level;
    }

    public void setLevel(int l) {
        level = l;
    }

    public void incrementLevel() {
        level++;
    }

    public int tps() {
        return tps;
    }

    public Player player() {
        return player;
    }

    public List<Enemy> enemies() {
        return enemies;
    }

    public List<Bullet> bullets() {
        return bullets;
    }

    public int bulletCount() {
        return bullets.size();
    }

    public int enemyCount() {
        return enemies.size();
    }

    public Bounds bounds() {
        return bounds;
    }

    public int shotsFired() {
        return shotsFired;
    }

    public int shotsHit() {
        return shotsHit;
    }

    public int kills() {
        return kills;
    }

    public int timeSeconds() {
        return (int) (elapsedTime / 1_000_000L);
    }

    public int accuracyPercent() {
        return shotsFired > 0 ? (shotsHit * 100) / shotsFired : 0;
    }

    public void incrementShotsFired() {
        shotsFired++;
    }

    public void incrementShotsHit() {
        shotsHit++;
    }

    public void incrementKills() {
        kills++;
    }
}
