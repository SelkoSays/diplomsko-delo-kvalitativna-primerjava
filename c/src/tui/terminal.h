#ifndef TUI_TERMINAL_H
#define TUI_TERMINAL_H

#include <termios.h>

typedef struct Terminal {
    struct termios orig;  // original terminal settings
} Terminal;

// Get terminal size
void terminal_get_size(int *width, int *height);

// Enter raw mode (non-blocking input, no echo)
void terminal_enter_raw(Terminal *term);

// Exit raw mode (restore original settings)
void terminal_exit_raw(Terminal *term);

// Setup terminal for TUI (alt screen, hide cursor, enable mouse)
void terminal_setup(Terminal *term);

// Restore terminal to normal state
void terminal_restore(Terminal *term);

#endif // TUI_TERMINAL_H
