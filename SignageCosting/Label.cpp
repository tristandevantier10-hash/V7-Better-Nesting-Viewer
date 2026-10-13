#include "Label.h"
#include "Renderer.h"
#include "TextRenderer.h"
#include <algorithm>

Label::Label()
{}

void Label::setText(const std::string& value)
{
    text = value;
}

const std::string& Label::getText() const
{
    return text;
}

void Label::render(Renderer& renderer)
{
    if (!visible)
        return;

    TTF_Font* font = nullptr;

    switch (style)
    {
    case LabelStyle::Small:
        font = renderer.getFontManager().getSmallFont();
        break;

    case LabelStyle::Heading:
        font = renderer.getFontManager().getHeadingFont();
        break;

    case LabelStyle::Normal:
    default:
        font = renderer.getFontManager().getNormalFont();
        break;
    }

    SDL_Color colour;

    switch (textTheme)
    {
    case TextTheme::Light:
        colour = DefaultTheme.lightText;
        break;

    case TextTheme::Dark:
        colour = DefaultTheme.darkText;
        break;

    case TextTheme::LightSecondary:
        colour = DefaultTheme.lightSecondaryText;
        break;

    case TextTheme::DarkSecondary:
        colour = DefaultTheme.darkSecondaryText;
        break;

    case TextTheme::Title:
        colour = DefaultTheme.titleText;
        break;

    case TextTheme::Success:
        colour = DefaultTheme.successText;
        break;

    case TextTheme::Warning:
        colour = DefaultTheme.warningText;
        break;

    case TextTheme::Error:
        colour = DefaultTheme.errorText;
        break;

    default:
        colour = DefaultTheme.darkText;
        break;
    }

    int x = getX();
    if (alignment != LabelAlignment::Left && font)
    {
        int textWidth = 0;
        int textHeight = 0;
        if (TTF_SizeUTF8(font, text.c_str(), &textWidth, &textHeight) == 0)
        {
            if (alignment == LabelAlignment::Center)
                x += std::max(0, (getWidth() - textWidth) / 2) + alignmentOffsetX;
            else
                x += std::max(0, getWidth() - textWidth);
        }
    }

    renderer.drawText(
        text,
        x,
        getY(),
        font,
        colour);
}

void Label::setStyle(LabelStyle value)
{
    style = value;
}

void Label::setTextTheme(TextTheme theme)
{
    textTheme = theme;
}

void Label::setAlignment(LabelAlignment value)
{
    alignment = value;
}

void Label::setAlignmentOffsetX(int offset)
{
    alignmentOffsetX = offset;
}
