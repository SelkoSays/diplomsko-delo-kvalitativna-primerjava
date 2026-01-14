package si.ui;

import si.tui.Screen;
import java.util.function.Supplier;

public final class ValueItem implements PanelItem {
    private final String label;
    private final Supplier<String> stringify;

    public ValueItem(String label, Supplier<String> stringify) {
        this.label = label;
        this.stringify = stringify;
    }

    @Override
    public void draw(Screen screen, int x, int y) {
        String value = stringify.get();
        screen.putString(x, y, label + ": " + value);
    }
}
