package si.tui;

public final class Escape {
    public static final String ALT_SCREEN_ON = "\u001b[?1049h";
    public static final String ALT_SCREEN_OFF = "\u001b[?1049l";
    public static final String CURSOR_HIDE = "\u001b[?25l";
    public static final String CURSOR_SHOW = "\u001b[?25h";
    public static final String MOUSE_ON = "\u001b[?1003h\u001b[?1006h";
    public static final String MOUSE_OFF = "\u001b[?1003l\u001b[?1006l";
    public static final String CLEAR_SCREEN = "\u001b[2J";
    public static final String RESET_ATTRS = "\u001b[0m";
    public static final String HOME = "\u001b[H";
    public static final String RESET_FG = "\u001b[39m";
    public static final String RESET_BG = "\u001b[49m";

    public static String moveTo(int row, int col) {
        return "\u001b[" + (row + 1) + ";" + (col + 1) + "H";
    }

    public static String fgColor(int r, int g, int b) {
        return "\u001b[38;2;" + r + ";" + g + ";" + b + "m";
    }

    public static String bgColor(int r, int g, int b) {
        return "\u001b[48;2;" + r + ";" + g + ";" + b + "m";
    }

    private Escape() {
    }
}
