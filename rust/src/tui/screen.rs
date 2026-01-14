//! Double-buffered screen

use super::cell::{Cell, Color};
use super::terminal::Terminal;
use std::io::{self, Write};

pub struct Screen {
    back: Vec<Cell>,
    front: Vec<Cell>,
    width: usize,
    height: usize,
    _terminal: Terminal,
}

impl Screen {
    pub fn new() -> io::Result<Self> {
        let terminal = Terminal::new()?;
        let (w, h) = Terminal::size()?;
        let (width, height) = (w as usize, h as usize);
        let size = width * height;

        Ok(Screen {
            back: vec![Cell::default(); size],
            front: vec![Cell::default(); size],
            width,
            height,
            _terminal: terminal,
        })
    }

    pub fn width(&self) -> usize {
        self.width
    }
    pub fn height(&self) -> usize {
        self.height
    }

    pub fn clear(&mut self) {
        for cell in &mut self.back {
            *cell = Cell::default();
        }
    }

    fn cell_mut(&mut self, x: usize, y: usize) -> Option<&mut Cell> {
        if x < self.width && y < self.height {
            Some(&mut self.back[y * self.width + x])
        } else {
            None
        }
    }

    pub fn put_char(&mut self, x: usize, y: usize, ch: &str) {
        if let Some(cell) = self.cell_mut(x, y) {
            cell.set_char(ch);
        }
    }

    pub fn put_str(&mut self, x: usize, y: usize, s: &str) {
        let mut cx = x;
        let mut cy = y;
        for ch in s.chars() {
            if cx >= self.width || cy >= self.height {
                break;
            }
            if ch == '\n' {
                cy += 1;
                cx = x;
                continue;
            }

            if let Some(cell) = self.cell_mut(cx, cy) {
                let mut buf = [0u8; 4];
                let encoded = ch.encode_utf8(&mut buf);
                cell.set_char(encoded);
            }
            cx += 1;
        }
    }

    pub fn set_fg_color(&mut self, x: usize, y: usize, color: Color) {
        if let Some(cell) = self.cell_mut(x, y) {
            cell.fg = color;
        }
    }

    #[allow(dead_code)]
    pub fn set_bg_color(&mut self, x: usize, y: usize, color: Color) {
        if let Some(cell) = self.cell_mut(x, y) {
            cell.bg = color;
        }
    }

    /// Differential flush - only write changed cells
    pub fn flush(&mut self) -> io::Result<()> {
        let mut buf = Vec::with_capacity(65536);
        let mut last_fg = Color(0xFFFFFFFF); // invalid initial
        let mut last_bg = Color(0xFFFFFFFF);
        let mut last_x: i32 = -2;
        let mut last_y: i32 = -2;

        for y in 0..self.height {
            for x in 0..self.width {
                let idx = y * self.width + x;
                let back = &self.back[idx];
                let front = &self.front[idx];

                if back == front {
                    last_x = -2;
                    continue;
                }

                if x as i32 != last_x + 1 || y as i32 != last_y {
                    write!(buf, "\x1b[{};{}H", y + 1, x + 1)?;
                }

                if back.fg != last_fg {
                    if back.fg.is_set() {
                        write!(
                            buf,
                            "\x1b[38;2;{};{};{}m",
                            back.fg.r(),
                            back.fg.g(),
                            back.fg.b()
                        )?;
                    } else {
                        buf.extend_from_slice(b"\x1b[39m");
                    }
                    last_fg = back.fg;
                }

                if back.bg != last_bg {
                    if back.bg.is_set() {
                        write!(
                            buf,
                            "\x1b[48;2;{};{};{}m",
                            back.bg.r(),
                            back.bg.g(),
                            back.bg.b()
                        )?;
                    } else {
                        buf.extend_from_slice(b"\x1b[49m");
                    }
                    last_bg = back.bg;
                }

                buf.extend_from_slice(back.ch_str().as_bytes());

                self.front[idx] = back.clone();

                last_x = x as i32;
                last_y = y as i32;
            }
        }

        io::stdout().write_all(&buf)?;
        io::stdout().flush()
    }
}
