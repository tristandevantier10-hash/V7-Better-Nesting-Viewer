#include "NavigationItem.h"
#include "Renderer.h"
#include "Theme.h"
#include "Label.h"
#include "FontManager.h"
#include "Metrics.h"

NavigationItem::NavigationItem()
{
}

const std::string& NavigationItem::getText() const
{
    return text;
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

        renderer.fillRoundedRect(
            selectedBackground,
            selected ? DefaultTheme.sidebarSelected : DefaultTheme.navigationHover, 5);
    }

    // ---------------------------------------------------------
    // Selected accent indicator
    // ---------------------------------------------------------

    if (selected && !heading && isEnabled())
    {
        SDL_Rect indicator =
        {
            bounds.x + 4,
            bounds.y + 2,
            Metrics::NavigationIndicatorWidth,
            bounds.h - 4
        };

        renderer.fillRoundedRect(
            indicator,
            DefaultTheme.accent, 2);
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
        auto* font = renderer.getFontManager().getSectionFont();
        int height = 0;
        if (font && TTF_SizeUTF8(font, text.c_str(), nullptr, &height) == 0)
        {
            renderer.drawText(text, bounds.x + 7,
                bounds.y + (bounds.h - height) / 2, font, DefaultTheme.lightText);
        }
        const SDL_Color green{ 0, 148, 74, 255 };
        const int right = bounds.x + bounds.w - 7;
        const int y = bounds.y + bounds.h - 1;
        renderer.fillRect({ bounds.x + 7, y, bounds.w - 14, 1 }, green);
        renderer.drawLine(right - 24, y - 4, right - 18, y - 16, green);
        renderer.drawLine(right - 18, y - 16, right - 12, y - 4, green);
        renderer.drawLine(right - 24, y - 4, right - 12, y - 4, green);
        for (int row = 0; row < 12; ++row)
        {
            const int inset = row / 2;
            renderer.fillRect({ right - 13 + inset, y - 16 + row, 13 - 2 * inset, 1 }, green);
        }
        return;
    }

    // ---------------------------------------------------------
    // Navigation item
    //
    // Keep the icon and label in a consistent left-aligned column.
    // ---------------------------------------------------------

    constexpr int IconSize = Metrics::NavigationIconSize;
    constexpr int IconTextGap = Metrics::NavigationIconGap;

    auto* font = renderer.getFontManager().getNavigationFont();

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
        (bounds.h - textHeight) / 2 + 2;

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

        renderer.drawSVG(iconPath, iconRect, textColour);
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
