package si.ui;

import si.tui.Screen;

public interface Widget {
    void draw(Screen screen, BBox bbox);
}
