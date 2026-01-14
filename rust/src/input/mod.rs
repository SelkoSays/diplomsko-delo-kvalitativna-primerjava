//! Input event types and ANSI escape sequence parser

use std::cell::RefCell;

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum KeyCode {
    Char(char),
    Enter,
    Tab,
    Backspace,
    Escape,
    Up,
    Down,
    Left,
    Right,
    Home,
    End,
    PageUp,
    PageDown,
    Insert,
    Delete,
    F(u8),
}

#[derive(Debug, Clone, Copy, PartialEq, Eq, Default)]
pub struct KeyModifiers(u8);

impl KeyModifiers {
    pub const NONE: Self = Self(0);
    pub const SHIFT: Self = Self(1 << 0);
    pub const ALT: Self = Self(1 << 1);
    pub const CONTROL: Self = Self(1 << 2);

    pub fn contains(self, other: Self) -> bool {
        (self.0 & other.0) == other.0
    }
}

impl std::ops::BitOr for KeyModifiers {
    type Output = Self;
    fn bitor(self, rhs: Self) -> Self {
        Self(self.0 | rhs.0)
    }
}

impl std::ops::BitOrAssign for KeyModifiers {
    fn bitor_assign(&mut self, rhs: Self) {
        self.0 |= rhs.0;
    }
}

#[derive(Debug, Clone)]
pub struct KeyEvent {
    pub code: KeyCode,
    pub modifiers: KeyModifiers,
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum MouseButton {
    None,
    Left,
    Middle,
    Right,
    ScrollUp,
    ScrollDown,
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum MouseAction {
    Press,
    Release,
    Move,
    Drag,
}

#[allow(unused)]
#[derive(Debug, Clone)]
pub struct MouseEvent {
    pub button: MouseButton,
    pub action: MouseAction,
    pub x: i32,
    pub y: i32,
    pub modifiers: KeyModifiers,
}

#[allow(unused)]
#[derive(Debug, Clone)]
pub enum Event {
    Key(KeyEvent),
    Mouse(MouseEvent),
}

struct InputBuffer {
    buf: [u8; 64],
    len: usize,
    pos: usize,
}

impl InputBuffer {
    fn new() -> Self {
        Self {
            buf: [0; 64],
            len: 0,
            pos: 0,
        }
    }

    fn read(&mut self) -> usize {
        // Shift remaining data to start
        if self.pos > 0 && self.pos < self.len {
            self.buf.copy_within(self.pos..self.len, 0);
            self.len -= self.pos;
            self.pos = 0;
        } else if self.pos >= self.len {
            self.len = 0;
            self.pos = 0;
        }

        // Read more data (non-blocking)
        let space = self.buf.len() - self.len;
        if space > 0 {
            let n = unsafe {
                libc::read(
                    libc::STDIN_FILENO,
                    self.buf[self.len..].as_mut_ptr() as *mut libc::c_void,
                    space,
                )
            };
            if n > 0 {
                self.len += n as usize;
            }
        }

        self.len - self.pos
    }

    fn remaining(&self) -> usize {
        self.len - self.pos
    }

    fn peek(&self, offset: usize) -> u8 {
        let idx = self.pos + offset;
        if idx >= self.len {
            0
        } else {
            self.buf[idx]
        }
    }

    fn match_seq(&self, seq: &[u8]) -> bool {
        if self.pos + seq.len() > self.len {
            return false;
        }
        &self.buf[self.pos..self.pos + seq.len()] == seq
    }

    fn consume(&mut self, n: usize) {
        self.pos += n;
    }
}

fn parse_mouse_sgr(ib: &mut InputBuffer) -> Option<Event> {
    if !ib.match_seq(b"\x1b[<") {
        return None;
    }

    let mut end = None;
    for i in 3..ib.remaining() {
        let c = ib.peek(i);
        if c == b'M' || c == b'm' {
            end = Some(i);
            break;
        }
        if c < b'0' || (c > b'9' && c != b';') {
            return None;
        }
    }
    let end = end?;

    let mut params = [0i32; 3];
    let mut param_idx = 0;
    let mut current = 0i32;

    for i in 3..end {
        let c = ib.peek(i);
        if c >= b'0' && c <= b'9' {
            current = current * 10 + (c - b'0') as i32;
        } else if c == b';' {
            if param_idx < 3 {
                params[param_idx] = current;
                param_idx += 1;
            }
            current = 0;
        }
    }
    if param_idx < 3 {
        params[param_idx] = current;
    }

    let btn = params[0];
    let x = params[1] - 1;
    let y = params[2] - 1;
    let released = ib.peek(end) == b'm';

    let mut mods = KeyModifiers::NONE;
    if btn & 4 != 0 {
        mods |= KeyModifiers::SHIFT;
    }
    if btn & 8 != 0 {
        mods |= KeyModifiers::ALT;
    }
    if btn & 16 != 0 {
        mods |= KeyModifiers::CONTROL;
    }

    let base_btn = btn & 3;
    let motion = (btn & 32) != 0;
    let scroll = (btn & 64) != 0;

    let (button, action) = if scroll {
        let btn = if base_btn == 0 {
            MouseButton::ScrollUp
        } else {
            MouseButton::ScrollDown
        };
        (btn, MouseAction::Press)
    } else if motion {
        let btn = if base_btn == 3 {
            MouseButton::None
        } else {
            match base_btn {
                0 => MouseButton::Left,
                1 => MouseButton::Middle,
                2 => MouseButton::Right,
                _ => MouseButton::None,
            }
        };
        let act = if base_btn == 3 {
            MouseAction::Move
        } else {
            MouseAction::Drag
        };
        (btn, act)
    } else {
        let btn = if base_btn == 3 {
            MouseButton::None
        } else {
            match base_btn {
                0 => MouseButton::Left,
                1 => MouseButton::Middle,
                2 => MouseButton::Right,
                _ => MouseButton::None,
            }
        };
        let act = if released {
            MouseAction::Release
        } else {
            MouseAction::Press
        };
        (btn, act)
    };

    ib.consume(end + 1);

    Some(Event::Mouse(MouseEvent {
        button,
        action,
        x,
        y,
        modifiers: mods,
    }))
}

fn parse_csi(ib: &mut InputBuffer) -> Option<Event> {
    if !ib.match_seq(b"\x1b[") {
        return None;
    }

    if ib.peek(2) == b'<' {
        return parse_mouse_sgr(ib);
    }

    let mut params = [0u32; 4];
    let mut param_count = 0;
    let mut i = 2;
    let mut current_param = 0u32;

    while i < ib.remaining() {
        let c = ib.peek(i);

        if c >= b'0' && c <= b'9' {
            current_param = current_param * 10 + (c - b'0') as u32;
            i += 1;
        } else if c == b';' {
            if param_count < 4 {
                params[param_count] = current_param;
                param_count += 1;
            }
            current_param = 0;
            i += 1;
        } else if c >= 0x40 && c <= 0x7E {
            if param_count < 4 {
                params[param_count] = current_param;
                param_count += 1;
            }

            let mut mods = KeyModifiers::NONE;

            if param_count >= 2 && params[1] > 1 {
                let m = params[1] - 1;
                if m & 1 != 0 {
                    mods |= KeyModifiers::SHIFT;
                }
                if m & 2 != 0 {
                    mods |= KeyModifiers::ALT;
                }
                if m & 4 != 0 {
                    mods |= KeyModifiers::CONTROL;
                }
            }

            let code = match c {
                b'A' => Some(KeyCode::Up),
                b'B' => Some(KeyCode::Down),
                b'C' => Some(KeyCode::Right),
                b'D' => Some(KeyCode::Left),
                b'H' => Some(KeyCode::Home),
                b'F' => Some(KeyCode::End),
                b'~' => match params[0] {
                    1 => Some(KeyCode::Home),
                    2 => Some(KeyCode::Insert),
                    3 => Some(KeyCode::Delete),
                    4 => Some(KeyCode::End),
                    5 => Some(KeyCode::PageUp),
                    6 => Some(KeyCode::PageDown),
                    11 => Some(KeyCode::F(1)),
                    12 => Some(KeyCode::F(2)),
                    13 => Some(KeyCode::F(3)),
                    14 => Some(KeyCode::F(4)),
                    15 => Some(KeyCode::F(5)),
                    17 => Some(KeyCode::F(6)),
                    18 => Some(KeyCode::F(7)),
                    19 => Some(KeyCode::F(8)),
                    20 => Some(KeyCode::F(9)),
                    21 => Some(KeyCode::F(10)),
                    23 => Some(KeyCode::F(11)),
                    24 => Some(KeyCode::F(12)),
                    _ => None,
                },
                _ => None,
            };

            ib.consume(i + 1);

            return code.map(|code| {
                Event::Key(KeyEvent {
                    code,
                    modifiers: mods,
                })
            });
        } else {
            break;
        }
    }

    None
}

fn parse_ss3(ib: &mut InputBuffer) -> Option<Event> {
    if !ib.match_seq(b"\x1bO") {
        return None;
    }
    if ib.remaining() < 3 {
        return None;
    }

    let c = ib.peek(2);

    let code = match c {
        b'P' => Some(KeyCode::F(1)),
        b'Q' => Some(KeyCode::F(2)),
        b'R' => Some(KeyCode::F(3)),
        b'S' => Some(KeyCode::F(4)),
        b'A' => Some(KeyCode::Up),
        b'B' => Some(KeyCode::Down),
        b'C' => Some(KeyCode::Right),
        b'D' => Some(KeyCode::Left),
        b'H' => Some(KeyCode::Home),
        b'F' => Some(KeyCode::End),
        _ => return None,
    };

    ib.consume(3);

    code.map(|code| {
        Event::Key(KeyEvent {
            code,
            modifiers: KeyModifiers::NONE,
        })
    })
}

fn parse_alt(ib: &mut InputBuffer) -> Option<Event> {
    if ib.peek(0) != 0x1b {
        return None;
    }
    if ib.remaining() < 2 {
        return None;
    }

    let c = ib.peek(1);

    if c == b'[' || c == b'O' {
        return None;
    }

    let mut mods = KeyModifiers::ALT;
    let code;

    if c >= 1 && c <= 26 {
        code = KeyCode::Char((b'a' + c - 1) as char);
        mods |= KeyModifiers::CONTROL;
    } else if c >= 32 && c <= 126 {
        code = KeyCode::Char(c as char);
    } else {
        return None;
    }

    ib.consume(2);

    Some(Event::Key(KeyEvent {
        code,
        modifiers: mods,
    }))
}

fn parse_escape(ib: &mut InputBuffer) -> Option<Event> {
    if ib.peek(0) != 0x1b {
        return None;
    }

    if ib.remaining() == 1 {
        ib.consume(1);
        return Some(Event::Key(KeyEvent {
            code: KeyCode::Escape,
            modifiers: KeyModifiers::NONE,
        }));
    }

    None
}

fn parse_char(ib: &mut InputBuffer) -> Option<Event> {
    let c = ib.peek(0);
    if c == 0 {
        return None;
    }

    match c {
        0 => return None,
        9 => {
            ib.consume(1);
            return Some(Event::Key(KeyEvent {
                code: KeyCode::Tab,
                modifiers: KeyModifiers::NONE,
            }));
        }
        10 | 13 => {
            ib.consume(1);
            return Some(Event::Key(KeyEvent {
                code: KeyCode::Enter,
                modifiers: KeyModifiers::NONE,
            }));
        }
        27 => return None,
        127 => {
            ib.consume(1);
            return Some(Event::Key(KeyEvent {
                code: KeyCode::Backspace,
                modifiers: KeyModifiers::NONE,
            }));
        }
        _ => {}
    }

    if c >= 1 && c <= 26 {
        ib.consume(1);
        return Some(Event::Key(KeyEvent {
            code: KeyCode::Char((b'a' + c - 1) as char),
            modifiers: KeyModifiers::CONTROL,
        }));
    }

    if c >= 32 && c <= 126 {
        ib.consume(1);
        return Some(Event::Key(KeyEvent {
            code: KeyCode::Char(c as char),
            modifiers: KeyModifiers::NONE,
        }));
    }

    if c >= 0x80 {
        let len = if (c & 0xE0) == 0xC0 {
            2
        } else if (c & 0xF0) == 0xE0 {
            3
        } else if (c & 0xF8) == 0xF0 {
            4
        } else {
            1
        };

        if ib.remaining() < len {
            return None;
        }

        let bytes = &ib.buf[ib.pos..ib.pos + len];
        if let Ok(s) = std::str::from_utf8(bytes) {
            if let Some(ch) = s.chars().next() {
                ib.consume(len);
                return Some(Event::Key(KeyEvent {
                    code: KeyCode::Char(ch),
                    modifiers: KeyModifiers::NONE,
                }));
            }
        }

        ib.consume(1);
        return None;
    }

    None
}

thread_local! {
    static INPUT_BUFFER: RefCell<InputBuffer> = RefCell::new(InputBuffer::new());
}

/// Poll for events (non-blocking)
pub fn poll() -> Option<Event> {
    INPUT_BUFFER.with(|ib| {
        let mut ib = ib.borrow_mut();

        ib.read();

        if ib.remaining() == 0 {
            return None;
        }

        if let Some(ev) = parse_csi(&mut ib) {
            return Some(ev);
        }
        if let Some(ev) = parse_ss3(&mut ib) {
            return Some(ev);
        }
        if let Some(ev) = parse_alt(&mut ib) {
            return Some(ev);
        }
        if let Some(ev) = parse_escape(&mut ib) {
            return Some(ev);
        }
        if let Some(ev) = parse_char(&mut ib) {
            return Some(ev);
        }

        ib.consume(1);
        None
    })
}

/// Check if key event matches a character (case-insensitive, no modifiers)
pub fn is_char(key: &KeyEvent, c: char) -> bool {
    matches!(key.code, KeyCode::Char(ch) if ch.to_ascii_lowercase() == c.to_ascii_lowercase())
        && !key.modifiers.contains(KeyModifiers::CONTROL)
        && !key.modifiers.contains(KeyModifiers::ALT)
}

/// Check if key event matches a key code
pub fn is_key(key: &KeyEvent, code: KeyCode) -> bool {
    key.code == code
}

/// Check if key event is Ctrl+char
#[allow(dead_code)]
pub fn is_ctrl(key: &KeyEvent, c: char) -> bool {
    matches!(key.code, KeyCode::Char(ch) if ch.to_ascii_lowercase() == c.to_ascii_lowercase())
        && key.modifiers.contains(KeyModifiers::CONTROL)
}
