#include "HorizontalLayout.h"

void HorizontalLayout::performLayout(
    int x,
    int y,
    int w,
    int h)
{
    if (items.empty())
        return;

    int fixedWidth = 0;
    int fillCount = 0;

    for (const auto& item : items)
    {
        if (item.policy == SizePolicy::Fixed)
            fixedWidth += item.size;
        else
            fillCount++;
    }

    int remainingWidth = w - fixedWidth;

    int fillWidth =
        (fillCount > 0)
        ? remainingWidth / fillCount
        : 0;

    int currentX = x;

    for (auto& item : items)
    {
        int panelWidth =
            (item.policy == SizePolicy::Fixed)
            ? item.size
            : fillWidth;

        item.element->setPosition(currentX, y);
        item.element->setSize(panelWidth, h);

        currentX += panelWidth;
    }
}