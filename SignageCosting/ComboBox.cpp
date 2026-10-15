#include "ComboBox.h"
#include "Renderer.h"
#include "Theme.h"
#include "Metrics.h"
#include <algorithm>

namespace
{
    ComboBox* activeDropdown = nullptr;
    Uint32 consumedClickTimestamp = 0;
}

ComboBox::ComboBox()
{
    setSize(200, Metrics::ControlHeight);
}

ComboBox::~ComboBox()
{
    if (activeDropdown == this)
        activeDropdown = nullptr;
}

bool ComboBox::hasOpenDropdown()
{
    return activeDropdown != nullptr;
}

bool ComboBox::wasClickConsumed(Uint32 timestamp)
{
    return timestamp != 0 && timestamp == consumedClickTimestamp;
}

void ComboBox::closeDropdown()
{
    dropdownOpen = false;
    hoveredItem = -1;
    if (activeDropdown == this)
        activeDropdown = nullptr;
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
    firstVisibleItem = 0;
    hoveredItem = -1;
    closeDropdown();
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
        if (selectedIndex < firstVisibleItem)
            firstVisibleItem = selectedIndex;
        else if (selectedIndex >= firstVisibleItem + 8)
            firstVisibleItem = selectedIndex - 7;
    }
}

void ComboBox::update(const SDL_Event& e)
{
    if (!visible || !enabled || items.empty())
        return;

    if (e.type == SDL_MOUSEBUTTONDOWN &&
        e.button.timestamp != 0 &&
        e.button.timestamp == consumedClickTimestamp)
        return;

    if (activeDropdown && activeDropdown != this)
        return;

    SDL_Rect r = getBounds();

    if (dropdownOpen && e.type == SDL_MOUSEMOTION)
    {
        const SDL_Rect menu = getDropdownBounds();
        SDL_Point mouse{ e.motion.x, e.motion.y };
        if (SDL_PointInRect(&mouse, &menu))
            hoveredItem = firstVisibleItem + (e.motion.y - menu.y) / Metrics::DropdownOptionHeight;
        else
            hoveredItem = -1;
        return;
    }

    if (dropdownOpen && e.type == SDL_KEYDOWN)
    {
        const int maxFirst = std::max(0, static_cast<int>(items.size()) - 8);
        switch (e.key.keysym.sym)
        {
        case SDLK_ESCAPE:
            closeDropdown();
            return;
        case SDLK_UP:
            selectedIndex = std::max(0, selectedIndex - 1);
            if (selectedIndex < firstVisibleItem)
                firstVisibleItem = selectedIndex;
            if (onSelectionChanged)
                onSelectionChanged(selectedIndex);
            return;
        case SDLK_DOWN:
            selectedIndex = std::min(static_cast<int>(items.size()) - 1, selectedIndex + 1);
            if (selectedIndex >= firstVisibleItem + 8)
                firstVisibleItem = std::min(maxFirst, selectedIndex - 7);
            if (onSelectionChanged)
                onSelectionChanged(selectedIndex);
            return;
        case SDLK_RETURN:
        case SDLK_KP_ENTER:
            closeDropdown();
            return;
        default:
            break;
        }
    }

    if (e.type == SDL_MOUSEWHEEL && dropdownOpen)
    {
        int mouseX = 0;
        int mouseY = 0;
        SDL_GetMouseState(&mouseX, &mouseY);
        const SDL_Rect menu = getDropdownBounds();
        SDL_Point mouse{ mouseX, mouseY };
        if (SDL_PointInRect(&mouse, &menu))
        {
            const int maxFirst = std::max(0, static_cast<int>(items.size()) - 8);
            firstVisibleItem = std::clamp(firstVisibleItem - e.wheel.y, 0, maxFirst);
        }
        return;
    }

    if (e.type != SDL_MOUSEBUTTONDOWN || e.button.button != SDL_BUTTON_LEFT)
        return;

    int mx = e.button.x;
    int my = e.button.y;
    SDL_Point mouse{ mx, my };

    if (dropdownOpen)
    {
        const SDL_Rect menu = getDropdownBounds();
        if (SDL_PointInRect(&mouse, &menu))
        {
            const int row = (my - menu.y) / Metrics::DropdownOptionHeight;
            const int index = firstVisibleItem + row;
            if (index >= 0 && index < static_cast<int>(items.size()))
            {
                const bool changed = selectedIndex != index;
                selectedIndex = index;
                closeDropdown();
                consumedClickTimestamp = e.button.timestamp;
                if (changed && onSelectionChanged)
                    onSelectionChanged(selectedIndex);
            }
            return;
        }

        if (!SDL_PointInRect(&mouse, &r))
        {
            closeDropdown();
            consumedClickTimestamp = e.button.timestamp;
            return;
        }
    }

    if (SDL_PointInRect(&mouse, &r))
    {
        dropdownOpen = !dropdownOpen;
        hoveredItem = -1;
        activeDropdown = dropdownOpen ? this : nullptr;
        consumedClickTimestamp = e.button.timestamp;
        const int maxFirst = std::max(0, static_cast<int>(items.size()) - 8);
        firstVisibleItem = std::clamp(firstVisibleItem, 0, maxFirst);
    }
}

SDL_Rect ComboBox::getDropdownBounds() const
{
    const int visibleCount = std::min(8, static_cast<int>(items.size()));
    const SDL_Rect bounds = getBounds();
    return { bounds.x, bounds.y + bounds.h, bounds.w, visibleCount * Metrics::DropdownOptionHeight };
}

void ComboBox::renderOverlay(Renderer& renderer)
{
    if (!visible || !enabled || !dropdownOpen || items.empty())
        return;

    const SDL_Rect menu = getDropdownBounds();
    renderer.fillRoundedRect(menu, { 255, 255, 255, 255 }, Metrics::ControlRadius);
    renderer.drawRoundedRect(menu, DefaultTheme.border, Metrics::ControlRadius);

    const int end = std::min(
        static_cast<int>(items.size()), firstVisibleItem + menu.h / Metrics::DropdownOptionHeight);
    for (int index = firstVisibleItem; index < end; ++index)
    {
        const SDL_Rect row{ menu.x + 1, menu.y + (index - firstVisibleItem) * Metrics::DropdownOptionHeight,
            menu.w - 2, Metrics::DropdownOptionHeight };
        if (index == hoveredItem)
            renderer.fillRect(row, { 235, 243, 249, 255 });
        else if (index == selectedIndex)
            renderer.fillRect(row, { 245, 247, 249, 255 });

        renderer.drawText(items[index], row.x + Metrics::ControlPaddingX, row.y + 5);
    }

    if (items.size() > 8)
    {
        const int indicatorY = menu.y + menu.h - 5;
        renderer.drawLine(menu.x + menu.w - 7, indicatorY - 3,
            menu.x + menu.w - 4, indicatorY, DefaultTheme.darkSecondaryText);
    }
}

void ComboBox::render(Renderer& renderer)
{
    if (!visible)
        return;

    SDL_Rect r = getBounds();

    //==================================================
    // Background
    //==================================================

    renderer.fillRoundedRect(
        r,
        { 255,255,255,255 },
        Metrics::ControlRadius);

    //==================================================
    // Border
    //==================================================

    renderer.drawRoundedRing(
        r,
        dropdownOpen ? DefaultTheme.accent : DefaultTheme.border,
        Metrics::ControlRadius,
        dropdownOpen ? 2 : 1);

    //==================================================
    // Selected Text
    //==================================================

    renderer.drawText(
        getSelectedText(),
        getX() + Metrics::ControlPaddingX,
        getY() + (getHeight() - Metrics::ControlTextHeight) / 2);

    //==================================================
    // Dropdown Indicator
    //==================================================

    const int arrowCenterX = getX() + getWidth() - 16;
    const int arrowCenterY = getY() + getHeight() / 2;
    renderer.drawLine(
        arrowCenterX - 4,
        arrowCenterY - 2,
        arrowCenterX,
        arrowCenterY + 2,
        DefaultTheme.darkSecondaryText);
    renderer.drawLine(
        arrowCenterX,
        arrowCenterY + 2,
        arrowCenterX + 4,
        arrowCenterY - 2,
        DefaultTheme.darkSecondaryText);
}

void ComboBox::setSelectionChangedCallback(
    std::function<void(int)> callback)
{
    onSelectionChanged = callback;
}
