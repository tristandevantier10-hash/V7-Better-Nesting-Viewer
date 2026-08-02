#include "VerticalLayout.h"
#include "iostream"
#include "Metrics.h"

void VerticalLayout::performLayout(
    int x,
    int y,
    int w,
    int h,
    int padding)
{

    if (items.empty())
        return;

    int fixedHeight = 0;
    int fillCount = 0;
    int visibleCount = 0;

    for (const auto& item : items)
    {
        if (!item.element->isVisible())
            continue;

        visibleCount++;

        if (item.policy == SizePolicy::Fixed)
            fixedHeight += item.size;
        else
            fillCount++;
    }

    // Account for spacing between controls
    if (visibleCount > 1)
    {
        fixedHeight += (visibleCount - 1) * spacing;
    }

    int remainingHeight =
        h
        - (padding * 2)
        - topInset
        - fixedHeight;

    int fillHeight =
        (fillCount > 0)
        ? remainingHeight / fillCount
        : 0;

    int currentY =
        y
        + padding
        + topInset;

    for (auto& item : items)
    {
        if (!item.element->isVisible())
            continue;

        int panelHeight =
            (item.policy == SizePolicy::Fixed)
            ? item.size
            : fillHeight;

        item.element->setPosition(
            x + padding,
            currentY);

        item.element->setSize(
            w - (padding * 2),
            panelHeight);

        currentY += panelHeight + spacing;
    }

    /*std::cout << "==========================" << std::endl;*/
}