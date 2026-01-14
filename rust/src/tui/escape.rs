//! ANSI escape sequence constants

pub const ALT_SCREEN_ON: &str = "\x1b[?1049h";
pub const ALT_SCREEN_OFF: &str = "\x1b[?1049l";
pub const CURSOR_HIDE: &str = "\x1b[?25l";
pub const CURSOR_SHOW: &str = "\x1b[?25h";
pub const MOUSE_ON: &str = "\x1b[?1003h\x1b[?1006h";
pub const MOUSE_OFF: &str = "\x1b[?1003l\x1b[?1006l";
pub const CLEAR_SCREEN: &str = "\x1b[2J";
pub const RESET_ATTRS: &str = "\x1b[0m";
