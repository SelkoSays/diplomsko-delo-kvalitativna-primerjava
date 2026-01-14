//! Stats panel with dynamic values using closures

use crate::tui::screen::Screen;
use crate::ui::widget::{BBox, Widget};

/// Panel item types
pub enum PanelItem {
    Empty,
    Text(String),
    Value {
        label: String,
        get_value: Box<dyn Fn() -> String + Send + Sync>,
    },
}

/// Stats panel widget
pub struct Panel {
    items: Vec<PanelItem>,
}

#[allow(unused)]
impl Panel {
    pub fn new() -> Self {
        Panel { items: Vec::new() }
    }

    pub fn add_empty_line(&mut self) {
        self.items.push(PanelItem::Empty);
    }

    pub fn add_text(&mut self, text: &str) {
        self.items.push(PanelItem::Text(text.to_string()));
    }

    pub fn add_value<F>(&mut self, label: &str, get_value: F)
    where
        F: Fn() -> String + Send + Sync + 'static,
    {
        self.items.push(PanelItem::Value {
            label: label.to_string(),
            get_value: Box::new(get_value),
        });
    }
}

impl Default for Panel {
    fn default() -> Self {
        Self::new()
    }
}

impl Widget for Panel {
    fn draw(&self, screen: &mut Screen, bbox: BBox) {
        for (i, item) in self.items.iter().enumerate() {
            if i as u32 >= bbox.h {
                break;
            }

            let x = bbox.x as usize;
            let y = (bbox.y + i as u32) as usize;

            match item {
                PanelItem::Empty => {}
                PanelItem::Text(text) => {
                    screen.put_str(x, y, text);
                }
                PanelItem::Value { label, get_value } => {
                    let value = get_value();
                    let text = format!("{}: {}", label, value);
                    screen.put_str(x, y, &text);
                }
            }
        }
    }
}
