package si.game;

import si.tui.Color;

public enum EntityColor {
    NONE(Color.WHITE),
    RED(Color.RED),
    GREEN(Color.GREEN),
    BLUE(Color.BLUE),
    CYAN(Color.CYAN),
    YELLOW(Color.YELLOW);

    private final Color screenColor;

    EntityColor(Color c) {
        this.screenColor = c;
    }

    public Color toScreenColor() {
        return screenColor;
    }
}
