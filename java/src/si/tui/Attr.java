package si.tui;

public final class Attr {
    public static final int NONE = 0;
    public static final int BOLD = 1 << 0;
    public static final int DIM = 1 << 1;
    public static final int ITALIC = 1 << 2;
    public static final int UNDERLINE = 1 << 3;
    public static final int BLINK = 1 << 4;
    public static final int REVERSE = 1 << 5;
    public static final int CROSSED = 1 << 6;

    private Attr() {
    }
}
