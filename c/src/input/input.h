#ifndef INPUT_INPUT_H
#define INPUT_INPUT_H

#include <stdbool.h>
#include <stdint.h>

typedef enum {
    EVENT_NONE = 0,
    EVENT_KEY,
    EVENT_MOUSE,
    EVENT_RESIZE,
} EventType;

// Special key codes (values chosen to not conflict with ASCII)
typedef enum {
    KEY_NONE = 0,
    // ASCII printable range: 32-126
    // Control characters: 0-31 (Ctrl+A = 1, Ctrl+B = 2, etc.)

    // Special keys start at 256
    KEY_ESCAPE = 256,
    KEY_ENTER,
    KEY_TAB,
    KEY_BACKSPACE,
    KEY_DELETE,
    KEY_INSERT,

    KEY_UP,
    KEY_DOWN,
    KEY_LEFT,
    KEY_RIGHT,

    KEY_HOME,
    KEY_END,
    KEY_PAGE_UP,
    KEY_PAGE_DOWN,

    KEY_F1, KEY_F2, KEY_F3, KEY_F4,
    KEY_F5, KEY_F6, KEY_F7, KEY_F8,
    KEY_F9, KEY_F10, KEY_F11, KEY_F12,
} KeyCode;

// Modifier flags
#define MOD_NONE  0
#define MOD_SHIFT (1 << 0)
#define MOD_ALT   (1 << 1)
#define MOD_CTRL  (1 << 2)

// Mouse button codes
typedef enum {
    MOUSE_NONE = 0,
    MOUSE_LEFT,
    MOUSE_MIDDLE,
    MOUSE_RIGHT,
    MOUSE_SCROLL_UP,
    MOUSE_SCROLL_DOWN,
} MouseButton;

// Mouse event types
typedef enum {
    MOUSE_PRESS,
    MOUSE_RELEASE,
    MOUSE_MOVE,
    MOUSE_DRAG,
} MouseAction;

typedef struct {
    KeyCode key;      // key code (ASCII char or special key)
    uint8_t mods;     // modifier flags (MOD_SHIFT, MOD_ALT, MOD_CTRL)
    char ch[5];       // UTF-8 character if printable, else empty
} KeyEvent;

typedef struct {
    MouseButton button;
    MouseAction action;
    int x, y;         // 0-indexed position
    uint8_t mods;     // modifier flags
} MouseEvent;

typedef struct {
    EventType type;
    union {
        KeyEvent key;
        MouseEvent mouse;
    };
} Event;

typedef struct {
    char buf[64];
    int len;
    int pos;
} InputBuffer;

// Initialize input buffer
void input_buffer_init(InputBuffer *ib);

// Read available input into buffer
int input_read(InputBuffer *ib);

// Poll for next event (non-blocking)
// Returns true if event was parsed, false if no event available
bool input_poll(InputBuffer *ib, Event *ev);

// Check if key event matches (ignoring modifiers)
bool key_is(const Event *ev, KeyCode key);

// Check if key event matches with specific modifiers
bool key_is_mod(const Event *ev, KeyCode key, uint8_t mods);

// Check for character (ASCII letter/number)
bool key_is_char(const Event *ev, char c);

// Check for Ctrl+char
bool key_is_ctrl(const Event *ev, char c);

// Get key name for debugging
const char *key_name(KeyCode key);

#endif // INPUT_INPUT_H
