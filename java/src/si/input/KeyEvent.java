package si.input;

public record KeyEvent(KeyCode key, int charValue, int mods, String ch) implements Event {

    public boolean isChar(char c) {
        return (charValue == c || charValue == Character.toLowerCase(c) || charValue == Character.toUpperCase(c))
                && mods == Modifier.NONE;
    }

    public boolean isKey(KeyCode k) {
        return key == k;
    }

    public boolean isCtrl(char c) {
        char lower = Character.toLowerCase(c);
        return charValue == lower && (mods & Modifier.CTRL) != 0;
    }
}
