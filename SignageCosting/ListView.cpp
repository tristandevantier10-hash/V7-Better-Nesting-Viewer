#include "ListView.h"
#include "Label.h"
#include "Renderer.h"
#include "Theme.h"

ListView::ListView()
{
    setSize(200, 200);
}

void ListView::addItem(const std::string& text)
{
    items.push_back(text);

    if (selectedIndex == -1)
        selectedIndex = 0;
}

void ListView::removeItem(int index)
{
    if (index < 0 || index >= static_cast<int>(items.size()))
        return;

    items.erase(items.begin() + index);

    if (selectedIndex >= static_cast<int>(items.size()))
        selectedIndex = static_cast<int>(items.size()) - 1;
}

void ListView::clear()
{
    items.clear();
    selectedIndex = -1;
}

int ListView::getSelectedIndex() const
{
    return selectedIndex;
}

const std::string& ListView::getSelectedItem() const
{
    static std::string empty;

    if (selectedIndex < 0 ||
        selectedIndex >= static_cast<int>(items.size()))
    {
        return empty;
    }

    return items[selectedIndex];
}

void ListView::setSelectedIndex(int index)
{
    if (index >= 0 &&
        index < static_cast<int>(items.size()))
    {
        selectedIndex = index;
    }
}

void ListView::update(const SDL_Event& e) {
    // 1. Handle Mouse Hover Motion
    if (e.type == SDL_MOUSEMOTION) {
        int mx = e.motion.x;
        int my = e.motion.y;

        // Check if mouse is within ListView boundaries
        if (mx >= getX() && mx <= getX() + getWidth() &&
            my >= getY() && my <= getY() + getHeight()) {

            int index = (my - getY()) / rowHeight;
            if (index >= 0 && index < static_cast<int>(items.size())) {
                hoveredIndex = index; // Update tracked hover row
            }
            else {
                hoveredIndex = -1;
            }
        }
        else {
            hoveredIndex = -1; // Reset when mouse leaves component
        }
    }

    // 2. Handle Mouse Clicks (Your Original Code)
    if (e.type != SDL_MOUSEBUTTONDOWN) return;
    if (e.button.button != SDL_BUTTON_LEFT) return;

    int mx = e.button.x;
    int my = e.button.y;

    if (mx < getX() || mx > getX() + getWidth() || my < getY() || my > getY() + getHeight()) {
        return;
    }

    int index = (my - getY()) / rowHeight;
    if (index >= 0 && index < static_cast<int>(items.size())) {
        selectedIndex = index;
        if (onSelectionChanged) {
            onSelectionChanged(selectedIndex);
        }
    }
}

void ListView::render(Renderer& renderer)
{
    renderer.pushClip(bounds);

    renderer.fillRect(
        bounds,
        { 255, 255, 255, 255 });

    int y = getY();
    for (int i = 0; i < static_cast<int>(items.size()); i++) {
        SDL_Rect row = { getX(), y, getWidth(), rowHeight };

        SDL_Color rowColour =
            (i % 2 == 0)
            ? SDL_Color{ 255, 255, 255, 255 }
            : SDL_Color{ 250, 250, 250, 255 };

        if (i == selectedIndex) {
            rowColour = DefaultTheme.accent;
        }
        else if (i == hoveredIndex) {
            rowColour = { 240, 244, 248, 255 };
        }

        renderer.fillRect(row, rowColour);
        renderer.drawText(
            items[i],
            row.x + 10,
            row.y + (rowHeight - 16) / 2,
            LabelStyle::Small,
            i == selectedIndex
                ? SDL_Color{ 255, 255, 255, 255 }
                : DefaultTheme.darkText);

        renderer.drawLine(
            row.x,
            row.y + rowHeight - 1,
            row.x + row.w,
            row.y + rowHeight - 1,
            { 235, 237, 240, 255 });

        y += rowHeight;
    }

    renderer.drawRect(bounds, { 220, 220, 220, 255 });
    renderer.popClip();
}

void ListView::setSelectionChangedCallback(
    std::function<void(int)> callback)
{
    onSelectionChanged = callback;
}

void ListView::updateItem(
    int index,
    const std::string& text)
{
    if (index < 0 ||
        index >= static_cast<int>(items.size()))
    {
        return;
    }

    items[index] = text;
}

void ListView::setItem(
    int index,
    const std::string& text)
{
    if (index < 0 ||
        index >= static_cast<int>(items.size()))
    {
        return;
    }

    items[index] = text;
}
