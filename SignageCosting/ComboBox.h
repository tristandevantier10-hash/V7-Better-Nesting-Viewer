#pragma once

#include "UIElement.h"
#include <string>
#include <vector>
#include <functional>

class Renderer;

class ComboBox : public UIElement
{
public:

    ComboBox();

    void update(const SDL_Event& e) override;

    void render(Renderer& renderer) override;

    void addItem(const std::string& item);

    void clear();

    const std::string& getSelectedText() const;

    int getSelectedIndex() const;

    void setSelectedIndex(int index);

    void setSelectionChangedCallback(
        std::function<void(int)> callback);

private:

    std::vector<std::string> items;

    int selectedIndex = -1;

    std::function<void(int)> onSelectionChanged;
};
