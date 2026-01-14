package si.ui;

import si.tui.Color;
import si.tui.Screen;
import java.util.ArrayList;
import java.util.List;

public final class Menu {
    private final List<MenuEntry> entries = new ArrayList<>();
    private int at = -1;
    private int x = 0;
    private int y = 0;

    public void addEntry(String name, boolean selectable, Runnable onSelected) {
        entries.add(new MenuEntry(name, selectable, onSelected));
    }

    public void addSeparator() {
        entries.add(MenuEntry.separator());
    }

    public void setPosition(int x, int y) {
        this.x = x;
        this.y = y;
    }

    public void moveUp() {
        if (entries.isEmpty()) {
            return;
        }

        if (at < 0) {
            for (int i = 0; i < entries.size(); i++) {
                if (entries.get(i).selectable()) {
                    at = i;
                    break;
                }
            }
        } else if (at > 0) {
            for (int i = at - 1; i >= 0; i--) {
                if (entries.get(i).selectable()) {
                    at = i;
                    break;
                }
            }
        }
    }

    public void moveDown() {
        if (entries.isEmpty()) {
            return;
        }

        int count = entries.size();

        if (at < count - 1) {
            for (int i = at + 1; i < count; i++) {
                if (entries.get(i).selectable()) {
                    at = i;
                    break;
                }
            }
        } else {
            for (int i = count - 1; i >= 0; i--) {
                if (entries.get(i).selectable()) {
                    at = i;
                    break;
                }
            }
        }
    }

    public void select() {
        if (entries.isEmpty()) {
            return;
        }

        if (at < 0) {
            at = 0;
        } else if (at >= entries.size()) {
            at = entries.size() - 1;
        }

        Runnable callback = entries.get(at).onSelected();
        if (callback != null) {
            callback.run();
        }
    }

    public void draw(Screen screen) {
        if (entries.isEmpty()) {
            return;
        }

        int maxLen = 0;
        for (MenuEntry entry : entries) {
            if (!entry.name().isEmpty()) {
                int len = entry.name().length() + 2;
                if (len > maxLen) {
                    maxLen = len;
                }
            }
        }

        int startX = x;
        int startY = y;
        if (startX == 0) {
            startX = (screen.width() - maxLen) / 2;
        }
        if (startY == 0) {
            startY = (screen.height() - entries.size()) / 2;
        }

        for (int i = 0; i < entries.size(); i++) {
            int yy = startY + i;
            MenuEntry entry = entries.get(i);

            if (entry.name().isEmpty()) {
                continue;
            }

            screen.putString(startX, yy, entry.name());

            Color fg = entry.selectable() ? Color.WHITE : Color.GRAY;
            int len = entry.name().length();
            for (int j = 0; j < len; j++) {
                screen.setFgColor(startX + j, yy, fg);
            }

            if (i == at) {
                screen.putChar(startX + len + 1, yy, "<");
                screen.setFgColor(startX + len + 1, yy, Color.YELLOW);
            }
        }
    }
}
