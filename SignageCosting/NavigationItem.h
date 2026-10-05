#pragma once

#include "UIElement.h"
#include <functional>
#include <string>

class Renderer;

class NavigationItem : public UIElement
{
public:
    NavigationItem();

    void setText(const std::string& text);
    void setSelected(bool selected);
    void setHeading(bool heading);
    void setOnClick(std::function<void()> callback);

    void setIcon(const std::string& iconPath);

    void update(const SDL_Event& e) override;
    void render(Renderer& renderer) override;

private:
    std::string text;
    std::string iconPath;

    bool selected = false;
    bool heading = false;

    std::function<void()> onClick;
};