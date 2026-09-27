#include "SegmentedControl.h"
#include "Renderer.h"
#include "Theme.h"
#include "FontManager.h"

SegmentedControl::SegmentedControl()
{
    animatedPosition = 0.0f;
}

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

void SegmentedControl::update(const SDL_Event& e)
{

    if (!visible)
        return;

    if (e.type == SDL_MOUSEMOTION)
    {
        hoveredIndex = -1;

        int mouseX = e.motion.x;
        int mouseY = e.motion.y;

        if (!items.empty() &&
            mouseX >= getX() &&
            mouseX < getX() + getWidth() &&
            mouseY >= getY() &&
            mouseY < getY() + getHeight())
        {
            int segmentWidth =
                getWidth() / static_cast<int>(items.size());

            hoveredIndex =
                (mouseX - getX()) / segmentWidth;
        }
    }

    if (e.type != SDL_MOUSEBUTTONDOWN)
        return;

    if (e.button.button != SDL_BUTTON_LEFT)
        return;

    int mouseX = e.button.x;
    int mouseY = e.button.y;

    if (mouseX < getX() ||
        mouseX >= getX() + getWidth() ||
        mouseY < getY() ||
        mouseY >= getY() + getHeight())
    {
        return;
    }

    if (items.empty())
        return;

    int segmentWidth = getWidth() / static_cast<int>(items.size());

    int index = (mouseX - getX()) / segmentWidth;

    if (index != selectedIndex)
    {
        selectedIndex = index;

        if (selectionChanged)
            selectionChanged(selectedIndex);
    }
}

void SegmentedControl::tick(float)
{
    animatedPosition += (selectedIndex - animatedPosition) * 0.18f;
}

void SegmentedControl::render(Renderer& renderer)
{
    if (!visible)
        return;

    if (items.empty())
        return;

    int segmentWidth =
        getWidth() / static_cast<int>(items.size());

    //--------------------------------------------------
    // Combined transparent control background
    //--------------------------------------------------

    SDL_Rect controlBounds =
    {
        getX(),
        getY(),
        getWidth(),
        getHeight()
    };

    // No fill — just the combined outline
    renderer.drawRoundedRect(
        controlBounds,
        { 0, 0, 0, 255 },
        7);

    //--------------------------------------------------
    // Animated green selection pill
    //--------------------------------------------------

    SDL_Rect selected =
    {
        getX() +
        static_cast<int>(animatedPosition * segmentWidth) +
        4,

        getY() + 4,

        segmentWidth - 8,
        getHeight() - 8
    };

    renderer.fillRoundedRect(
        selected,
        DefaultTheme.accent,
        7);

    //--------------------------------------------------
    // Text
    //--------------------------------------------------

    for (int i = 0;
        i < static_cast<int>(items.size());
        i++)
    {
        int textWidth =
            renderer.getTextWidth(items[i]);

        int textX =
            getX() +
            i * segmentWidth +
            (segmentWidth - textWidth) / 2;

        int textY =
            getY() +
            (getHeight() - 20) / 2;

        SDL_Color textColour;

        if (i == selectedIndex)
        {
            // Selected = white on green
            textColour =
            { 255, 255, 255, 255 };
        }
        else
        {
            // Unselected = dark text on transparent background
            textColour =
            { 70, 70, 70, 255 };
        }

        renderer.drawText(
            items[i],
            textX,
            textY,
            renderer.getFontManager().getNormalFont(),
            textColour);
    }
}