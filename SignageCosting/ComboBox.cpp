#include "ComboBox.h"
#include "Renderer.h"
#include "Theme.h"

ComboBox::ComboBox()
{
    setSize(200, 32);
}

void ComboBox::addItem(const std::string& item)
{
    items.push_back(item);

    if (selectedIndex == -1)
    {
        selectedIndex = 0;
    }
}

void ComboBox::clear()
{
    items.clear();
    selectedIndex = -1;
}

const std::string& ComboBox::getSelectedText() const
{
    static std::string empty;

    if (selectedIndex < 0 ||
        selectedIndex >= static_cast<int>(items.size()))
    {
        return empty;
    }

    return items[selectedIndex];
}

int ComboBox::getSelectedIndex() const
{
    return selectedIndex;
}

void ComboBox::setSelectedIndex(int index)
{
    if (index >= 0 &&
        index < static_cast<int>(items.size()))
    {
        selectedIndex = index;
    }
}

void ComboBox::update(const SDL_Event& e)
{
    if (e.type != SDL_MOUSEBUTTONDOWN)
        return;

    if (e.button.button != SDL_BUTTON_LEFT)
        return;

    SDL_Rect r = getBounds();

    int mx = e.button.x;
    int my = e.button.y;

    if (mx < r.x || mx >= r.x + r.w ||
        my < r.y || my >= r.y + r.h)
    {
        return;
    }

    if (items.empty())
        return;

    selectedIndex++;

    if (selectedIndex >= static_cast<int>(items.size()))
    {
        selectedIndex = 0;
    }

    if (onSelectionChanged)
    {
        onSelectionChanged(selectedIndex);
    }

}

void ComboBox::render(Renderer& renderer)
{
    if (!visible)
        return;

    renderer.fillRect(
        getBounds(),
        DefaultTheme.panelBackground);

    renderer.drawRect(
        getBounds(),
        DefaultTheme.border);

    renderer.drawText(
        getSelectedText(),
        getX() + 8,
        getY() + 8);

    renderer.drawText(
        ">",
        getX() + getWidth() - 18,
        getY() + 8);
}

void ComboBox::setSelectionChangedCallback(
    std::function<void(int)> callback)
{
    onSelectionChanged = callback;
}