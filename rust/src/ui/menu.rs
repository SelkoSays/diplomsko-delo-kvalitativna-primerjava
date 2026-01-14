//! Menu widget

use crate::tui::{cell::Color, screen::Screen};

/// Menu entry
pub struct Entry {
    pub name: Option<String>,
    pub selectable: bool,
    pub callback: Option<Box<dyn FnMut() + Send>>,
}

/// Menu widget
pub struct Menu {
    entries: Vec<Entry>,
    selected: i32,
}

impl Menu {
    pub fn new() -> Self {
        Menu {
            entries: Vec::new(),
            selected: -1,
        }
    }

    pub fn add_entry<F>(&mut self, name: &str, selectable: bool, callback: Option<F>)
    where
        F: FnMut() + Send + 'static,
    {
        self.entries.push(Entry {
            name: Some(name.to_string()),
            selectable,
            callback: callback.map(|f| Box::new(f) as Box<dyn FnMut() + Send>),
        });

        // Auto-select first selectable entry
        if selectable && self.selected < 0 {
            self.selected = (self.entries.len() - 1) as i32;
        }
    }

    pub fn add_separator(&mut self) {
        self.entries.push(Entry {
            name: None,
            selectable: false,
            callback: None,
        });
    }

    pub fn move_up(&mut self) {
        if self.selected < 0 {
            // Find first selectable
            for (i, entry) in self.entries.iter().enumerate() {
                if entry.selectable {
                    self.selected = i as i32;
                    break;
                }
            }
        } else if self.selected > 0 {
            for i in (0..self.selected as usize).rev() {
                if self.entries[i].selectable {
                    self.selected = i as i32;
                    break;
                }
            }
        }
    }

    pub fn move_down(&mut self) {
        let count = self.entries.len() as i32;
        if self.selected < count - 1 {
            for i in (self.selected as usize + 1)..self.entries.len() {
                if self.entries[i].selectable {
                    self.selected = i as i32;
                    break;
                }
            }
        }
    }

    pub fn select(&mut self) {
        if self.selected >= 0 && (self.selected as usize) < self.entries.len() {
            if let Some(ref mut callback) = self.entries[self.selected as usize].callback {
                callback();
            }
        }
    }

    pub fn draw_on(&self, screen: &mut Screen) {
        // Calculate max width
        let max_len = self
            .entries
            .iter()
            .filter_map(|e| e.name.as_ref())
            .map(|n| n.len() + 2)
            .max()
            .unwrap_or(10);

        // Calculate position (centered)
        let start_x = (screen.width().saturating_sub(max_len)) / 2;
        let start_y = (screen.height().saturating_sub(self.entries.len())) / 2;

        // Draw entries
        for (i, entry) in self.entries.iter().enumerate() {
            let y = start_y + i;

            if let Some(ref name) = entry.name {
                screen.put_str(start_x, y, name);

                // Set color
                let fg = if entry.selectable {
                    Color::WHITE
                } else {
                    Color::GRAY
                };
                for j in 0..name.len() {
                    screen.set_fg_color(start_x + j, y, fg);
                }

                // Draw selection indicator
                if i as i32 == self.selected {
                    let indicator_x = start_x + name.len() + 1;
                    screen.put_str(indicator_x, y, "<");
                    screen.set_fg_color(indicator_x, y, Color::YELLOW);
                }
            }
        }
    }
}

impl Default for Menu {
    fn default() -> Self {
        Self::new()
    }
}
