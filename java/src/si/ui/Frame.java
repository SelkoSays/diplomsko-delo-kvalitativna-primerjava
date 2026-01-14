package si.ui;

import si.tui.Color;
import si.tui.Screen;
import java.util.ArrayList;
import java.util.List;

public final class Frame {
    private static final String UP_LEFT = "╭";
    private static final String UP_RIGHT = "╮";
    private static final String DOWN_LEFT = "╰";
    private static final String DOWN_RIGHT = "╯";
    private static final String HDASH = "─";
    private static final String VDASH = "│";
    private static final String FORK_DOWN = "┬";
    private static final String FORK_LEFT = "┤";
    private static final String FORK_UP = "┴";
    private static final String FORK_RIGHT = "├";

    private static final Color BORDER_COLOR = Color.GRAY;

    private final int w, h;
    private final int x, y;
    private final Frame[] split = new Frame[2];
    private final List<Widget> widgets = new ArrayList<>();

    public Frame(int w, int h, int x, int y) {
        this.w = w;
        this.h = h;
        this.x = x;
        this.y = y;
    }

    public Frame[] split(int coord, FrameSplit splitType) {
        if (split[0] != null || split[1] != null) {
            throw new IllegalStateException("Frame is already split");
        }

        int pad = 2;

        if (splitType == FrameSplit.VERTICAL) {
            if (coord >= (w - pad) || coord < pad) {
                throw new IllegalArgumentException("Invalid vertical split");
            }
            split[0] = new Frame(coord, h, x, y);
            split[1] = new Frame(w - coord + 1, h, x + coord - 1, y);
        } else {
            if (coord >= (h - pad) || coord < pad) {
                throw new IllegalArgumentException("Invalid horizontal split");
            }
            split[0] = new Frame(w, coord, x, y);
            split[1] = new Frame(w, h - coord + 1, x, y + coord - 1);
        }

        return split;
    }

    public void addWidget(Widget widget) {
        widgets.add(widget);
    }

    public void draw(Screen screen) {
        drawStage(screen, 1);
        drawStage(screen, 2);
        drawStage(screen, 3);
    }

    private void drawStage(Screen screen, int stage) {
        if (split[0] != null) {
            split[0].drawStage(screen, stage);
        }
        if (split[1] != null) {
            split[1].drawStage(screen, stage);
        }

        if (stage == 1) {
            drawBorders(screen);
        } else if (stage == 2) {
            drawJoints(screen);
        } else if (stage == 3) {
            drawWidgets(screen);
        }
    }

    private void drawBorders(Screen screen) {
        screen.putChar(x, y, UP_LEFT);
        screen.setFgColor(x, y, BORDER_COLOR);

        for (int i = 1; i < (w - 1); i++) {
            screen.putChar(x + i, y, HDASH);
            screen.setFgColor(x + i, y, BORDER_COLOR);
        }

        screen.putChar(x + w - 1, y, UP_RIGHT);
        screen.setFgColor(x + w - 1, y, BORDER_COLOR);

        for (int i = 1; i < (h - 1); i++) {
            screen.putChar(x, y + i, VDASH);
            screen.setFgColor(x, y + i, BORDER_COLOR);
            screen.putChar(x + w - 1, y + i, VDASH);
            screen.setFgColor(x + w - 1, y + i, BORDER_COLOR);
        }

        screen.putChar(x, y + h - 1, DOWN_LEFT);
        screen.setFgColor(x, y + h - 1, BORDER_COLOR);

        for (int i = 1; i < (w - 1); i++) {
            screen.putChar(x + i, y + h - 1, HDASH);
            screen.setFgColor(x + i, y + h - 1, BORDER_COLOR);
        }

        screen.putChar(x + w - 1, y + h - 1, DOWN_RIGHT);
        screen.setFgColor(x + w - 1, y + h - 1, BORDER_COLOR);
    }

    private void drawJoints(Screen screen) {
        if (split[1] != null) {
            if (y == split[1].y()) {
                screen.putChar(split[1].x(), y, FORK_DOWN);
                screen.setFgColor(split[1].x(), y, BORDER_COLOR);
                screen.putChar(split[1].x(), y + h - 1, FORK_UP);
                screen.setFgColor(split[1].x(), y + h - 1, BORDER_COLOR);
            } else {
                screen.putChar(x, split[1].y(), FORK_RIGHT);
                screen.setFgColor(x, split[1].y(), BORDER_COLOR);
                screen.putChar(x + w - 1, split[1].y(), FORK_LEFT);
                screen.setFgColor(x + w - 1, split[1].y(), BORDER_COLOR);
            }
        }
    }

    private void drawWidgets(Screen screen) {
        BBox bbox = new BBox(x + 1, y + 1, w - 2, h - 2);
        for (Widget widget : widgets) {
            if (widget != null) {
                widget.draw(screen, bbox);
            }
        }
    }

    public int width() {
        return w;
    }

    public int height() {
        return h;
    }

    public int x() {
        return x;
    }

    public int y() {
        return y;
    }
}
