package si.ui;

public final class MenuEntry {
    private final String name;
    private final boolean selectable;
    private final Runnable onSelected;

    public MenuEntry(String name, boolean selectable, Runnable onSelected) {
        this.name = name;
        this.selectable = selectable;
        this.onSelected = onSelected;
    }

    public static MenuEntry separator() {
        return new MenuEntry("", false, null);
    }

    public String name() {
        return name;
    }

    public boolean selectable() {
        return selectable;
    }

    public Runnable onSelected() {
        return onSelected;
    }
}
