#pragma once

#include "UIElement.h"

#include <vector>
#include <string>
#include <functional>

class Renderer;

class SegmentedControl : public UIElement
{
public:

    SegmentedControl();

    void addSegment(const std::string& text);

    void setSelectedIndex(int index);

    int getSelectedIndex() const;

    void setSelectionChangedCallback(
        std::function<void(int)> callback);

    void update(const SDL_Event& e) override;

    void tick(float deltaTime) override;

    void render(Renderer& renderer) override;

private:

    std::vector<std::string> items;

    int selectedIndex = 0;

    float animatedPosition = 0.0f;

    int hoveredIndex = -1;

    std::function<void(int)> selectionChanged;
};
