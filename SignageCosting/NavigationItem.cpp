#include "NavigationItem.h"
#include "Renderer.h"
#include "Theme.h"
#include "Label.h"
#include "FontManager.h"
#include "Metrics.h"

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

    if (!heading && isEnabled() && (selected || hovered))
    {
        SDL_Rect selectedBackground =
        {
            bounds.x + 4,
            bounds.y + 2,
            bounds.w - 8,
            bounds.h - 4
        };

        renderer.fillRect(
            selectedBackground,
            selected ? DefaultTheme.sidebarSelected : DefaultTheme.navigationHover);
    }

    // ---------------------------------------------------------
    // Selected accent indicator
    // ---------------------------------------------------------

    if (selected && !heading && isEnabled())
    {
        SDL_Rect indicator =
        {
            bounds.x + 2,
            bounds.y,
            Metrics::NavigationIndicatorWidth,
            bounds.h
        };

        renderer.fillRect(
            indicator,
            DefaultTheme.accent);
    }

    // ---------------------------------------------------------
    // Text colour
    // ---------------------------------------------------------

    SDL_Color textColour =
        DefaultTheme.lightText;

    if (!isEnabled())
    {
        textColour = DefaultTheme.lightSecondaryText;
    }
    else if (heading)
    {
        textColour =
            DefaultTheme.lightSecondaryText;
    }
    else if (selected)
    {
        textColour = DefaultTheme.lightText;
    }
    else if (hovered)
    {
        textColour = DefaultTheme.lightText;
    }

    // ---------------------------------------------------------
    // Heading
    // ---------------------------------------------------------

    if (heading)
    {
        constexpr int LeftPadding = 22;
        constexpr int TopPadding = 10;
        constexpr int RightPadding = 8;
        constexpr int LabelGap = 8;

        int headingWidth = 0;
        int headingHeight = 0;
        TTF_Font* headingFont = renderer.getFontManager().getSmallFont();
        if (headingFont)
        {
            TTF_SizeUTF8(headingFont, text.c_str(), &headingWidth, &headingHeight);
        }
        (void)headingHeight;

        const int lineX = bounds.x + LeftPadding + headingWidth + LabelGap;
        const int lineWidth = bounds.x + bounds.w - RightPadding - lineX;
        if (lineWidth > 0)
        {
            renderer.fillRect(
                { lineX, bounds.y + bounds.h / 2, lineWidth, 1 },
                { 58, 64, 60, 255 });
        }

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
    // Keep the icon and label in a consistent left-aligned column.
    // ---------------------------------------------------------

    constexpr int IconSize = Metrics::NavigationIconSize;
    constexpr int IconTextGap = Metrics::NavigationIconGap;

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
    (void)textWidth;

    const bool hasIcon =
        !iconPath.empty();

    int groupX =
        bounds.x + Metrics::NavigationIndent;

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

    if (heading || !isEnabled())
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

    if (e.type == SDL_MOUSEBUTTONDOWN ||
        e.type == SDL_MOUSEBUTTONUP)
    {
        SDL_Point mouse =
        {
            e.button.x,
            e.button.y
        };
        hovered = SDL_PointInRect(&mouse, &bounds);

        if (e.type == SDL_MOUSEBUTTONDOWN)
        {
            if (e.button.button == SDL_BUTTON_LEFT)
                pressed = hovered;
        }
        else
        {
            const bool activate =
                e.button.button == SDL_BUTTON_LEFT && pressed && hovered;
            pressed = false;

            if (activate && onClick)
                onClick();
        }
    }
}
