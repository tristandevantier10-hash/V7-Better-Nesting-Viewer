#include "HorizontalLayout.h"
#include "Button.h"
#include "Label.h"
#include <algorithm>

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

        if (centerChildrenVertically)
        {
            int childHeight = h - (padding * 2);
            auto label = std::dynamic_pointer_cast<Label>(item.element);
            auto button = std::dynamic_pointer_cast<Button>(item.element);

            if (label)
            {
                switch (label->getStyle())
                {
                case LabelStyle::Small: childHeight = 18; break;
                case LabelStyle::Heading: childHeight = 30; break;
                case LabelStyle::PageTitle: childHeight = 42; break;
                case LabelStyle::Normal: childHeight = 22; break;
                }
            }
            else if (button)
            {
                childHeight = 40;
            }

            childHeight = std::min(childHeight, h - (padding * 2));
            item.element->setPosition(currentX, y + (h - childHeight) / 2);
            item.element->setSize(panelWidth, childHeight);
        }

        currentX += panelWidth + spacing;
    }
}
