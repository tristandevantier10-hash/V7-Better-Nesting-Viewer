#pragma once

#include "UIElement.h"
#include <vector>
#include <string>
#include <functional>

class Renderer;

class ListView : public UIElement
{
public:

    ListView();

    void update(const SDL_Event& e) override;

    void render(Renderer& renderer) override;

    void addItem(const std::string& text);

    void updateItem(
        int index,
        const std::string& text);

    void removeItem(int index);

    void clear();

    int getSelectedIndex() const;

    const std::string& getSelectedItem() const;

    void setSelectedIndex(int index);

    void setSelectionChangedCallback(
        std::function<void(int)> callback);

    void setItem(
        int index,
        const std::string& text);

private:

    std::vector<std::string> items;

    int selectedIndex = -1;

    int rowHeight = 28;

    std::function<void(int)> onSelectionChanged;
};
