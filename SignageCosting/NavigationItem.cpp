#include "NavigationItem.h"
#include "Renderer.h"
#include "Theme.h"
#include "Label.h"
#include "FontManager.h"

NavigationItem::NavigationItem()
{
}

void NavigationItem::render(Renderer& renderer)
{
    if (!visible)
        return;

    // ---------------------------------------------------------
    // Selected background
    // ---------------------------------------------------------

    if (selected && !heading)
    {
        SDL_Rect selectedBackground =
        {
            bounds.x + 8,
            bounds.y + 4,
            bounds.w - 16,
            bounds.h - 8
        };

        renderer.fillRect(
            selectedBackground,
            DefaultTheme.sidebarSelected);
    }

    // ---------------------------------------------------------
    // Selected accent indicator
    // ---------------------------------------------------------

    if (selected && !heading)
    {
        SDL_Rect indicator =
        {
            bounds.x,
            bounds.y + 4,
            4,
            bounds.h - 8
        };

        renderer.fillRect(
            indicator,
            DefaultTheme.accent);
    }

    // ---------------------------------------------------------
    // Text colour
    // ---------------------------------------------------------

    SDL_Color textColour =
        DefaultTheme.text;

    if (heading)
    {
        textColour =
            DefaultTheme.darkSecondaryText;
    }
    else if (selected)
    {
        textColour =
            DefaultTheme.lightText;
    }
    else if (hovered)
    {
        textColour =
            DefaultTheme.navigationHover;
    }

    // ---------------------------------------------------------
    // Heading
    // ---------------------------------------------------------

    if (heading)
    {
        constexpr int LeftPadding = 22;
        constexpr int TopPadding = 10;

        renderer.drawText(
            text,
            bounds.x + LeftPadding,
            bounds.y + TopPadding,
            LabelStyle::Small,
            textColour);

        return;
    }

    // ---------------------------------------------------------
    // Navigation item
    //
    // Icon + text are treated as ONE group and centred
    // horizontally inside the navigation item.
    // ---------------------------------------------------------

    constexpr int IconSize = 22;
    constexpr int IconTextGap = 10;

    TTF_Font* font =
        renderer.getFontManager().getSmallFont();

    int textWidth = 0;
    int textHeight = 0;

    if (font)
    {
        TTF_SizeUTF8(
            font,
            text.c_str(),
            &textWidth,
            &textHeight);
    }

    const bool hasIcon =
        !iconPath.empty();

    int groupWidth =
        textWidth;

    if (hasIcon)
    {
        groupWidth +=
            IconSize +
            IconTextGap;
    }

    int groupX =
        bounds.x + 22;

    int iconY =
        bounds.y +
        (bounds.h - IconSize) / 2;

    int textY =
        bounds.y +
        (bounds.h - textHeight) / 2;

    // ---------------------------------------------------------
    // Icon
    // ---------------------------------------------------------

    if (hasIcon)
    {
        SDL_Rect iconRect =
        {
            groupX,
            iconY,
            IconSize,
            IconSize
        };

        renderer.drawSVG(
            iconPath,
            iconRect,
            textColour);
    }

    // ---------------------------------------------------------
    // Text
    // ---------------------------------------------------------

    int textX =
        groupX;

    if (hasIcon)
    {
        textX +=
            IconSize +
            IconTextGap;
    }

    renderer.drawText(
        text,
        textX,
        textY,
        LabelStyle::Small,
        textColour);
}

void NavigationItem::setText(
    const std::string& newText)
{
    text = newText;
}

void NavigationItem::setSelected(
    bool value)
{
    selected = value;
}

void NavigationItem::setHeading(
    bool value)
{
    heading = value;
}

void NavigationItem::setOnClick(
    std::function<void()> callback)
{
    onClick = callback;
}

void NavigationItem::setIcon(
    const std::string& newIconPath)
{
    iconPath = newIconPath;
}

void NavigationItem::update(
    const SDL_Event& e)
{
    if (!visible)
        return;

    if (heading)
        return;

    if (e.type == SDL_MOUSEMOTION)
    {
        SDL_Point mouse =
        {
            e.motion.x,
            e.motion.y
        };

        hovered =
            SDL_PointInRect(
                &mouse,
                &bounds);
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