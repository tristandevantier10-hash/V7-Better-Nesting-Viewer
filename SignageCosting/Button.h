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

    void setIcon(const std::string& path);

    void setOnClick(std::function<void()> callback);

    enum class ButtonStyle
    {
        Normal,
        Navigation,
        FilterChip,
        Primary,
        Accent,
        Secondary
    };

    void setStyle(ButtonStyle style)
    {
        this->style = style;
    }

    void setSelected(bool value);

    void setTextCentered(bool centered)
    {
        textCentered = centered;
    }

    bool selected = false;

private:

    std::string text;
    std::string iconPath;

    std::function<void()> onClick;

    ButtonStyle style = ButtonStyle::Normal;

    bool hovered = false;

    bool textCentered = false;
};
