//! Hierarchical frame layout

use crate::tui::{cell::Color, screen::Screen};
use crate::ui::widget::{BBox, Widget};

/// Frame split direction
#[derive(Clone, Copy, Debug)]
pub enum FrameSplit {
    Vertical,
    Horizontal,
}

/// Box-drawing characters
const FRAME_CHARS: [&str; 10] = [
    "\u{256D}", // UP_LEFT ╭
    "\u{256E}", // UP_RIGHT ╮
    "\u{2570}", // DOWN_LEFT ╰
    "\u{256F}", // DOWN_RIGHT ╯
    "\u{2500}", // HDASH ─
    "\u{2502}", // VDASH │
    "\u{252C}", // FORK_DOWN ┬
    "\u{2524}", // FORK_LEFT ┤
    "\u{2534}", // FORK_UP ┴
    "\u{251C}", // FORK_RIGHT ├
];

const UP_LEFT: usize = 0;
const UP_RIGHT: usize = 1;
const DOWN_LEFT: usize = 2;
const DOWN_RIGHT: usize = 3;
const HDASH: usize = 4;
const VDASH: usize = 5;
const FORK_DOWN: usize = 6;
const FORK_LEFT: usize = 7;
const FORK_UP: usize = 8;
const FORK_RIGHT: usize = 9;

const FRAME_BORDER_COLOR: Color = Color::GRAY;

/// Frame node in the layout tree
pub struct Frame {
    x: u32,
    y: u32,
    w: u32,
    h: u32,
    children: Option<(Box<Frame>, Box<Frame>)>,
    widgets: Vec<Box<dyn Widget>>,
}

impl Frame {
    pub fn new(w: u32, h: u32, x: u32, y: u32) -> Self {
        Frame {
            x,
            y,
            w,
            h,
            children: None,
            widgets: Vec::new(),
        }
    }

    #[allow(dead_code)]
    pub fn width(&self) -> u32 {
        self.w
    }
    #[allow(dead_code)]
    pub fn height(&self) -> u32 {
        self.h
    }

    /// Split frame, returns references to the two child frames
    pub fn split(&mut self, coord: u32, direction: FrameSplit) -> (&mut Frame, &mut Frame) {
        let (child1, child2) = match direction {
            FrameSplit::Vertical => {
                let child1 = Frame::new(coord, self.h, self.x, self.y);
                let child2 = Frame::new(self.w - coord + 1, self.h, self.x + coord - 1, self.y);
                (child1, child2)
            }
            FrameSplit::Horizontal => {
                let child1 = Frame::new(self.w, coord, self.x, self.y);
                let child2 = Frame::new(self.w, self.h - coord + 1, self.x, self.y + coord - 1);
                (child1, child2)
            }
        };

        self.children = Some((Box::new(child1), Box::new(child2)));

        let children = self.children.as_mut().unwrap();
        (&mut children.0, &mut children.1)
    }

    pub fn add_widget(&mut self, widget: Box<dyn Widget>) {
        self.widgets.push(widget);
    }

    pub fn draw(&self, screen: &mut Screen) {
        self.draw_stage(screen, 1); // borders
        self.draw_stage(screen, 2); // joints
        self.draw_stage(screen, 3); // widgets
    }

    fn draw_stage(&self, screen: &mut Screen, stage: u8) {
        // Recursively draw children
        if let Some((ref child1, ref child2)) = self.children {
            child1.draw_stage(screen, stage);
            child2.draw_stage(screen, stage);
        }

        match stage {
            1 => self.draw_borders(screen),
            2 => self.draw_joints(screen),
            3 => self.draw_widgets(screen),
            _ => {}
        }
    }

    fn draw_borders(&self, screen: &mut Screen) {
        let (x, y, w, h) = (
            self.x as usize,
            self.y as usize,
            self.w as usize,
            self.h as usize,
        );

        if w == 0 || h == 0 {
            return;
        }

        // Top-left corner
        screen.put_char(x, y, FRAME_CHARS[UP_LEFT]);
        screen.set_fg_color(x, y, FRAME_BORDER_COLOR);

        // Top edge
        for i in 1..(w.saturating_sub(1)) {
            screen.put_char(x + i, y, FRAME_CHARS[HDASH]);
            screen.set_fg_color(x + i, y, FRAME_BORDER_COLOR);
        }

        // Top-right corner
        if w > 1 {
            screen.put_char(x + w - 1, y, FRAME_CHARS[UP_RIGHT]);
            screen.set_fg_color(x + w - 1, y, FRAME_BORDER_COLOR);
        }

        // Side edges
        for i in 1..(h.saturating_sub(1)) {
            screen.put_char(x, y + i, FRAME_CHARS[VDASH]);
            screen.set_fg_color(x, y + i, FRAME_BORDER_COLOR);
            if w > 1 {
                screen.put_char(x + w - 1, y + i, FRAME_CHARS[VDASH]);
                screen.set_fg_color(x + w - 1, y + i, FRAME_BORDER_COLOR);
            }
        }

        // Bottom-left corner
        if h > 1 {
            screen.put_char(x, y + h - 1, FRAME_CHARS[DOWN_LEFT]);
            screen.set_fg_color(x, y + h - 1, FRAME_BORDER_COLOR);
        }

        // Bottom edge
        for i in 1..(w.saturating_sub(1)) {
            if h > 1 {
                screen.put_char(x + i, y + h - 1, FRAME_CHARS[HDASH]);
                screen.set_fg_color(x + i, y + h - 1, FRAME_BORDER_COLOR);
            }
        }

        // Bottom-right corner
        if w > 1 && h > 1 {
            screen.put_char(x + w - 1, y + h - 1, FRAME_CHARS[DOWN_RIGHT]);
            screen.set_fg_color(x + w - 1, y + h - 1, FRAME_BORDER_COLOR);
        }
    }

    fn draw_joints(&self, screen: &mut Screen) {
        if let Some((_, ref child2)) = self.children {
            let (x, y, w, h) = (
                self.x as usize,
                self.y as usize,
                self.w as usize,
                self.h as usize,
            );
            let c2x = child2.x as usize;
            let c2y = child2.y as usize;

            if y == c2y {
                // Vertical split
                screen.put_char(c2x, y, FRAME_CHARS[FORK_DOWN]);
                screen.set_fg_color(c2x, y, FRAME_BORDER_COLOR);
                if h > 1 {
                    screen.put_char(c2x, y + h - 1, FRAME_CHARS[FORK_UP]);
                    screen.set_fg_color(c2x, y + h - 1, FRAME_BORDER_COLOR);
                }
            } else {
                // Horizontal split
                screen.put_char(x, c2y, FRAME_CHARS[FORK_RIGHT]);
                screen.set_fg_color(x, c2y, FRAME_BORDER_COLOR);
                if w > 1 {
                    screen.put_char(x + w - 1, c2y, FRAME_CHARS[FORK_LEFT]);
                    screen.set_fg_color(x + w - 1, c2y, FRAME_BORDER_COLOR);
                }
            }
        }
    }

    fn draw_widgets(&self, screen: &mut Screen) {
        let bbox = BBox {
            x: self.x + 1,
            y: self.y + 1,
            w: self.w.saturating_sub(2),
            h: self.h.saturating_sub(2),
        };

        for widget in &self.widgets {
            widget.draw(screen, bbox);
        }
    }
}
