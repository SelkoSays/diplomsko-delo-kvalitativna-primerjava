package si.input;

import java.io.IOException;
import java.util.Optional;

public final class InputHandler {
    private final byte[] buf = new byte[64];
    private int len = 0;
    private int pos = 0;

    public InputHandler() {
    }

    public Optional<Event> poll() {
        read();

        if (remaining() == 0) {
            return Optional.empty();
        }

        Event ev;

        if ((ev = parseCsi()) != null) {
            return Optional.of(ev);
        }
        if ((ev = parseSs3()) != null) {
            return Optional.of(ev);
        }
        if ((ev = parseAlt()) != null) {
            return Optional.of(ev);
        }
        if ((ev = parseEscape()) != null) {
            return Optional.of(ev);
        }
        if ((ev = parseChar()) != null) {
            return Optional.of(ev);
        }

        consume(1);
        return Optional.empty();
    }

    private void read() {
        if (pos > 0 && pos < len) {
            System.arraycopy(buf, pos, buf, 0, len - pos);
            len -= pos;
            pos = 0;
        } else if (pos >= len) {
            len = 0;
            pos = 0;
        }

        try {
            int available = System.in.available();
            if (available > 0) {
                int space = buf.length - len;
                int toRead = Math.min(available, space);
                int n = System.in.read(buf, len, toRead);
                if (n > 0) {
                    len += n;
                }
            }
        } catch (IOException e) {
            // Ignore
        }
    }

    private int remaining() {
        return len - pos;
    }

    private byte peek(int offset) {
        int idx = pos + offset;
        return (idx >= 0 && idx < len) ? buf[idx] : 0;
    }

    private void consume(int n) {
        pos += n;
    }

    private boolean match(byte[] seq) {
        if (pos + seq.length > len) {
            return false;
        }
        for (int i = 0; i < seq.length; i++) {
            if (buf[pos + i] != seq[i]) {
                return false;
            }
        }
        return true;
    }

    private Event parseMouseSgr() {
        if (!match(new byte[]{0x1b, '[', '<'})) {
            return null;
        }

        int end = -1;
        for (int i = 3; i < remaining(); i++) {
            byte c = peek(i);
            if (c == 'M' || c == 'm') {
                end = i;
                break;
            }
            if (c < '0' || (c > '9' && c != ';')) {
                return null;
            }
        }
        if (end < 0) {
            return null;
        }

        StringBuilder sb = new StringBuilder();
        for (int i = 3; i < end; i++) {
            sb.append((char) peek(i));
        }
        String[] parts = sb.toString().split(";");
        if (parts.length != 3) {
            return null;
        }

        int btn, x, y;
        try {
            btn = Integer.parseInt(parts[0]);
            x = Integer.parseInt(parts[1]) - 1;
            y = Integer.parseInt(parts[2]) - 1;
        } catch (NumberFormatException e) {
            return null;
        }

        boolean released = (peek(end) == 'm');

        int mods = Modifier.NONE;
        if ((btn & 4) != 0) {
            mods |= Modifier.SHIFT;
        }
        if ((btn & 8) != 0) {
            mods |= Modifier.ALT;
        }
        if ((btn & 16) != 0) {
            mods |= Modifier.CTRL;
        }

        int baseBtn = btn & 3;
        boolean motion = (btn & 32) != 0;
        boolean scroll = (btn & 64) != 0;

        MouseButton button;
        MouseAction action;

        if (scroll) {
            button = (baseBtn == 0) ? MouseButton.SCROLL_UP : MouseButton.SCROLL_DOWN;
            action = MouseAction.PRESS;
        } else if (motion) {
            button = (baseBtn == 3) ? MouseButton.NONE : MouseButton.values()[baseBtn + 1];
            action = (baseBtn == 3) ? MouseAction.MOVE : MouseAction.DRAG;
        } else {
            button = (baseBtn == 3) ? MouseButton.NONE : MouseButton.values()[baseBtn + 1];
            action = released ? MouseAction.RELEASE : MouseAction.PRESS;
        }

        consume(end + 1);
        return new MouseEvent(button, action, x, y, mods);
    }

    private Event parseCsi() {
        if (!match(new byte[]{0x1b, '['})) {
            return null;
        }

        if (peek(2) == '<') {
            return parseMouseSgr();
        }

        int[] params = new int[4];
        int paramCount = 0;
        int i = 2;
        int currentParam = 0;

        while (i < remaining()) {
            byte c = peek(i);

            if (c >= '0' && c <= '9') {
                currentParam = currentParam * 10 + (c - '0');
                i++;
            } else if (c == ';') {
                if (paramCount < 4) {
                    params[paramCount++] = currentParam;
                }
                currentParam = 0;
                i++;
            } else if (c >= 0x40 && c <= 0x7E) {
                if (paramCount < 4) {
                    params[paramCount++] = currentParam;
                }

                int mods = Modifier.NONE;
                if (paramCount >= 2 && params[1] > 1) {
                    int mod = params[1] - 1;
                    if ((mod & Modifier.SHIFT) != 0) {
                        mods |= Modifier.SHIFT;
                    }
                    if ((mod & Modifier.ALT) != 0) {
                        mods |= Modifier.ALT;
                    }
                    if ((mod & Modifier.CTRL) != 0) {
                        mods |= Modifier.CTRL;
                    }
                }

                KeyCode key = KeyCode.NONE;
                switch (c) {
                    case 'A':
                        key = KeyCode.UP;
                        break;
                    case 'B':
                        key = KeyCode.DOWN;
                        break;
                    case 'C':
                        key = KeyCode.RIGHT;
                        break;
                    case 'D':
                        key = KeyCode.LEFT;
                        break;
                    case 'H':
                        key = KeyCode.HOME;
                        break;
                    case 'F':
                        key = KeyCode.END;
                        break;
                    case '~':
                        switch (params[0]) {
                            case 1:
                                key = KeyCode.HOME;
                                break;
                            case 2:
                                key = KeyCode.INSERT;
                                break;
                            case 3:
                                key = KeyCode.DELETE;
                                break;
                            case 4:
                                key = KeyCode.END;
                                break;
                            case 5:
                                key = KeyCode.PAGE_UP;
                                break;
                            case 6:
                                key = KeyCode.PAGE_DOWN;
                                break;
                            case 11:
                                key = KeyCode.F1;
                                break;
                            case 12:
                                key = KeyCode.F2;
                                break;
                            case 13:
                                key = KeyCode.F3;
                                break;
                            case 14:
                                key = KeyCode.F4;
                                break;
                            case 15:
                                key = KeyCode.F5;
                                break;
                            case 17:
                                key = KeyCode.F6;
                                break;
                            case 18:
                                key = KeyCode.F7;
                                break;
                            case 19:
                                key = KeyCode.F8;
                                break;
                            case 20:
                                key = KeyCode.F9;
                                break;
                            case 21:
                                key = KeyCode.F10;
                                break;
                            case 23:
                                key = KeyCode.F11;
                                break;
                            case 24:
                                key = KeyCode.F12;
                                break;
                            default:
                                key = KeyCode.NONE;
                                break;
                        }
                        break;
                    default:
                        key = KeyCode.NONE;
                        break;
                }

                consume(i + 1);
                if (key != KeyCode.NONE) {
                    return new KeyEvent(key, 0, mods, "");
                }
                return null;
            } else {
                break;
            }
        }

        return null;
    }

    private Event parseSs3() {
        if (!match(new byte[]{0x1b, 'O'})) {
            return null;
        }
        if (remaining() < 3) {
            return null;
        }

        byte c = peek(2);
        KeyCode key;

        switch (c) {
            case 'P':
                key = KeyCode.F1;
                break;
            case 'Q':
                key = KeyCode.F2;
                break;
            case 'R':
                key = KeyCode.F3;
                break;
            case 'S':
                key = KeyCode.F4;
                break;
            case 'A':
                key = KeyCode.UP;
                break;
            case 'B':
                key = KeyCode.DOWN;
                break;
            case 'C':
                key = KeyCode.RIGHT;
                break;
            case 'D':
                key = KeyCode.LEFT;
                break;
            case 'H':
                key = KeyCode.HOME;
                break;
            case 'F':
                key = KeyCode.END;
                break;
            default:
                return null;
        }

        consume(3);
        return new KeyEvent(key, 0, Modifier.NONE, "");
    }

    private Event parseAlt() {
        if (peek(0) != 0x1b) {
            return null;
        }
        if (remaining() < 2) {
            return null;
        }

        byte c = peek(1);

        if (c == '[' || c == 'O') {
            return null;
        }

        int mods = Modifier.ALT;
        int charValue;
        String ch;

        if (c >= 1 && c <= 26) {
            mods |= Modifier.CTRL;
            charValue = 'a' + c - 1;
            ch = "";
        } else if (c >= 32 && c <= 126) {
            charValue = c;
            ch = String.valueOf((char) c);
        } else {
            return null;
        }

        consume(2);
        return new KeyEvent(KeyCode.NONE, charValue, mods, ch);
    }

    private Event parseEscape() {
        if (peek(0) != 0x1b) {
            return null;
        }

        if (remaining() == 1) {
            consume(1);
            return new KeyEvent(KeyCode.ESCAPE, 0, Modifier.NONE, "");
        }

        return null;
    }

    private Event parseChar() {
        byte c = peek(0);
        if (c == 0) {
            return null;
        }

        switch (c) {
            case 9:
                consume(1);
                return new KeyEvent(KeyCode.TAB, 0, Modifier.NONE, "");
            case 10:
            case 13:
                consume(1);
                return new KeyEvent(KeyCode.ENTER, 0, Modifier.NONE, "");
            case 27:
                return null;
            case 127:
                consume(1);
                return new KeyEvent(KeyCode.BACKSPACE, 0, Modifier.NONE, "");
        }

        if (c >= 1 && c <= 26) {
            consume(1);
            return new KeyEvent(KeyCode.NONE, 'a' + c - 1, Modifier.CTRL, "");
        }

        if (c >= 32 && c <= 126) {
            consume(1);
            return new KeyEvent(KeyCode.NONE, c, Modifier.NONE, String.valueOf((char) c));
        }

        if ((c & 0xFF) >= 0x80) {
            int cLen = 1;
            int first = c & 0xFF;
            if ((first & 0xE0) == 0xC0) {
                cLen = 2;
            } else if ((first & 0xF0) == 0xE0) {
                cLen = 3;
            } else if ((first & 0xF8) == 0xF0) {
                cLen = 4;
            }

            if (remaining() < cLen) {
                return null;
            }

            byte[] utf8 = new byte[cLen];
            for (int i = 0; i < cLen; i++) {
                utf8[i] = peek(i);
            }
            String ch = new String(utf8, java.nio.charset.StandardCharsets.UTF_8);
            consume(cLen);
            return new KeyEvent(KeyCode.NONE, 0, Modifier.NONE, ch);
        }

        return null;
    }
}
