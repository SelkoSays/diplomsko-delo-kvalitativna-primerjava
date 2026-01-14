//! Cell representation for terminal buffer

/// Color in 0xAARRGGBB format (A=0 means default/no color)
#[derive(Clone, Copy, PartialEq, Eq, Default)]
pub struct Color(pub u32);

#[allow(unused)]
impl Color {
    pub const NONE: Color = Color(0x00000000);
    pub const BLACK: Color = Color(0xFF000000);
    pub const WHITE: Color = Color(0xFFFFFFFF);
    pub const RED: Color = Color(0xFFFF0000);
    pub const GREEN: Color = Color(0xFF00FF00);
    pub const BLUE: Color = Color(0xFF0000FF);
    pub const YELLOW: Color = Color(0xFFFFFF00);
    pub const CYAN: Color = Color(0xFF00FFFF);
    pub const MAGENTA: Color = Color(0xFFFF00FF);
    pub const GRAY: Color = Color(0xFF808080);

    pub fn rgb(r: u8, g: u8, b: u8) -> Self {
        Color(0xFF000000 | ((r as u32) << 16) | ((g as u32) << 8) | (b as u32))
    }

    pub fn is_set(self) -> bool {
        (self.0 >> 24) != 0
    }

    pub fn r(self) -> u8 {
        ((self.0 >> 16) & 0xFF) as u8
    }
    pub fn g(self) -> u8 {
        ((self.0 >> 8) & 0xFF) as u8
    }
    pub fn b(self) -> u8 {
        (self.0 & 0xFF) as u8
    }
}

/// Text attributes (bitflags)
#[derive(Clone, Copy, PartialEq, Eq, Default)]
pub struct Attributes(pub u8);

#[allow(unused)]
impl Attributes {
    pub const NONE: Attributes = Attributes(0);
    pub const BOLD: Attributes = Attributes(1 << 0);
    pub const DIM: Attributes = Attributes(1 << 1);
    pub const ITALIC: Attributes = Attributes(1 << 2);
    pub const UNDERLINE: Attributes = Attributes(1 << 3);
    pub const BLINK: Attributes = Attributes(1 << 4);
    pub const REVERSE: Attributes = Attributes(1 << 5);
    pub const CROSSED: Attributes = Attributes(1 << 6);

    pub fn contains(self, other: Attributes) -> bool {
        (self.0 & other.0) == other.0
    }
}

/// A single terminal cell
#[derive(Clone, PartialEq, Eq)]
pub struct Cell {
    /// UTF-8 character (up to 4 bytes)
    pub ch: [u8; 4],
    /// Foreground color
    pub fg: Color,
    /// Background color
    pub bg: Color,
    /// Text attributes
    pub attrs: Attributes,
}

impl Default for Cell {
    fn default() -> Self {
        let mut ch = [0u8; 4];
        ch[0] = b' ';
        Cell {
            ch,
            fg: Color::NONE,
            bg: Color::NONE,
            attrs: Attributes::NONE,
        }
    }
}

impl Cell {
    pub fn ch_str(&self) -> &str {
        let len = self.ch.iter().position(|&b| b == 0).unwrap_or(5);
        std::str::from_utf8(&self.ch[..len]).unwrap_or(" ")
    }

    pub fn set_char(&mut self, ch: &str) {
        self.ch = [0; 4];
        let bytes = ch.as_bytes();
        let len = bytes.len().min(4);
        self.ch[..len].copy_from_slice(&bytes[..len]);
    }
}
