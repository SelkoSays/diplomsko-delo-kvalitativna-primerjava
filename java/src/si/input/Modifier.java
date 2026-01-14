package si.input;

public final class Modifier {
    public static final int NONE = 0;
    public static final int SHIFT = 1 << 0;
    public static final int ALT = 1 << 1;
    public static final int CTRL = 1 << 2;

    private Modifier() {
    }
}
