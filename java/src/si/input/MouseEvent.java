package si.input;

public record MouseEvent(MouseButton button, MouseAction action, int x, int y, int mods) implements Event {
}
