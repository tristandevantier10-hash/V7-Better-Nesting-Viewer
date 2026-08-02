#include "SegmentedControl.h"
#include "Renderer.h"
#include "Theme.h"
#include "FontManager.h"

SegmentedControl::SegmentedControl()
{}

void SegmentedControl::addSegment(const std::string& text)
{
    items.push_back(text);
}

void SegmentedControl::setSelectedIndex(int index)
{
    if (index < 0)
        return;

    if (index >= static_cast<int>(items.size()))
        return;

    selectedIndex = index;
}

int SegmentedControl::getSelectedIndex() const
{
    return selectedIndex;
}

void SegmentedControl::setSelectionChangedCallback(
    std::function<void(int)> callback)
{
    selectionChanged = callback;
}

void SegmentedControl::update(const SDL_Event&)
{}

void SegmentedControl::render(Renderer& renderer)
{
    if (!visible)
        return;

    SDL_Rect background =
    {
        getX(),
        getY(),
        getWidth(),
        getHeight()
    };

    // Background
    renderer.fillRoundedRect(
        background,
        DefaultTheme.panelBackground,
        10);

    if (items.empty())
        return;

    int segmentWidth = background.w / static_cast<int>(items.size());

    for (int i = 0; i < static_cast<int>(items.size()); i++)
    {
        SDL_Rect segment =
        {
            background.x + i * segmentWidth,
            background.y,
            segmentWidth,
            background.h
        };

        // Selected underline
        if (i == selectedIndex)
        {
            SDL_Rect underline =
            {
                segment.x + 20,
                segment.y + segment.h - 3,
                segment.w - 40,
                3
            };

            renderer.fillRoundedRect(
                underline,
                DefaultTheme.accent,
                2);
        }

        // Draw text
        SDL_Color colour =
            (i == selectedIndex)
            ? DefaultTheme.darkText
            : DefaultTheme.darkSecondaryText;

        renderer.drawText(
            items[i],
            segment.x + 18,
            segment.y + 12,
            renderer.getFontManager().getNormalFont(),
            colour);
    }
}