#pragma once

#include "widget.hpp"
#include <functional>

namespace ui {

// Abstract panel item
class PanelItem {
public:
    virtual ~PanelItem() = default;
    virtual void draw(tui::Screen& screen, int x, int y) = 0;
};

// Text label item
class TextItem : public PanelItem {
public:
    explicit TextItem(std::string text);
    void draw(tui::Screen& screen, int x, int y) override;

private:
    std::string m_text;
};

// Label + value display (calls stringify function to get value as string)
class ValueItem : public PanelItem {
public:
    ValueItem(std::string label, std::function<std::string()> stringify);
    void draw(tui::Screen& screen, int x, int y) override;

private:
    std::string m_label;
    std::function<std::string()> m_stringify;
};

// Empty line placeholder
class EmptyItem : public PanelItem {
public:
    void draw(tui::Screen& screen, int x, int y) override {}
};

// Vertical panel of items
class Panel : public Widget {
public:
    void addItem(std::unique_ptr<PanelItem> item);
    void addText(std::string text);
    void addValue(std::string label, std::function<std::string()> stringify);
    void addEmptyLine();

    void draw(tui::Screen& screen, BBox bbox) override;

private:
    std::vector<std::unique_ptr<PanelItem>> m_items;
};

} // namespace ui
