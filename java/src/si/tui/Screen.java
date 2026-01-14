package si.tui;

public final class Screen {
    private final Terminal terminal;
    private final int width;
    private final int height;
    private final Cell[] back;
    private final Cell[] front;

    public Screen() {
        this.terminal = new Terminal();
        this.width = terminal.width();
        this.height = terminal.height();

        int size = width * height;
        back = new Cell[size];
        front = new Cell[size];
        for (int i = 0; i < size; i++) {
            back[i] = new Cell();
            front[i] = new Cell();
        }
    }

    public int width() {
        return width;
    }

    public int height() {
        return height;
    }

    public void close() {
        terminal.close();
    }

    private Cell cell(int x, int y) {
        if (x < 0 || x >= width || y < 0 || y >= height) {
            return null;
        }
        return back[y * width + x];
    }

    public void clear() {
        for (Cell c : back) {
            c.clear();
        }
    }

    public void putChar(int x, int y, String ch) {
        Cell c = cell(x, y);
        if (c != null) {
            c.setChar(ch);
        }
    }

    public void putString(int x, int y, String str) {
        int cx = x;
        int cy = y;
        for (int i = 0; i < str.length();) {
            int codePoint = str.codePointAt(i);

            if (codePoint == '\n') {
                cy++;
                cx = x;
                i++;
                continue;
            }

            if (codePoint < 0x20 && codePoint != '\t') {
                i += Character.charCount(codePoint);
                continue;
            }

            if (cx < width && cy < height) {
                putChar(cx, cy, new String(Character.toChars(codePoint)));
            }

            cx++;
            i += Character.charCount(codePoint);
        }
    }

    public void setFgColor(int x, int y, Color color) {
        Cell c = cell(x, y);
        if (c != null) {
            c.setFg(color);
        }
    }

    public void setBgColor(int x, int y, Color color) {
        Cell c = cell(x, y);
        if (c != null) {
            c.setBg(color);
        }
    }

    public void setAttr(int x, int y, int attrs) {
        Cell c = cell(x, y);
        if (c != null) {
            c.setAttrs(attrs);
        }
    }

    public void fill(int x, int y, int w, int h, String ch) {
        for (int cy = y; cy < y + h && cy < height; cy++) {
            for (int cx = x; cx < x + w && cx < width; cx++) {
                putChar(cx, cy, ch);
            }
        }
    }

    public void fillColor(int x, int y, int w, int h, Color fg, Color bg) {
        for (int cy = y; cy < y + h && cy < height; cy++) {
            for (int cx = x; cx < x + w && cx < width; cx++) {
                Cell c = cell(cx, cy);
                if (c != null) {
                    if (fg.isSet()) {
                        c.setFg(fg);
                    }
                    if (bg.isSet()) {
                        c.setBg(bg);
                    }
                }
            }
        }
    }

    public void flush() {
        StringBuilder buf = new StringBuilder(65536);

        int lastX = -2;
        int lastY = -2;
        Color lastFg = new Color(0xFFFFFFFF);
        Color lastBg = new Color(0xFFFFFFFF);
        int lastAttrs = 0xFF;
        boolean firstCell = true;

        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                Cell backCell = back[y * width + x];
                Cell frontCell = front[y * width + x];

                if (backCell.equalsCell(frontCell)) {
                    lastX = -2;
                    continue;
                }

                if (x != lastX + 1 || y != lastY) {
                    buf.append(Escape.moveTo(y, x));
                }

                if (firstCell || backCell.attrs() != lastAttrs) {
                    buf.append("\u001b[0");
                    int attrs = backCell.attrs();
                    if ((attrs & Attr.BOLD) != 0) {
                        buf.append(";1");
                    }
                    if ((attrs & Attr.DIM) != 0) {
                        buf.append(";2");
                    }
                    if ((attrs & Attr.ITALIC) != 0) {
                        buf.append(";3");
                    }
                    if ((attrs & Attr.UNDERLINE) != 0) {
                        buf.append(";4");
                    }
                    if ((attrs & Attr.BLINK) != 0) {
                        buf.append(";5");
                    }
                    if ((attrs & Attr.REVERSE) != 0) {
                        buf.append(";7");
                    }
                    if ((attrs & Attr.CROSSED) != 0) {
                        buf.append(";9");
                    }
                    buf.append("m");
                    lastAttrs = attrs;
                    lastFg = new Color(0xFFFFFFFF);
                    lastBg = new Color(0xFFFFFFFF);
                }

                if (backCell.fg().isSet()) {
                    if (firstCell || !backCell.fg().equals(lastFg)) {
                        buf.append(Escape.fgColor(backCell.fg().r(), backCell.fg().g(), backCell.fg().b()));
                        lastFg = backCell.fg();
                    }
                } else if (firstCell || lastFg.isSet()) {
                    buf.append(Escape.RESET_FG);
                    lastFg = Color.NONE;
                }

                if (backCell.bg().isSet()) {
                    if (firstCell || !backCell.bg().equals(lastBg)) {
                        buf.append(Escape.bgColor(backCell.bg().r(), backCell.bg().g(), backCell.bg().b()));
                        lastBg = backCell.bg();
                    }
                } else if (firstCell || lastBg.isSet()) {
                    buf.append(Escape.RESET_BG);
                    lastBg = Color.NONE;
                }

                buf.append(backCell.getChar());

                frontCell.copyFrom(backCell);

                lastX = x;
                lastY = y;
                firstCell = false;
            }
        }

        System.out.print(buf.toString());
        System.out.flush();
    }

    public void flushFull() {
        for (Cell c : front) {
            c.setChar("\0");
        }
        System.out.print(Escape.CLEAR_SCREEN);
        flush();
    }
}
