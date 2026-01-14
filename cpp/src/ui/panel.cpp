#include "panel.hpp"
#include <cstdio>

namespace ui {

TextItem::TextItem(std::string text) : m_text(std::move(text)) {}

void TextItem::draw(tui::Screen& screen, int x, int y) {
    screen.putString(x, y, m_text);
}

ValueItem::ValueItem(std::string label, std::function<std::string()> stringify)
    : m_label(std::move(label)), m_stringify(std::move(stringify)) {}

void ValueItem::draw(tui::Screen& screen, int x, int y) {
    std::string display = m_label + ": " + m_stringify();
    screen.putString(x, y, display);
}

void Panel::addItem(std::unique_ptr<PanelItem> item) {
    m_items.push_back(std::move(item));
}

void Panel::addText(std::string text) {
    m_items.push_back(std::make_unique<TextItem>(std::move(text)));
}

void Panel::addValue(std::string label, std::function<std::string()> stringify) {
    m_items.push_back(std::make_unique<ValueItem>(std::move(label), std::move(stringify)));
}

void Panel::addEmptyLine() {
    m_items.push_back(std::make_unique<EmptyItem>());
}

void Panel::draw(tui::Screen& screen, BBox bbox) {
    for (size_t i = 0; i < m_items.size() && i < bbox.h; i++) {
        if (m_items[i]) {
            m_items[i]->draw(screen, bbox.x, bbox.y + static_cast<int>(i));
        }
    }
}

} // namespace ui
