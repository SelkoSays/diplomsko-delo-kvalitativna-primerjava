#ifndef TUI_SCREEN_H
#define TUI_SCREEN_H

#include "cell.h"
#include "terminal.h"

typedef struct Screen {
    Cell *back;           // buffer we draw to
    Cell *front;          // buffer representing current screen state
    int width;
    int height;
    Terminal terminal;    // terminal state
} Screen;

// Create new screen (allocates buffers, enters raw mode)
Screen *screen_new(void);

// Free screen (restores terminal, frees buffers)
void screen_free(Screen *s);

// Handle terminal resize
void screen_resize(Screen *s);

// Clear back buffer
void screen_clear(Screen *s);

// Get cell at position (NULL if out of bounds)
Cell *screen_cell(Screen *s, int x, int y);

// Put single UTF-8 character at position
void screen_putc(Screen *s, int x, int y, const char *ch);

// Put string at position (handles newlines)
void screen_puts(Screen *s, int x, int y, const char *str);

// Set foreground color at position
void screen_set_fg_color(Screen *s, int x, int y, Color color);

// Set background color at position
void screen_set_bg_color(Screen *s, int x, int y, Color color);

// Set foreground using RGB values
void screen_set_fg(Screen *s, int x, int y, uint8_t r, uint8_t g, uint8_t b);

// Set background using RGB values
void screen_set_bg(Screen *s, int x, int y, uint8_t r, uint8_t g, uint8_t b);

// Set text attributes at position
void screen_set_attr(Screen *s, int x, int y, uint8_t attr);

// Put character with colors and attributes
void screen_put_color(Screen *s, int x, int y, const char *ch,
                      Color fg, Color bg, uint8_t attrs);

// Fill rectangular region with character
void screen_fill(Screen *s, int x, int y, int w, int h, const char *ch);

// Fill rectangular region with colors
void screen_fill_color(Screen *s, int x, int y, int w, int h,
                       Color fg, Color bg);

// Flush changes to terminal (differential update)
void screen_flush(Screen *s);

// Force full redraw (invalidates front buffer)
void screen_flush_full(Screen *s);

#endif // TUI_SCREEN_H
