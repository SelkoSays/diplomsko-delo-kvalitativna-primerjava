package si.tui;

public record Color(int value) {
    public static final Color NONE = new Color(0x00000000);
    public static final Color BLACK = new Color(0xFF000000);
    public static final Color WHITE = new Color(0xFFFFFFFF);
    public static final Color RED = new Color(0xFFFF0000);
    public static final Color GREEN = new Color(0xFF00FF00);
    public static final Color BLUE = new Color(0xFF0000FF);
    public static final Color YELLOW = new Color(0xFFFFFF00);
    public static final Color CYAN = new Color(0xFF00FFFF);
    public static final Color MAGENTA = new Color(0xFFFF00FF);
    public static final Color GRAY = new Color(0xFF808080);

    public static Color rgb(int r, int g, int b) {
        return new Color(0xFF000000 | (r << 16) | (g << 8) | b);
    }

    public int a() {
        return (value >> 24) & 0xFF;
    }

    public int r() {
        return (value >> 16) & 0xFF;
    }

    public int g() {
        return (value >> 8) & 0xFF;
    }

    public int b() {
        return value & 0xFF;
    }

    public boolean isSet() {
        return (value >> 24) != 0;
    }
}
