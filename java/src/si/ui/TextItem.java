package si.ui;

import si.tui.Screen;

public final class TextItem implements PanelItem {
    private final String text;

    public TextItem(String text) {
        this.text = text;
    }

    @Override
    public void draw(Screen screen, int x, int y) {
        screen.putString(x, y, text);
    }
}
