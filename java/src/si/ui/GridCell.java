package si.ui;

import si.tui.Screen;
import java.util.function.BiConsumer;

public final class GridCell {
    private BiConsumer<Screen, int[]> drawFn;

    public void setDrawCallback(BiConsumer<Screen, int[]> fn) {
        this.drawFn = fn;
    }

    public void clear() {
        drawFn = null;
    }

    public void draw(Screen screen, int x, int y) {
        if (drawFn != null) {
            drawFn.accept(screen, new int[]{x, y});
        }
    }

    public boolean hasCallback() {
        return drawFn != null;
    }
}
