#include "NavigationItem.h"
#include "Renderer.h"
#include "Theme.h"

NavigationItem::NavigationItem()
{
}

void NavigationItem::render(Renderer& renderer)
{
    if (!visible)
        return;

    // Selected indicator
    if (selected)
    {
        SDL_Rect indicator =
        {
            bounds.x,
            bounds.y,
            4,
            bounds.h
        };

        renderer.fillRect(
            indicator,
            DefaultTheme.accent);
    }

    SDL_Color textColour = DefaultTheme.text;

    if (selected)
    {
        textColour = DefaultTheme.accent;
    }
    else if (hovered)
    {
        textColour = DefaultTheme.navigationHover;
    }

    constexpr int LeftPadding = 28;
    constexpr int TopPadding = 18;

    renderer.drawText(
        text,
        bounds.x + LeftPadding,
        bounds.y + TopPadding,
        textColour);
}

void NavigationItem::setText(const std::string& newText)
{
    text = newText;
}

void NavigationItem::setSelected(bool value)
{
    selected = value;
}

void NavigationItem::setOnClick(std::function<void()> callback)
{
    onClick = callback;
}

void NavigationItem::update(const SDL_Event& e)
{
    if (!visible)
        return;

    if (e.type == SDL_MOUSEMOTION)
    {
        SDL_Point mouse =
        {
            e.motion.x,
            e.motion.y
        };

        hovered = SDL_PointInRect(&mouse, &bounds);
    }

    if (e.type == SDL_MOUSEBUTTONDOWN)
    {
        if (hovered)
            pressed = true;
    }

    if (e.type == SDL_MOUSEBUTTONUP)
    {
        if (pressed && hovered)
        {
            if (onClick)
                onClick();
        }

        pressed = false;
    }
}