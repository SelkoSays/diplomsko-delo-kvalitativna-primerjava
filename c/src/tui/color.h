#ifndef TUI_COLOR_H
#define TUI_COLOR_H

#include <stdint.h>

// ================================
// Color Type (0xAARRGGBB format)
// A = 0: Transparent
// A != 0: Apply this color
// ================================

typedef uint32_t Color;

#define COLOR_NONE      0x00000000
#define COLOR_BLACK     0xFF000000
#define COLOR_WHITE     0xFFFFFFFF
#define COLOR_RED       0xFFFF0000
#define COLOR_GREEN     0xFF00FF00
#define COLOR_BLUE      0xFF0000FF
#define COLOR_YELLOW    0xFFFFFF00
#define COLOR_CYAN      0xFF00FFFF
#define COLOR_MAGENTA   0xFFFF00FF
#define COLOR_GRAY      0xFF808080

// Color construction
#define COLOR_RGB(r, g, b)     (0xFF000000 | ((uint32_t)(r) << 16) | ((uint32_t)(g) << 8) | (uint32_t)(b))
#define COLOR_RGBA(r, g, b, a) (((uint32_t)(a) << 24) | ((uint32_t)(r) << 16) | ((uint32_t)(g) << 8) | (uint32_t)(b))

// Color extraction
#define COLOR_A(c)  (((c) >> 24) & 0xFF)
#define COLOR_R(c)  (((c) >> 16) & 0xFF)
#define COLOR_G(c)  (((c) >> 8) & 0xFF)
#define COLOR_B(c)  ((c) & 0xFF)

// Check if color should be applied
#define COLOR_IS_SET(c)  (((c) >> 24) != 0)

#define ESC_ALT_SCREEN_ON   "\x1b[?1049h"
#define ESC_ALT_SCREEN_OFF  "\x1b[?1049l"
#define ESC_CURSOR_HIDE     "\x1b[?25l"
#define ESC_CURSOR_SHOW     "\x1b[?25h"
#define ESC_MOUSE_ON        "\x1b[?1003h\x1b[?1006h"
#define ESC_MOUSE_OFF       "\x1b[?1003l\x1b[?1006l"
#define ESC_CLEAR_SCREEN    "\x1b[2J"
#define ESC_RESET_ATTRS     "\x1b[0m"
#define ESC_HOME            "\x1b[H"

#endif // TUI_COLOR_H
