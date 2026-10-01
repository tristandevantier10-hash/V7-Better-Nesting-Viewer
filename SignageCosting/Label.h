#pragma once

#include "UIElement.h"
#include <string>
#include <SDL2/SDL.h>

class Renderer;

enum class LabelStyle
{
    Small,
    Normal,
    Heading
};

enum class TextTheme
{
    Light,
    Dark,

    LightSecondary,
    DarkSecondary,

    Title,

    Success,
    Warning,
    Error
};

class Label : public UIElement
{
public:

    Label();

    void setText(const std::string& text);

    const std::string& getText() const;

    void render(Renderer& renderer) override;

    void setStyle(LabelStyle style);

    void setTextTheme(TextTheme theme);

private:

    std::string text;

    LabelStyle style = LabelStyle::Normal;

    TextTheme textTheme = TextTheme::Dark;
};