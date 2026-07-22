#include "ListView.h"
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

void ListView::update(const SDL_Event& e)
{
    if (e.type != SDL_MOUSEBUTTONDOWN)
        return;

    if (e.button.button != SDL_BUTTON_LEFT)
        return;

    int mx = e.button.x;
    int my = e.button.y;

    if (mx < getX() ||
        mx > getX() + getWidth() ||
        my < getY() ||
        my > getY() + getHeight())
    {
        return;
    }

    int index =
        (my - getY() - 4) / rowHeight;

    if (index >= 0 &&
        index < static_cast<int>(items.size()))
    {
        selectedIndex = index;

        if (onSelectionChanged)
        {
            onSelectionChanged(selectedIndex);
        }
    }
}

void ListView::render(Renderer& renderer)
{
    renderer.fillRect(
        bounds,
        DefaultTheme.panelBackground);

    renderer.drawRect(
        bounds,
        DefaultTheme.border);

    int y = getY() + 4;

    for (int i = 0; i < static_cast<int>(items.size()); i++)
    {
        SDL_Rect row =
        {
            getX() + 2,
            y,
            getWidth() - 4,
            rowHeight
        };

        if (i == selectedIndex)
        {
            renderer.fillRect(
                row,
                DefaultTheme.buttonHover);
        }

        renderer.drawText(
            items[i],
            row.x + 8,
            row.y + 6);

        y += rowHeight;
    }
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