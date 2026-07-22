#include "VerticalLayout.h"
#include "iostream"

#include "VerticalLayout.h"
#include <iostream>

void VerticalLayout::performLayout(
    int x,
    int y,
    int w,
    int h)
{
    /*
    std::cout << "\n===== VerticalLayout =====" << std::endl;
    std::cout << "Items: " << items.size() << std::endl;
    */

    if (items.empty())
        return;

    const int padding = 20;

    int fixedHeight = 0;
    int fillCount = 0;

    for (const auto& item : items)
    {
        if (!item.element->isVisible())
            continue;

        if (item.policy == SizePolicy::Fixed)
            fixedHeight += item.size;
        else
            fillCount++;
    }

    int remainingHeight =
        h - fixedHeight - (padding * 2);

    int fillHeight =
        (fillCount > 0)
        ? remainingHeight / fillCount
        : 0;

    int currentY = y + padding;

    for (auto& item : items)
    {
        if (!item.element->isVisible())
            continue;

        int panelHeight =
            (item.policy == SizePolicy::Fixed)
            ? item.size
            : fillHeight;

        /*std::cout
            << "Element: " << item.element.get()
            << "  x=" << x
            << " y=" << currentY
            << " w=" << w
            << " h=" << panelHeight
            << std::endl;
            */

        item.element->setPosition(
            x + padding,
            currentY);

        item.element->setSize(
            w - (padding * 2),
            panelHeight);

        currentY += panelHeight;
    }

    /*std::cout << "==========================" << std::endl;*/
}