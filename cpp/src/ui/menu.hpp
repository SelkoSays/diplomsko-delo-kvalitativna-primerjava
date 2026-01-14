#pragma once

#include "../common.hpp"
#include "../tui/screen.hpp"

namespace ui {

struct MenuEntry {
    std::string name;
    bool selectable = true;
    std::function<void()> onSelected;

    // Separator entry
    static MenuEntry Separator() {
        return {"", false, nullptr};
    }
};

class Menu {
public:
    Menu() = default;

    void addEntry(MenuEntry entry);
    void addEntry(std::string name, bool selectable, std::function<void()> onSelected);
    void addSeparator();

    void moveUp();
    void moveDown();
    void select();

    void draw(tui::Screen& screen);

    int currentIndex() const { return m_at; }

private:
    std::vector<MenuEntry> m_entries;
    int m_at = 0;
    int m_x = 0, m_y = 0;  // 0 means centered
};

} // namespace ui
