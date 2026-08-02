#include "Label.h"
#include "Renderer.h"
#include "TextRenderer.h"
#include "iostream"

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

    default:
        font = renderer.getFontManager().getNormalFont();
        break;
    }

    SDL_Color colour;

    if (useOverrideColour)
    {
        colour = overrideColour;
    }
    else
    {
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
    }

    renderer.drawText(
        text,
        getX(),
        getY(),
        font,
        colour);
}

void Label::setTextColour(SDL_Color colour)
{
    textColour = colour;
}

void Label::setStyle(LabelStyle value)
{
    style = value;
}

LabelStyle Label::getStyle() const
{
    return style;
}

void Label::setTextTheme(TextTheme theme)
{
    textTheme = theme;
}