#include "input.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>

void input_buffer_init(InputBuffer *ib) {
    ib->len = 0;
    ib->pos = 0;
}

int input_read(InputBuffer *ib) {
    // Shift remaining data to start
    if (ib->pos > 0 && ib->pos < ib->len) {
        memmove(ib->buf, ib->buf + ib->pos, ib->len - ib->pos);
        ib->len -= ib->pos;
        ib->pos = 0;
    } else if (ib->pos >= ib->len) {
        ib->len = 0;
        ib->pos = 0;
    }

    // Read more data
    int space = sizeof(ib->buf) - ib->len;
    if (space > 0) {
        int n = read(STDIN_FILENO, ib->buf + ib->len, space);
        if (n > 0) ib->len += n;
    }

    return ib->len - ib->pos;
}

static bool input_match(InputBuffer *ib, const char *seq, int len) {
    if (ib->pos + len > ib->len) return false;
    return memcmp(ib->buf + ib->pos, seq, len) == 0;
}

static void input_consume(InputBuffer *ib, int n) {
    ib->pos += n;
}

static int input_remaining(InputBuffer *ib) {
    return ib->len - ib->pos;
}

static char input_peek(InputBuffer *ib, int offset) {
    int idx = ib->pos + offset;
    if (idx < 0 || idx >= ib->len) return 0;
    return ib->buf[idx];
}

// Parse mouse event in SGR format: \x1b[<btn;x;yM or \x1b[<btn;x;ym
static bool parse_mouse_sgr(InputBuffer *ib, Event *ev) {
    if (!input_match(ib, "\x1b[<", 3)) return false;

    // Find the terminator (M or m)
    int end = -1;
    for (int i = 3; i < input_remaining(ib); i++) {
        char c = input_peek(ib, i);
        if (c == 'M' || c == 'm') {
            end = i;
            break;
        }
        if (c < '0' || (c > '9' && c != ';')) return false;
    }
    if (end < 0) return false;

    // Parse: btn;x;y
    char tmp[32];
    int len = end - 3;
    if (len >= (int)sizeof(tmp)) return false;
    memcpy(tmp, ib->buf + ib->pos + 3, len);
    tmp[len] = '\0';

    int btn = 0, x = 0, y = 0;
    if (sscanf(tmp, "%d;%d;%d", &btn, &x, &y) != 3) return false;

    bool released = (input_peek(ib, end) == 'm');

    ev->type = EVENT_MOUSE;
    ev->mouse.x = x - 1;
    ev->mouse.y = y - 1;
    ev->mouse.mods = MOD_NONE;

    // Decode modifiers from button code
    if (btn & 4)  ev->mouse.mods |= MOD_SHIFT;
    if (btn & 8)  ev->mouse.mods |= MOD_ALT;
    if (btn & 16) ev->mouse.mods |= MOD_CTRL;

    // Decode button and action
    int base_btn = btn & 3;
    bool motion = (btn & 32) != 0;
    bool scroll = (btn & 64) != 0;

    if (scroll) {
        ev->mouse.button = (base_btn == 0) ? MOUSE_SCROLL_UP : MOUSE_SCROLL_DOWN;
        ev->mouse.action = MOUSE_PRESS;
    } else if (motion) {
        ev->mouse.button = (base_btn == 3) ? MOUSE_NONE : (MouseButton)(base_btn + 1);
        ev->mouse.action = (base_btn == 3) ? MOUSE_MOVE : MOUSE_DRAG;
    } else {
        ev->mouse.button = (base_btn == 3) ? MOUSE_NONE : (MouseButton)(base_btn + 1);
        ev->mouse.action = released ? MOUSE_RELEASE : MOUSE_PRESS;
    }

    input_consume(ib, end + 1);
    return true;
}

// Parse CSI sequence (escape [ ...)
static bool parse_csi(InputBuffer *ib, Event *ev) {
    if (!input_match(ib, "\x1b[", 2)) return false;

    if (input_peek(ib, 2) == '<') {
        return parse_mouse_sgr(ib, ev);
    }

    int params[4] = {0};
    int param_count = 0;
    int i = 2;
    int current_param = 0;

    while (i < input_remaining(ib)) {
        char c = input_peek(ib, i);

        if (c >= '0' && c <= '9') {
            current_param = current_param * 10 + (c - '0');
            i++;
        } else if (c == ';') {
            if (param_count < 4) params[param_count++] = current_param;
            current_param = 0;
            i++;
        } else if (c >= 0x40 && c <= 0x7E) {
            // Final byte found
            if (param_count < 4) params[param_count++] = current_param;

            ev->type = EVENT_KEY;
            ev->key.mods = MOD_NONE;
            ev->key.ch[0] = '\0';

            // Decode modifiers from param
            if (param_count >= 2 && params[1] > 1) {
                int mod = params[1] - 1;
                if (mod & MOD_SHIFT) ev->key.mods |= MOD_SHIFT;
                if (mod & MOD_ALT) ev->key.mods |= MOD_ALT;
                if (mod & MOD_CTRL) ev->key.mods |= MOD_CTRL;
            }

            // Decode key
            switch (c) {
                case 'A': ev->key.key = KEY_UP; break;
                case 'B': ev->key.key = KEY_DOWN; break;
                case 'C': ev->key.key = KEY_RIGHT; break;
                case 'D': ev->key.key = KEY_LEFT; break;
                case 'H': ev->key.key = KEY_HOME; break;
                case 'F': ev->key.key = KEY_END; break;
                case '~':
                    switch (params[0]) {
                        case 1:  ev->key.key = KEY_HOME; break;
                        case 2:  ev->key.key = KEY_INSERT; break;
                        case 3:  ev->key.key = KEY_DELETE; break;
                        case 4:  ev->key.key = KEY_END; break;
                        case 5:  ev->key.key = KEY_PAGE_UP; break;
                        case 6:  ev->key.key = KEY_PAGE_DOWN; break;
                        case 11: ev->key.key = KEY_F1; break;
                        case 12: ev->key.key = KEY_F2; break;
                        case 13: ev->key.key = KEY_F3; break;
                        case 14: ev->key.key = KEY_F4; break;
                        case 15: ev->key.key = KEY_F5; break;
                        case 17: ev->key.key = KEY_F6; break;
                        case 18: ev->key.key = KEY_F7; break;
                        case 19: ev->key.key = KEY_F8; break;
                        case 20: ev->key.key = KEY_F9; break;
                        case 21: ev->key.key = KEY_F10; break;
                        case 23: ev->key.key = KEY_F11; break;
                        case 24: ev->key.key = KEY_F12; break;
                        default: ev->key.key = KEY_NONE; break;
                    }
                    break;
                default:
                    ev->key.key = KEY_NONE;
                    break;
            }

            input_consume(ib, i + 1);
            return ev->key.key != KEY_NONE;
        } else {
            break;
        }
    }

    return false;
}

// Parse SS3 sequence (escape O ...) - alternate function key format
static bool parse_ss3(InputBuffer *ib, Event *ev) {
    if (!input_match(ib, "\x1bO", 2)) return false;
    if (input_remaining(ib) < 3) return false;

    char c = input_peek(ib, 2);

    ev->type = EVENT_KEY;
    ev->key.mods = MOD_NONE;
    ev->key.ch[0] = '\0';

    switch (c) {
        case 'P': ev->key.key = KEY_F1; break;
        case 'Q': ev->key.key = KEY_F2; break;
        case 'R': ev->key.key = KEY_F3; break;
        case 'S': ev->key.key = KEY_F4; break;
        case 'A': ev->key.key = KEY_UP; break;
        case 'B': ev->key.key = KEY_DOWN; break;
        case 'C': ev->key.key = KEY_RIGHT; break;
        case 'D': ev->key.key = KEY_LEFT; break;
        case 'H': ev->key.key = KEY_HOME; break;
        case 'F': ev->key.key = KEY_END; break;
        default:  return false;
    }

    input_consume(ib, 3);
    return true;
}

// Parse Alt+key (escape followed by key)
static bool parse_alt(InputBuffer *ib, Event *ev) {
    if (input_peek(ib, 0) != '\x1b') return false;
    if (input_remaining(ib) < 2) return false;

    char c = input_peek(ib, 1);

    // Don't consume if it might be a CSI or SS3 sequence
    if (c == '[' || c == 'O') return false;

    ev->type = EVENT_KEY;
    ev->key.mods = MOD_ALT;

    if (c >= 1 && c <= 26) {
        ev->key.key = (KeyCode)('a' + c - 1);
        ev->key.mods |= MOD_CTRL;
        ev->key.ch[0] = '\0';
    } else if (c >= 32 && c <= 126) {
        ev->key.key = (KeyCode)c;
        ev->key.ch[0] = c;
        ev->key.ch[1] = '\0';
    } else {
        return false;
    }

    input_consume(ib, 2);
    return true;
}

// Parse standalone escape
static bool parse_escape(InputBuffer *ib, Event *ev) {
    if (input_peek(ib, 0) != '\x1b') return false;

    if (input_remaining(ib) == 1) {
        ev->type = EVENT_KEY;
        ev->key.key = KEY_ESCAPE;
        ev->key.mods = MOD_NONE;
        ev->key.ch[0] = '\0';
        input_consume(ib, 1);
        return true;
    }

    return false;
}

// Parse regular character or control character
static bool parse_char(InputBuffer *ib, Event *ev) {
    char c = input_peek(ib, 0);
    if (c == 0) return false;

    ev->type = EVENT_KEY;
    ev->key.mods = MOD_NONE;

    // Handle special control chars
    switch (c) {
        case 0:    return false;
        case 9:    ev->key.key = KEY_TAB; ev->key.ch[0] = '\0'; input_consume(ib, 1); return true;
        case 10:
        case 13:   ev->key.key = KEY_ENTER; ev->key.ch[0] = '\0'; input_consume(ib, 1); return true;
        case 27:   return false;
        case 127:  ev->key.key = KEY_BACKSPACE; ev->key.ch[0] = '\0'; input_consume(ib, 1); return true;
        default:   break;
    }

    // Control characters (Ctrl+A through Ctrl+Z)
    if (c >= 1 && c <= 26) {
        ev->key.key = (KeyCode)('a' + c - 1);
        ev->key.mods = MOD_CTRL;
        ev->key.ch[0] = '\0';
        input_consume(ib, 1);
        return true;
    }

    // Printable ASCII
    if (c >= 32 && c <= 126) {
        ev->key.key = (KeyCode)c;
        ev->key.ch[0] = c;
        ev->key.ch[1] = '\0';
        input_consume(ib, 1);
        return true;
    }

    // UTF-8
    if ((unsigned char)c >= 0x80) {
        int len = 1;
        unsigned char first = (unsigned char)c;
        if ((first & 0xE0) == 0xC0) len = 2;
        else if ((first & 0xF0) == 0xE0) len = 3;
        else if ((first & 0xF8) == 0xF0) len = 4;

        if (input_remaining(ib) < len) return false;

        ev->key.key = KEY_NONE;
        memcpy(ev->key.ch, ib->buf + ib->pos, len);
        ev->key.ch[len] = '\0';
        input_consume(ib, len);
        return true;
    }

    return false;
}

bool input_poll(InputBuffer *ib, Event *ev) {
    ev->type = EVENT_NONE;

    input_read(ib);

    if (input_remaining(ib) == 0) return false;

    if (parse_csi(ib, ev)) return true;
    if (parse_ss3(ib, ev)) return true;
    if (parse_alt(ib, ev)) return true;
    if (parse_escape(ib, ev)) return true;
    if (parse_char(ib, ev)) return true;

    // Unknown sequence - skip one byte
    input_consume(ib, 1);
    return false;
}

bool key_is(const Event *ev, KeyCode key) {
    return ev->type == EVENT_KEY && ev->key.key == key;
}

bool key_is_mod(const Event *ev, KeyCode key, uint8_t mods) {
    return ev->type == EVENT_KEY && ev->key.key == key && ev->key.mods == mods;
}

bool key_is_char(const Event *ev, char c) {
    if (ev->type != EVENT_KEY || ev->key.mods != MOD_NONE) return false;
    int key = (int)ev->key.key;
    int lower_c = (c >= 'A' && c <= 'Z') ? c + 32 : c;
    int lower_k = (key >= 'A' && key <= 'Z') ? key + 32 : key;
    return lower_k == lower_c;
}

bool key_is_ctrl(const Event *ev, char c) {
    char lower = (c >= 'A' && c <= 'Z') ? c + 32 : c;
    return ev->type == EVENT_KEY && ev->key.key == (KeyCode)lower && (ev->key.mods & MOD_CTRL);
}

const char *key_name(KeyCode key) {
    switch (key) {
        case KEY_NONE: return "NONE";
        case KEY_ESCAPE: return "ESC";
        case KEY_ENTER: return "ENTER";
        case KEY_TAB: return "TAB";
        case KEY_BACKSPACE: return "BACKSPACE";
        case KEY_DELETE: return "DELETE";
        case KEY_INSERT: return "INSERT";
        case KEY_UP: return "UP";
        case KEY_DOWN: return "DOWN";
        case KEY_LEFT: return "LEFT";
        case KEY_RIGHT: return "RIGHT";
        case KEY_HOME: return "HOME";
        case KEY_END: return "END";
        case KEY_PAGE_UP: return "PGUP";
        case KEY_PAGE_DOWN: return "PGDN";
        case KEY_F1: return "F1";
        case KEY_F2: return "F2";
        case KEY_F3: return "F3";
        case KEY_F4: return "F4";
        case KEY_F5: return "F5";
        case KEY_F6: return "F6";
        case KEY_F7: return "F7";
        case KEY_F8: return "F8";
        case KEY_F9: return "F9";
        case KEY_F10: return "F10";
        case KEY_F11: return "F11";
        case KEY_F12: return "F12";
        default:
            if (key >= 32 && key <= 126) {
                static char buf[2];
                buf[0] = (char)key;
                buf[1] = '\0';
                return buf;
            }
            return "?";
    }
}
