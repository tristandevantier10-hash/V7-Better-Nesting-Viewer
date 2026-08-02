#include "HorizontalLayout.h"

void HorizontalLayout::performLayout(
    int x,
    int y,
    int w,
    int h,
    int padding)
{
    if (items.empty())
        return;

    int fixedWidth = 0;
    int fillCount = 0;
    int visibleCount = 0;

    for (const auto& item : items)
    {
        if (!item.element->isVisible())
            continue;

        visibleCount++;

        if (item.policy == SizePolicy::Fixed)
            fixedWidth += item.size;
        else
            fillCount++;
    }

    if (visibleCount > 1)
    {
        fixedWidth += (visibleCount - 1) * spacing;
    }

    int remainingWidth =
        w - fixedWidth - (padding * 2);

    int fillWidth =
        (fillCount > 0)
        ? remainingWidth / fillCount
        : 0;

    int currentX = x + padding;

    for (auto& item : items)
    {
        if (!item.element->isVisible())
            continue;

        int panelWidth =
            (item.policy == SizePolicy::Fixed)
            ? item.size
            : fillWidth;

        item.element->setPosition(
            currentX,
            y + padding);

        item.element->setSize(
            panelWidth,
            h - (padding * 2));

        currentX += panelWidth + spacing;
    }
}