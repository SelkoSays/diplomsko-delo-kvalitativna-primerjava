package si.game;

import si.tui.Screen;

public abstract class Entity {
    protected int x;
    protected int y;
    protected boolean alive = true;
    protected final EntityType type;
    protected String shape;
    protected EntityColor color = EntityColor.NONE;

    protected Entity(int x, int y, EntityType type, String shape) {
        this.x = x;
        this.y = y;
        this.type = type;
        this.shape = shape;
    }

    public int damage(int amount) {
        return 0;
    }

    public void update() {
    }

    public void draw(Screen screen, int screenX, int screenY) {
        if (!alive) {
            return;
        }
        screen.putChar(screenX, screenY, shape);
        screen.setFgColor(screenX, screenY, color.toScreenColor());
    }

    public int x() {
        return x;
    }

    public int y() {
        return y;
    }

    public void setPos(int x, int y) {
        this.x = x;
        this.y = y;
    }

    public void move(int dx, int dy) {
        x += dx;
        y += dy;
    }

    public boolean isAlive() {
        return alive;
    }

    public void kill() {
        alive = false;
    }

    public void setAlive(boolean alive) {
        this.alive = alive;
    }

    public EntityType type() {
        return type;
    }

    public String shape() {
        return shape;
    }

    public void setShape(String s) {
        shape = s;
    }

    public EntityColor color() {
        return color;
    }

    public void setColor(EntityColor c) {
        color = c;
    }

    public static boolean collision(Entity a, Entity b) {
        return a.x == b.x && a.y == b.y;
    }
}
