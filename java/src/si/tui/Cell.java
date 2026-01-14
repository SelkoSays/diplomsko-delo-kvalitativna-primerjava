package si.tui;

public final class Cell {
    private String ch = " ";
    private Color fg = Color.NONE;
    private Color bg = Color.NONE;
    private int attrs = Attr.NONE;

    public Cell() {
    }

    public void setChar(String s) {
        if (s == null || s.isEmpty()) {
            ch = " ";
            return;
        }
        int codePoint = s.codePointAt(0);
        ch = new String(Character.toChars(codePoint));
    }

    public String getChar() {
        return ch;
    }

    public void clear() {
        ch = " ";
        fg = Color.NONE;
        bg = Color.NONE;
        attrs = Attr.NONE;
    }

    public void copyFrom(Cell other) {
        this.ch = other.ch;
        this.fg = other.fg;
        this.bg = other.bg;
        this.attrs = other.attrs;
    }

    public boolean equalsCell(Cell other) {
        return this.fg.equals(other.fg)
                && this.bg.equals(other.bg)
                && this.attrs == other.attrs
                && this.ch.equals(other.ch);
    }

    public Color fg() {
        return fg;
    }

    public void setFg(Color c) {
        fg = c;
    }

    public Color bg() {
        return bg;
    }

    public void setBg(Color c) {
        bg = c;
    }

    public int attrs() {
        return attrs;
    }

    public void setAttrs(int a) {
        attrs = a;
    }
}
