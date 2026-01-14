#include "terminal.h"
#include "color.h"
#include <stdio.h>
#include <sys/ioctl.h>
#include <unistd.h>

void terminal_get_size(int *width, int *height) {
    struct winsize ws;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws);
    *width = ws.ws_col;
    *height = ws.ws_row;
}

void terminal_enter_raw(Terminal *term) {
    struct termios raw;
    tcgetattr(STDIN_FILENO, &term->orig);
    raw = term->orig;
    raw.c_lflag &= ~(ECHO | ICANON | ISIG);  // no echo, canonical mode, or signals
    raw.c_iflag &= ~(IXON | ICRNL);          // no flow control, no CR->NL
    raw.c_cc[VMIN] = 0;                       // non-blocking read
    raw.c_cc[VTIME] = 0;
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

void terminal_exit_raw(Terminal *term) {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &term->orig);
}

void terminal_setup(Terminal *term) {
    terminal_enter_raw(term);
    printf(ESC_ALT_SCREEN_ON ESC_CURSOR_HIDE ESC_MOUSE_ON ESC_CLEAR_SCREEN);
    fflush(stdout);
}

void terminal_restore(Terminal *term) {
    printf(ESC_RESET_ATTRS ESC_MOUSE_OFF ESC_CURSOR_SHOW ESC_ALT_SCREEN_OFF);
    fflush(stdout);
    terminal_exit_raw(term);
}
