#pragma once

#include "UIElement.h"
#include "Theme.h"

#include <string>
#include <functional>

class Renderer;

class Button : public UIElement
{
public:

    Button();

    void update(const SDL_Event& e) override;

    void render(Renderer& renderer) override;

    void setText(const std::string& text);

    void setOnClick(std::function<void()> callback);

private:

    std::string text;

    std::function<void()> onClick;
};