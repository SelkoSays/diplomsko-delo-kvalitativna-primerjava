#include "menu.hpp"

namespace ui {

void Menu::addEntry(MenuEntry entry) {
    m_entries.push_back(std::move(entry));
}

void Menu::addEntry(std::string name, bool selectable, std::function<void()> onSelected) {
    m_entries.push_back({std::move(name), selectable, std::move(onSelected)});
}

void Menu::addSeparator() {
    m_entries.push_back(MenuEntry::Separator());
}

void Menu::moveUp() {
    if (m_entries.empty()) return;

    if (m_at < 0) {
        for (size_t i = 0; i < m_entries.size(); i++) {
            if (m_entries[i].selectable) {
                m_at = static_cast<int>(i);
                break;
            }
        }
    } else if (m_at > 0) {
        for (int i = m_at - 1; i >= 0; i--) {
            if (m_entries[i].selectable) {
                m_at = i;
                break;
            }
        }
    }
}

void Menu::moveDown() {
    if (m_entries.empty()) return;

    int count = static_cast<int>(m_entries.size());

    if (m_at < count - 1) {
        for (int i = m_at + 1; i < count; i++) {
            if (m_entries[i].selectable) {
                m_at = i;
                break;
            }
        }
    } else {
        for (int i = count - 1; i >= 0; i--) {
            if (m_entries[i].selectable) {
                m_at = i;
                break;
            }
        }
    }
}

void Menu::select() {
    if (m_entries.empty()) return;

    if (m_at < 0) {
        m_at = 0;
    } else if (m_at >= static_cast<int>(m_entries.size())) {
        m_at = static_cast<int>(m_entries.size()) - 1;
    }

    if (m_entries[m_at].onSelected) {
        m_entries[m_at].onSelected();
    }
}

void Menu::draw(tui::Screen& screen) {
    if (m_entries.empty()) return;

    int maxLen = 0;
    for (size_t i = 0; i < m_entries.size(); i++) {
        if (!m_entries[i].name.empty()) {
            int len = static_cast<int>(m_entries[i].name.size()) + 2;
            if (len > maxLen) maxLen = len;
        }
    }

    int startX = m_x;
    int startY = m_y;
    if (startX == 0) {
        startX = (screen.width() - maxLen) / 2;
    }
    if (startY == 0) {
        startY = (screen.height() - static_cast<int>(m_entries.size())) / 2;
    }

    for (size_t i = 0; i < m_entries.size(); i++) {
        int y = startY + static_cast<int>(i);

        if (m_entries[i].name.empty()) {
            continue;
        }

        screen.putString(startX, y, m_entries[i].name);

        auto fg = m_entries[i].selectable ? tui::Color::White() : tui::Color::Gray();
        int len = static_cast<int>(m_entries[i].name.size());
        for (int j = 0; j < len; j++) {
            screen.setFgColor(startX + j, y, fg);
        }

        if (static_cast<int>(i) == m_at) {
            screen.putChar(startX + len + 1, y, "<");
            screen.setFgColor(startX + len + 1, y, tui::Color::Yellow());
        }
    }
}

} // namespace ui
