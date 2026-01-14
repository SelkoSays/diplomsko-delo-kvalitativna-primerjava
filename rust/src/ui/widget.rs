//! Widget trait and common types

use crate::tui::screen::Screen;

/// Bounding box
#[derive(Clone, Copy, Debug)]
pub struct BBox {
    pub x: u32,
    pub y: u32,
    pub w: u32,
    pub h: u32,
}

/// Widget trait - objects that can draw themselves
pub trait Widget: Send + Sync {
    fn draw(&self, screen: &mut Screen, bbox: BBox);
}
