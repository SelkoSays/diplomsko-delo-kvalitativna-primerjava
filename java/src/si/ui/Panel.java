package si.ui;

import si.tui.Screen;
import java.util.ArrayList;
import java.util.List;
import java.util.function.Supplier;

public final class Panel implements Widget {
    private final List<PanelItem> items = new ArrayList<>();

    public void addText(String text) {
        items.add(new TextItem(text));
    }

    public void addValue(String label, Supplier<String> stringify) {
        items.add(new ValueItem(label, stringify));
    }

    public void addEmptyLine() {
        items.add(new EmptyItem());
    }

    @Override
    public void draw(Screen screen, BBox bbox) {
        int y = bbox.y();
        int maxY = bbox.y() + bbox.h();

        for (PanelItem item : items) {
            if (y >= maxY) {
                break;
            }
            item.draw(screen, bbox.x(), y);
            y++;
        }
    }
}
