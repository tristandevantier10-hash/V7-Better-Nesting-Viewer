#pragma once

#include "UIElement.h"
#include <string>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

class Renderer;

enum class LabelStyle
{
    Small,
    Normal,
    Heading
};

enum class LabelAlignment
{
    Left,
    Center,
    Right
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
    void setCustomFont(TTF_Font* font);
    LabelStyle getStyle() const { return style; }

    void setTextTheme(TextTheme theme);

    void setAlignment(LabelAlignment alignment);
    void setAlignmentOffsetX(int offset);

private:

    std::string text;

    LabelStyle style = LabelStyle::Normal;

    TextTheme textTheme = TextTheme::Dark;
    LabelAlignment alignment = LabelAlignment::Left;
    int alignmentOffsetX = 0;
    TTF_Font* customFont = nullptr;
};
